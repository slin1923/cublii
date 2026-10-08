#pragma once
// Three brushed/BLDC drivers behind one brake-release line.
// Commands are signed and normalised: -1.0 (full reverse) .. +1.0 (full forward).
// The active-low PWM and the direction pin are handled inside motors.cpp.

void motors_begin();                       // pins configured, brakes ENGAGED, outputs stopped
void motors_enable(bool enable);           // true = release brakes, false = stop + engage brakes
bool motors_enabled();
void motors_set(int idx, float cmd);       // ignored while disabled
void motors_set_all(const float* cmds);    // NUM_MOTORS values
void motors_stop();                        // zero all outputs (brake state unchanged)
