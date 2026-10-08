#include <Arduino.h>
#include "config.h"
#include "hw/beeper.h"
#include "hw/imu.h"
#include "control/calibration.h"

static void waitBeeps() {
  while (beeper_busy()) { beeper_update(); yield(); }
}

static void waitMs(uint32_t ms, void (*hook)()) {
  const uint32_t start = millis();
  while (millis() - start < ms) {
    if (hook) hook();
    delay(5);
  }
}

uint32_t calibration_run(ImuSample& ref, void (*idleHook)()) {
  beeper_once();                      // warning: calibration starts in CAL_WARNING_MS
  waitBeeps();
  waitMs(CAL_WARNING_MS, idleHook);

  beeper_twice();                     // calibration starts now
  waitBeeps();

  // Sample at the control rate so the average matches what the controller will see.
  float sum[6] = {};
  uint32_t n = 0;
  const uint32_t startMs = millis();
  uint32_t nextUs = micros();
  while (millis() - startMs < CAL_DURATION_MS) {
    if ((int32_t)(micros() - nextUs) < 0) { yield(); continue; }
    nextUs += CONTROL_PERIOD_US;

    ImuSample s;
    if (!imu_read(s)) continue;
    sum[0] += s.ax; sum[1] += s.ay; sum[2] += s.az;
    sum[3] += s.gx; sum[4] += s.gy; sum[5] += s.gz;
    n++;
  }
  if (n < CAL_MIN_SAMPLES) return 0;

  ref.t_ms = millis();
  ref.ax = sum[0] / n; ref.ay = sum[1] / n; ref.az = sum[2] / n;
  ref.gx = sum[3] / n; ref.gy = sum[4] / n; ref.gz = sum[5] / n;

  beeper_twice();                     // done
  waitBeeps();
  return n;
}
