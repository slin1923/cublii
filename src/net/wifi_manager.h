#pragma once
#include <stdint.h>

// Blocks up to connectTimeoutMs at boot, then relies on the core's auto-reconnect.
void wifi_begin(const char* ssid, const char* pass, uint32_t connectTimeoutMs);
bool wifi_connected();
