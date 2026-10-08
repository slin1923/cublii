#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include "config.h"
#include "net/mqtt_link.h"
#include "net/wifi_manager.h"

// ---- Module state (file-private) ----
static WiFiClient          s_wifi;                    // underlying TCP connection
static PubSubClient        s_client(s_wifi);          // MQTT client riding on top of it
static MqttMessageHandler  s_handler = nullptr;       // caller's callback for incoming messages
static const char*         s_clientId = "esp32";      // ID we present to the broker
static uint32_t            s_lastAttemptMs = 0;       // when we last tried to connect
static bool                s_attempted = false;       // false until the first connect attempt

// PubSubClient calls this for every incoming message.
// It hands us raw bytes (not null-terminated), so we copy into a
// buffer, terminate it, and pass a normal C string to the user's handler.
static void onRawMessage(char* topic, uint8_t* payload, unsigned int len) {
  static char buf[128];
  if (len >= sizeof(buf)) len = sizeof(buf) - 1;   // truncate oversized payloads
  memcpy(buf, payload, len);
  buf[len] = '\0';
  if (s_handler) s_handler(topic, buf);
}

// One-time setup. Doesn't connect yet; mqtt_loop() handles that.
void mqtt_begin(const char* broker, uint16_t port, const char* clientId, MqttMessageHandler handler) {
  s_handler = handler;
  s_clientId = clientId;
  s_client.setServer(broker, port);
  s_client.setBufferSize(512);           // max MQTT packet size (default is only 256)
  s_client.setSocketTimeout(2);          // seconds; PubSubClient's default is 15
  s_client.setCallback(onRawMessage);
}

// Call this repeatedly from loop(). Handles (re)connecting and message processing.
void mqtt_loop() {
  if (!wifi_connected()) return;         // no WiFi, nothing to do

  if (!s_client.connected()) {
    // Rate-limit reconnect attempts so we don't hammer the broker
    // (or block the main loop, since connect() can take a while).
    const uint32_t now = millis();
    if (s_attempted && (now - s_lastAttemptMs) < MQTT_RETRY_MS) return;
    s_attempted = true;
    s_lastAttemptMs = now;

    // On success, subscribe to all command topics under our prefix.
    // On failure we just retry after MQTT_RETRY_MS.
    if (s_client.connect(s_clientId)) {
      s_client.subscribe(MQTT_CMD_PREFIX "#");
    }
    return;
  }

  s_client.loop();                       // process incoming messages + keepalive pings
}

bool mqtt_connected() { return s_client.connected(); }

// Publish a message; returns false if we're not currently connected.
bool mqtt_publish(const char* topic, const char* payload) {
  if (!s_client.connected()) return false;
  return s_client.publish(topic, payload);
}