#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "hw/motors.h"

static bool s_enabled = false;
static const uint32_t MAX_DUTY = (1u << MOTOR_PWM_BITS) - 1;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  // Arduino-ESP32 core 3.x: LEDC is addressed by pin
  static void pwmAttach(int idx) { ledcAttach(MOTOR_PWM_PIN[idx], MOTOR_PWM_FREQ_HZ, MOTOR_PWM_BITS); }
  static void pwmWrite(int idx, uint32_t duty) { ledcWrite(MOTOR_PWM_PIN[idx], duty); }
#else
  // Arduino-ESP32 core 2.x: LEDC is addressed by channel (channel == motor index)
  static void pwmAttach(int idx) {
    ledcSetup(idx, MOTOR_PWM_FREQ_HZ, MOTOR_PWM_BITS);
    ledcWrite(idx, MOTOR_PWM_INVERTED ? MAX_DUTY : 0);   // stopped level before the pin is attached
    ledcAttachPin(MOTOR_PWM_PIN[idx], idx);
  }
  static void pwmWrite(int idx, uint32_t duty) { ledcWrite(idx, duty); }
#endif

static void writeMotor(int idx, float cmd) {
  if (!(cmd == cmd)) cmd = 0.0f;            // NaN -> stop
  if (cmd > 1.0f)  cmd = 1.0f;
  if (cmd < -1.0f) cmd = -1.0f;

  bool forward = cmd >= 0.0f;
  if (MOTOR_DIR_INVERT[idx]) forward = !forward;

  uint32_t duty = (uint32_t)(fabsf(cmd) * MAX_DUTY + 0.5f);
  if (MOTOR_PWM_INVERTED) duty = MAX_DUTY - duty;

  digitalWrite(MOTOR_DIR_PIN[idx], forward ? HIGH : LOW);
  pwmWrite(idx, duty);
}

void motors_stop() {
  for (int i = 0; i < NUM_MOTORS; i++) writeMotor(i, 0.0f);
}

void motors_begin() {
  // Brakes first, so nothing can spin while the PWM peripherals are being set up.
  pinMode(MOTOR_BRAKE_RELEASE_PIN, OUTPUT);
  digitalWrite(MOTOR_BRAKE_RELEASE_PIN, LOW);

  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(MOTOR_DIR_PIN[i], OUTPUT);
    pwmAttach(i);
  }
  motors_stop();
  s_enabled = false;
}

void motors_enable(bool enable) {
  if (enable == s_enabled) return;
  if (enable) {
    motors_stop();                                        // start from zero
    digitalWrite(MOTOR_BRAKE_RELEASE_PIN, HIGH);
    s_enabled = true;
  } else {
    s_enabled = false;
    motors_stop();
    digitalWrite(MOTOR_BRAKE_RELEASE_PIN, LOW);
  }
}

bool motors_enabled() { return s_enabled; }

void motors_set(int idx, float cmd) {
  if (!s_enabled || idx < 0 || idx >= NUM_MOTORS) return;
  writeMotor(idx, cmd);
}

void motors_set_all(const float* cmds) {
  for (int i = 0; i < NUM_MOTORS; i++) motors_set(i, cmds[i]);
}
