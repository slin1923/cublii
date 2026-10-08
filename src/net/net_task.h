#pragma once

// Everything that touches the network (OTA, MQTT, telemetry publishing) runs in one task
// on core 0. The control loop never calls into MQTT; it only exchanges small messages with this task.

struct NetCommand {
  char key[32];       // topic after cube/cmd/
  char value[64];
};

void net_on_mqtt(const char* topic, const char* payload);   // pass to mqtt_begin()
void net_on_ota_start();                                     // pass to ota_begin()

void net_task_start();                      // call once, at the END of setup()
bool net_command_pop(NetCommand& out);      // control loop: non-blocking, false if none waiting
bool net_ota_pending();                     // true once an OTA upload has begun (motors must stay off)
