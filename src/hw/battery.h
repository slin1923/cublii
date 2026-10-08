#pragma once
#include <stdint.h>
// Is the voltage on BATT_ADC_PIN below BATT_LOW_MV?

void battery_begin();
void battery_update();   // call every loop(): takes one reading
bool battery_low();
uint32_t battery_mv();   // latest raw reading at BATT_ADC_PIN (after any resistor divider)
