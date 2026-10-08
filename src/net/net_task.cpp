#include <Arduino.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "config.h"
#include "net/net_task.h"
#include "net/telemetry.h"
#include "net/ota.h"
#include "net/mqtt_link.h"

static QueueHandle_t   s_cmdQueue = nullptr;     // created in net_task_start(); commands before that are ignored
static volatile bool   s_otaPending = false;

// Runs inside mqtt_loop(), i.e. on the network task. Only queues the command.
void net_on_mqtt(const char* topic, const char* payload) {
  if (!s_cmdQueue) return;
  const size_t prefixLen = strlen(MQTT_CMD_PREFIX);
  if (strncmp(topic, MQTT_CMD_PREFIX, prefixLen) != 0) return;

  NetCommand c = {};
  strncpy(c.key, topic + prefixLen, sizeof(c.key) - 1);
  strncpy(c.value, payload, sizeof(c.value) - 1);
  xQueueSend(s_cmdQueue, &c, 0);                 // full queue: drop rather than wait
}

void net_on_ota_start() { s_otaPending = true; }

bool net_ota_pending() { return s_otaPending; }

bool net_command_pop(NetCommand& out) {
  return s_cmdQueue && xQueueReceive(s_cmdQueue, &out, 0) == pdTRUE;
}

static void netTask(void*) {
  uint32_t lastPubMs = millis();
  for (;;) {
    ota_loop();
    mqtt_loop();                                 // may block on reconnects; only this task waits

    const uint32_t now = millis();
    if (now - lastPubMs >= TELEMETRY_PERIOD_MS) {
      lastPubMs = now;
      char buf[320];
      if (telemetry_take(buf, sizeof(buf))) mqtt_publish(MQTT_TOPIC_TELEMETRY, buf);  // window resets even if offline
    }
    delay(2);                                    // yield to the WiFi stack and the idle task
  }
}

void net_task_start() {
  s_cmdQueue = xQueueCreate(8, sizeof(NetCommand));
  xTaskCreatePinnedToCore(netTask, "net", NET_TASK_STACK_BYTES, nullptr, 1, nullptr, NET_TASK_CORE);
}
