// ============================================================================
//  Cube firmware - main.cpp
//  Only wiring lives here: boot order, calibration, the fixed-rate control
//  loop, and command routing.
//
//  Two cores:
//    core 1 (loop)      control loop only: IMU, encoders, controller, motors
//    core 0 (net task)  OTA, MQTT, and the averaged telemetry report
//  The control loop never touches the network; it hands samples to
//  telemetry_push() and receives commands through a queue.
//
//  Beeper codes:
//    1 beep  -> calibration starts in 5 s
//    2 beeps -> calibration started / finished
//    3 beeps, repeating -> battery low
// ============================================================================
#include <Arduino.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <strings.h>

#include "config.h"
#include "secrets.h"    // where my private wifi password lives, not checked into git

#include "net/wifi_manager.h"
#include "net/ota.h"
#include "net/mqtt_link.h"
#include "net/net_task.h"
#include "net/telemetry.h"

#include "hw/motors.h"
#include "hw/imu.h"
#include "hw/encoders.h"
#include "hw/battery.h"
#include "hw/beeper.h"

#include "control/controller.h"
#include "control/calibration.h"
#include "control/idle_controller.h"
#include "control/motor_test_controller.h"
#include "control/custom_controller.h"

// ----------------------------------------------------------- Controllers ----
static IdleController      idleCtrl;
static MotorTestController motorTestCtrl;
static CustomController    customCtrl;

// To change the controller: edit the line below, rebuild, and re-flash (OTA).
//   Options: &idleCtrl, &motorTestCtrl, &customCtrl
static Controller* const active = &motorTestCtrl;

// ----------------------------------------------------------------- State ----
static bool      imuOk = false;
static bool      calibrated = false;
static ImuSample imuRef = {};
static float     yawTargetRad = 0.0f;   // relative to the yaw at boot; set via cube/cmd/yaw (degrees)
static uint32_t  lastControlUs = 0;

// -------------------------------------------------------------- Commands ----
static bool parseBool(const char* s) {
  return s[0] == '1' || strcasecmp(s, "on") == 0 || strcasecmp(s, "true") == 0;
}

// Runs on the control loop, fed by the network task's queue.
static void handleCommand(const char* key, const char* value) {
  if (strcmp(key, "enable") == 0) {
    const bool en = parseBool(value) && !net_ota_pending();
    if (en) active->reset();
    motors_enable(en);
  } else if (strcmp(key, "yaw") == 0) {
    const float deg = atof(value);
    if (isfinite(deg)) yawTargetRad = deg * (float)(M_PI / 180.0);
  } else {
    active->onCommand(key, value);
  }
}

// ---------------------------------------------------------- Setup reports ---
// Wait briefly for MQTT; skipped immediately if WiFi is down.
static bool connectMqtt(uint32_t timeoutMs) {
  const uint32_t start = millis();
  while (!mqtt_connected() && wifi_connected() && (millis() - start) < timeoutMs) {
    ota_loop();
    mqtt_loop();
    delay(20);
  }
  return mqtt_connected();
}

static void report(const char* json) {
  if (connectMqtt(MQTT_SETUP_TIMEOUT_MS)) mqtt_publish(MQTT_TOPIC_STATUS, json);
}

static void reportBoot() {
  char p[96];
  snprintf(p, sizeof(p), "{\"event\":\"boot\",\"ctrl\":\"%s\",\"imu\":%d,\"batt_low\":%d}",
           active->name(), imuOk ? 1 : 0, battery_low() ? 1 : 0);
  report(p);
}

static void calibrateAndReport() {
  const uint32_t n = calibration_run(imuRef, ota_loop);
  calibrated = n > 0;

  char p[200];
  if (calibrated) {
    snprintf(p, sizeof(p),
             "{\"event\":\"calibrated\",\"n\":%lu,\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,"
             "\"gx\":%.3f,\"gy\":%.3f,\"gz\":%.3f}",
             (unsigned long)n, imuRef.ax, imuRef.ay, imuRef.az, imuRef.gx, imuRef.gy, imuRef.gz);
  } else {
    snprintf(p, sizeof(p), "{\"event\":\"cal_failed\"}");
  }
  report(p);
}

// --------------------------------------------------------- Control cycle ----
static void controlStep(float dt) {
  const uint32_t t0 = micros();

  ControlInput in = {};
  in.nowMs      = millis();
  in.dt         = dt;
  in.imuValid   = imuOk && imu_read(in.imu);
  encoders_read(dt, in.rpm);
  in.yawTarget  = yawTargetRad;
  in.ref        = imuRef;
  in.refValid   = calibrated;
  in.batteryLow = battery_low();

  MotorCommand cmd = {};                       // all zeros unless the controller says otherwise
  active->update(in, cmd);
  if (motors_enabled()) motors_set_all(cmd.m);

  TelemetrySample s = {};
  s.t_ms = in.nowMs;
  s.imuValid = in.imuValid;
  s.imu = in.imu;
  for (int i = 0; i < NUM_MOTORS; i++) s.rpm[i] = in.rpm[i];
  s.dt = dt;
  s.enabled = motors_enabled();
  s.batteryMv = battery_mv();
  s.yawTarget = yawTargetRad;
  s.stepUs = micros() - t0;
  telemetry_push(s);                           // just accumulates; the network task does the sending
}

// ------------------------------------------------------------ Arduino API ---
void setup() {
  motors_begin();                              // first: brakes on, outputs stopped
  encoders_begin();
  beeper_begin();
  battery_begin();
  battery_update();

  imuOk = imu_begin();

  wifi_begin(WIFI_SSID, WIFI_PASS, 15000);     // from secrets.h, 15 s timeout
  ota_begin(DEVICE_HOSTNAME, net_on_ota_start);
  mqtt_begin(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, net_on_mqtt);

  active->begin();
  active->reset();

  reportBoot();
  if (imuOk) calibrateAndReport();             // no IMU: skipped, boot report already says imu=0

  net_task_start();                            // from here on, only the net task uses MQTT/OTA
  lastControlUs = micros();                    // motors stay DISABLED until cube/cmd/enable = 1
}

void loop() {
  battery_update();
  beeper_set_alarm(battery_low());
  beeper_update();
  // Optional failsafe: if (battery_low()) motors_enable(false);

  // OTA runs on the net core while this loop keeps going, so keep the motors off.
  if (net_ota_pending()) motors_enable(false);

  NetCommand nc;
  while (net_command_pop(nc)) handleCommand(nc.key, nc.value);

  // Fixed-rate control cycle.
  const uint32_t nowUs = micros();
  const uint32_t elapsedUs = nowUs - lastControlUs;
  if (elapsedUs >= CONTROL_PERIOD_US) {
    lastControlUs = nowUs;
    controlStep(elapsedUs * 1e-6f);
  }
}
