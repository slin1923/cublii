#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "net/ota.h"
#include "net/wifi_manager.h"

// ---- Module state (file-private) ----
static const char*  s_hostname = "esp32";    // name shown in the IDE / used for upload target
static OtaStartHook s_onStart = nullptr;     // optional caller hook, runs when an upload begins
static bool         s_started = false;       // true once ArduinoOTA.begin() has been called

// Configure and start the OTA service. Called lazily from ota_loop()
// once WiFi is up, since ArduinoOTA needs a network to listen on.
static void startNow() {
  ArduinoOTA.setHostname(s_hostname);
  // ArduinoOTA.setPassword("somepassword");   // if you set one, add --auth to upload_flags

  // Fires when an upload begins. Use the hook to shut things down
  // (motors, outputs, etc.) before flash is overwritten.
  ArduinoOTA.onStart([]() {
    if (s_onStart) s_onStart();
  });

  ArduinoOTA.begin();
  s_started = true;
}

// Store settings only. Doesn't start OTA yet; ota_loop() does that
// once WiFi is connected.
void ota_begin(const char* hostname, OtaStartHook onStart) {
  s_hostname = hostname;
  s_onStart = onStart;
}

// Call this repeatedly from loop(). Starts OTA on first WiFi connection,
// then services incoming upload requests.
void ota_loop() {
  if (!s_started) {
    if (!wifi_connected()) return;       // wait for WiFi before starting
    startNow();
  }
  ArduinoOTA.handle();
}