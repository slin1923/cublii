#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "net/telemetry.h"

struct Window {
  uint32_t n, imuN, lastMs, stepMaxUs;
  float    imu[6];
  float    rpm[NUM_MOTORS];
  float    dtMax;
  float    battMv;
  float    yawTarget;
  bool     en;
};

static Window     s_win;
static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

// The critical sections below are a few microseconds of float math, so the control
// loop never waits meaningfully on the network core.
void telemetry_push(const TelemetrySample& s) {
  portENTER_CRITICAL(&s_mux);
  s_win.n++;
  s_win.lastMs = s.t_ms;
  if (s.imuValid) {
    s_win.imuN++;
    s_win.imu[0] += s.imu.ax; s_win.imu[1] += s.imu.ay; s_win.imu[2] += s.imu.az;
    s_win.imu[3] += s.imu.gx; s_win.imu[4] += s.imu.gy; s_win.imu[5] += s.imu.gz;
  }
  for (int i = 0; i < NUM_MOTORS; i++) s_win.rpm[i] += s.rpm[i];
  if (s.dt > s_win.dtMax) s_win.dtMax = s.dt;
  if (s.stepUs > s_win.stepMaxUs) s_win.stepMaxUs = s.stepUs;
  s_win.battMv += s.batteryMv;
  s_win.yawTarget = s.yawTarget;
  s_win.en = s.enabled;
  portEXIT_CRITICAL(&s_mux);
}

static void app(char* buf, size_t size, size_t& len, const char* fmt, ...) {
  if (len >= size) return;
  va_list ap;
  va_start(ap, fmt);
  const int w = vsnprintf(buf + len, size - len, fmt, ap);
  va_end(ap);
  if (w > 0) len += (size_t)w;
}

bool telemetry_take(char* out, size_t size) {
  Window w;
  portENTER_CRITICAL(&s_mux);
  w = s_win;
  s_win = Window{};
  portEXIT_CRITICAL(&s_mux);
  if (!w.n) return false;

  size_t len = 0;
  app(out, size, len, "{\"t\":%lu,\"n\":%lu", (unsigned long)w.lastMs, (unsigned long)w.n);
  if (w.imuN) {                                // no valid IMU in this window: fields are omitted
    const float k = 1.0f / w.imuN;
    app(out, size, len, ",\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,\"gx\":%.3f,\"gy\":%.3f,\"gz\":%.3f",
        w.imu[0] * k, w.imu[1] * k, w.imu[2] * k, w.imu[3] * k, w.imu[4] * k, w.imu[5] * k);
  }
  for (int i = 0; i < NUM_MOTORS; i++) app(out, size, len, ",\"rpm%d\":%.1f", i + 1, w.rpm[i] / w.n);
  app(out, size, len, ",\"batt_mv\":%.0f,\"yaw_cmd\":%.1f,\"en\":%d,\"dt_max_ms\":%.2f,\"step_max_us\":%lu}",
      w.battMv / w.n, w.yawTarget * (float)(180.0 / M_PI), w.en ? 1 : 0, w.dtMax * 1000.0f, (unsigned long)w.stepMaxUs);
  return len < size;                           // false if it didn't fit
}
