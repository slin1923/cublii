#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "net/wifi_manager.h"

// Connect to WiFi as a station (client). Blocks until connected or the
// timeout expires; if it times out, auto-reconnect keeps trying in the background.
void wifi_begin(const char* ssid, const char* pass, uint32_t connectTimeoutMs) {
  WiFi.persistent(false);          // don't write credentials to flash on every connect
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_HOSTNAME);
  WiFi.setSleep(false);            // modem sleep adds latency to MQTT and OTA
  WiFi.setAutoReconnect(true);     // rejoin automatically if the link drops
  WiFi.begin(ssid, pass);

  // Wait for the connection, polling every 250 ms, up to the timeout.
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < connectTimeoutMs) {
    delay(250);
  }
}

// Used by the MQTT and OTA modules to know when the network is usable.
bool wifi_connected() {
  return WiFi.status() == WL_CONNECTED;
}