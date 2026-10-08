#pragma once
// ============================================================================
//  config.h - every pin, rate and threshold lives here. No logic.
// ============================================================================
#include <stdint.h>

// ---------------------------------------------------------------- Network ---
#define DEVICE_HOSTNAME     "esp32-cube"        // also the OTA / mDNS name
#define MQTT_BROKER         "192.168.0.150"     // my laptop running Mosquitto
#define MQTT_PORT           1883
#define MQTT_CLIENT_ID      "esp32-cube"
#define MQTT_TOPIC_STATUS   "cube/status"       // boot / calibration reports (setup only)
#define MQTT_TOPIC_TELEMETRY "cube/telemetry"   // averaged IMU + RPM, one flat JSON per report
#define MQTT_CMD_PREFIX     "cube/cmd/"         // subscribes to cube/cmd/#
#define MQTT_RETRY_MS       2000

// ----------------------------------------------------------------- Motors ---
constexpr int     NUM_MOTORS = 3;
constexpr uint8_t MOTOR_PWM_PIN[NUM_MOTORS] = { 18, 32, 25 };
constexpr uint8_t MOTOR_DIR_PIN[NUM_MOTORS] = {  5, 4,  15 };
constexpr uint8_t MOTOR_ENC_A_PIN[NUM_MOTORS] = { 17, 33, 14 };
constexpr uint8_t MOTOR_ENC_B_PIN[NUM_MOTORS] = { 16, 35, 13 };
constexpr bool    MOTOR_DIR_INVERT[NUM_MOTORS] = { false, false, false };  // flip a motor's sign
constexpr uint8_t MOTOR_BRAKE_RELEASE_PIN = 26;   // HIGH = brakes released
constexpr uint32_t MOTOR_PWM_FREQ_HZ = 20000;
constexpr uint8_t  MOTOR_PWM_BITS    = 8;
constexpr bool     MOTOR_PWM_INVERTED = true;     // drivers are active-low (duty 255 = stopped)

// --------------------------------------------------------------- Encoders ---
constexpr uint32_t ENC_PPR            = 100;              // pulses per rev, per channel
constexpr uint32_t ENC_COUNTS_PER_REV = ENC_PPR * 4;      // 4x quadrature decoding (set to ENC_PPR if 100 is total counts/rev)
constexpr bool     ENC_INVERT[NUM_MOTORS] = { false, false, false };  // flip an encoder's sign
constexpr uint16_t ENC_FILTER_CYCLES  = 200;              // glitch filter, 80 MHz cycles (200 = 2.5 us, max 1023)
constexpr int16_t  ENC_COUNT_LIMIT    = 32767;            // hardware counter range

// -------------------------------------------------------------------- IMU ---
constexpr uint8_t  IMU_SDA_PIN = 21;
constexpr uint8_t  IMU_SCL_PIN = 22;
constexpr uint32_t IMU_I2C_HZ  = 400000; // 400 kHz per fast mode datasheet spec

// ---------------------------------------------------------------- Beeper ----
constexpr uint8_t  BEEPER_PIN         = 27;
constexpr bool     BEEPER_ACTIVE_HIGH = true;     // false if the beeper sounds when the pin is LOW
constexpr uint32_t BEEP_ON_MS         = 150;      // length of one beep
constexpr uint32_t BEEP_GAP_MS        = 150;      // silence between beeps in a pattern
constexpr uint32_t BEEP_ALARM_PAUSE_MS = 800;     // silence between repeats of the low-battery triple beep

// --------------------------------------------------------------- Battery ----
constexpr uint8_t  BATT_ADC_PIN = 34;      // ADC1 - safe to use with WiFi on
constexpr uint32_t BATT_LOW_MV  = 2000;    // beep while the pin is below this (millivolts)

// ---------------------------------------------------------------- Control ---
constexpr uint32_t CONTROL_RATE_HZ        = 100;
constexpr uint32_t CONTROL_PERIOD_US      = 1000000UL / CONTROL_RATE_HZ;

// -------------------------------------------------------------- Telemetry ---
constexpr uint32_t TELEMETRY_RATE_HZ    = 50;      // averaged report rate
constexpr uint32_t TELEMETRY_PERIOD_MS  = 1000 / TELEMETRY_RATE_HZ;
constexpr int      NET_TASK_CORE        = 0;       // network core; the control loop stays on the Arduino loop core (1)
constexpr uint32_t NET_TASK_STACK_BYTES = 8192;

// ---------------------------------------------------------- Setup / Cal ----
constexpr uint32_t MQTT_SETUP_TIMEOUT_MS = 3000;   // how long setup() waits for MQTT before skipping a report
constexpr uint32_t CAL_WARNING_MS        = 5000;   // pause between the single beep and calibration start
constexpr uint32_t CAL_DURATION_MS       = 5000;   // IMU data is averaged over this long
constexpr uint32_t CAL_MIN_SAMPLES       = (CAL_DURATION_MS * CONTROL_RATE_HZ / 1000) / 2;  // fail if fewer

// Motor test controller (replaces the old blocking runMotor test)
constexpr float    MOTOR_TEST_SPEED    = 200.0f / 255.0f;  // same as old TEST_SPEED 100
constexpr uint32_t MOTOR_TEST_RUN_MS   = 1000;
constexpr uint32_t MOTOR_TEST_PAUSE_MS = 200;
