#pragma once

typedef void (*OtaStartHook)();

// Registers OTA. The actual ArduinoOTA.begin() happens lazily inside ota_loop()
// once WiFi is connected, so a late WiFi connection never leaves OTA dead.
// onStart is called right before an upload begins - use it to make the hardware safe.
void ota_begin(const char* hostname, OtaStartHook onStart);
void ota_loop();
