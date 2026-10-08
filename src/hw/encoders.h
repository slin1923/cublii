#pragma once
// Quadrature decoding of all motor encoders on the ESP32 PCNT hardware (no interrupts, no CPU load).
// Call encoders_read() from one task only, once per control cycle.

void encoders_begin();
void encoders_read(float dt, float* rpm);   // rpm[NUM_MOTORS], signed; dt = seconds since the previous call
