#pragma once
#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "hw/imu.h"

// One control-cycle snapshot, produced by the control loop.
struct TelemetrySample {
  uint32_t  t_ms;
  bool      imuValid;
  ImuSample imu;
  float     rpm[NUM_MOTORS];
  float     dt;            // measured cycle time (s)
  uint32_t  stepUs;        // how long the control step itself took
  bool      enabled;
  uint32_t  batteryMv;     // raw reading at the battery sense pin
  float     yawTarget;     // commanded yaw (rad)
};

// Control task: adds a sample to the current window. Never blocks on the network.
void telemetry_push(const TelemetrySample& s);

// Network task: closes the window and writes ONE flat JSON of averages into `out`
// (max values for the timing fields). Returns false if the window was empty.
bool telemetry_take(char* out, size_t size);
