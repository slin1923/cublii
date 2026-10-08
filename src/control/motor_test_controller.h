#pragma once
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include "control/controller.h"

// Non-blocking motor test: spins all motors forward together for MOTOR_TEST_RUN_MS,
// stops for MOTOR_TEST_PAUSE_MS, and repeats.
// Live tweak:  mosquitto_pub -t cube/cmd/speed -m 0.3
class MotorTestController : public Controller {
 public:
  const char* name() const override { return "motor_test"; }

  void reset() override {
    m_running = true;
    m_phaseStartMs = millis();
  }

  void update(const ControlInput& in, MotorCommand& out) override {
    const uint32_t elapsed = in.nowMs - m_phaseStartMs;
    if (m_running ? elapsed >= MOTOR_TEST_RUN_MS : elapsed >= MOTOR_TEST_PAUSE_MS) {
      m_running = !m_running;
      m_phaseStartMs = in.nowMs;
    }

    for (int i = 0; i < NUM_MOTORS; i++) out.m[i] = m_running ? m_speed : 0.0f;
  }

  void onCommand(const char* key, const char* value) override {
    if (strcmp(key, "speed") == 0) m_speed = atof(value);
  }

 private:
  bool     m_running = false;
  uint32_t m_phaseStartMs = 0;
  float    m_speed = MOTOR_TEST_SPEED;
};
