#pragma once
#include <stdlib.h>
#include <string.h>
#include "control/controller.h"

// ============================================================================
//  YOUR CONTROLLER GOES HERE (or copy this file to make more of them, then
//  select the active one in main.cpp and re-flash).
// ============================================================================
class CustomController : public Controller {
 public:
  const char* name() const override { return "custom"; }

  void begin() override {
    // One-time setup: load gains, allocate buffers, etc.
  }

  void reset() override {
    // Clear integrators, filter state, angle estimates, ...
    // Runs at boot and every time the motors are enabled.
  }

  void update(const ControlInput& in, MotorCommand& out) override {
    for (int i = 0; i < NUM_MOTORS; i++) out.m[i] = 0.0f;
    if (!in.imuValid) return;              // no IMU, no output

    // Inputs:   in.imu.ax/ay/az (m/s^2), in.imu.gx/gy/gz (rad/s), in.dt (s), in.batteryLow,
    //           in.rpm[0..2] (signed), in.yawTarget (rad, from cube/cmd/yaw),
    //           in.ref (boot-calibration average of the IMU, valid when in.refValid):
    //             in.ref.gx/gy/gz = gyro bias  -> subtract from in.imu.g* before integrating
    //             in.ref.ax/ay/az = gravity vector at the calibrated pose (your target pose)
    // Outputs:  out.m[0..2], each in -1..+1 (sign = direction)
    //
    // TODO: estimate attitude, compute your control law, then e.g.
    //   out.m[0] = m_kp * error;
  }

  void onCommand(const char* key, const char* value) override {
    // Live gain tuning:  mosquitto_pub -t cube/cmd/kp -m 1.5
    if (strcmp(key, "kp") == 0) m_kp = atof(value);
  }

 private:
  float m_kp = 0.0f;
};
