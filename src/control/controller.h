#pragma once
#include <stdint.h>
#include "config.h"
#include "hw/imu.h"

// Everything a controller is allowed to see each cycle.
struct ControlInput {
  ImuSample imu;        // only meaningful when imuValid is true
  bool      imuValid;
  float     yawTarget;        // commanded yaw to track, radians (set with cube/cmd/yaw, in degrees)
  float     rpm[NUM_MOTORS];  // signed motor speed from the encoders
  ImuSample ref;        // average IMU reading from the boot calibration (the target reference)
  bool      refValid;   // false if calibration was skipped or failed
  float     dt;         // seconds since the previous update (measured, not nominal)
  uint32_t  nowMs;
  bool      batteryLow;
};

// What a controller is allowed to ask for: one normalised command per motor, -1..+1.
// main.cpp zeroes this before every update() and applies it only if the motors are enabled.
struct MotorCommand {
  float m[NUM_MOTORS];
};

class Controller {
 public:
  virtual ~Controller() {}
  virtual const char* name() const = 0;

  virtual void begin() {}                    // once at boot
  virtual void reset() {}                    // when selected, and each time motors are enabled
  virtual void update(const ControlInput& in, MotorCommand& out) = 0;   // every control cycle

  // Live tuning over MQTT: topic cube/cmd/<key> with <value> as payload.
  // Runs on the control loop (not the network task), so gains need no locking.
  // "enable" is consumed by main.cpp; everything else lands here.
  virtual void onCommand(const char* key, const char* value) {}
};
