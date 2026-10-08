#include <Arduino.h>
#include "config.h"
#include "hw/battery.h"

static bool     s_low = false;
static uint32_t s_mv = 0;

void battery_begin() {
  pinMode(BATT_ADC_PIN, INPUT);
  analogSetPinAttenuation(BATT_ADC_PIN, ADC_11db);   // ~0..3.1 V input range
}

void battery_update() {
  s_mv = analogReadMilliVolts(BATT_ADC_PIN);
  s_low = s_mv < BATT_LOW_MV;
}

bool battery_low() { return s_low; }
uint32_t battery_mv() { return s_mv; }
