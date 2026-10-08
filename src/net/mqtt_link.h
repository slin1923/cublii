#pragma once
#include <stdint.h>

// Called for every message on cube/cmd/#. Strings are NUL-terminated and only valid during the call.
typedef void (*MqttMessageHandler)(const char* topic, const char* payload);

void mqtt_begin(const char* broker, uint16_t port, const char* clientId, MqttMessageHandler handler);
void mqtt_loop();                                   // non-blocking apart from a short connect attempt
bool mqtt_connected();
bool mqtt_publish(const char* topic, const char* payload);
