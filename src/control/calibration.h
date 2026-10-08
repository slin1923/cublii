#pragma once
#include <stdint.h>
#include "hw/imu.h"

// Blocking boot calibration (~13 s). Keep the cube still.
//   beep x1 -> CAL_WARNING_MS wait -> beep x2 -> average IMU for CAL_DURATION_MS -> beep x2
// idleHook (optional) is serviced during the warning wait only, never while sampling.
// Returns the number of samples averaged into `ref`, or 0 on failure (no closing beeps).
uint32_t calibration_run(ImuSample& ref, void (*idleHook)());
