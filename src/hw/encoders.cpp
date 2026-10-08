#include <Arduino.h>
#include "driver/pcnt.h"
#include "config.h"
#include "hw/encoders.h"

static const pcnt_unit_t UNIT[3] = { PCNT_UNIT_0, PCNT_UNIT_1, PCNT_UNIT_2 };
static_assert(NUM_MOTORS <= 3, "add PCNT units for more motors");

static int16_t s_prev[NUM_MOTORS];

// One PCNT channel counts edges of `pulsePin`; `ctrlPin` decides whether each edge counts up or down.
static void addChannel(pcnt_unit_t unit, pcnt_channel_t ch, int pulsePin, int ctrlPin,
                       pcnt_ctrl_mode_t lowMode, pcnt_ctrl_mode_t highMode) {
  pcnt_config_t c = {};
  c.pulse_gpio_num = pulsePin;
  c.ctrl_gpio_num  = ctrlPin;
  c.channel        = ch;
  c.unit           = unit;
  c.pos_mode       = PCNT_COUNT_INC;      // rising edge
  c.neg_mode       = PCNT_COUNT_DEC;      // falling edge
  c.lctrl_mode     = lowMode;
  c.hctrl_mode     = highMode;
  c.counter_h_lim  = ENC_COUNT_LIMIT;
  c.counter_l_lim  = -ENC_COUNT_LIMIT;
  pcnt_unit_config(&c);
}

void encoders_begin() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    // Two channels (A edges gated by B, B edges gated by A) give 4 counts per quadrature cycle.
    addChannel(UNIT[i], PCNT_CHANNEL_0, MOTOR_ENC_A_PIN[i], MOTOR_ENC_B_PIN[i], PCNT_MODE_KEEP,    PCNT_MODE_REVERSE);
    addChannel(UNIT[i], PCNT_CHANNEL_1, MOTOR_ENC_B_PIN[i], MOTOR_ENC_A_PIN[i], PCNT_MODE_REVERSE, PCNT_MODE_KEEP);
    pcnt_set_filter_value(UNIT[i], ENC_FILTER_CYCLES);
    pcnt_filter_enable(UNIT[i]);
    pcnt_counter_pause(UNIT[i]);
    pcnt_counter_clear(UNIT[i]);
    pcnt_counter_resume(UNIT[i]);
    s_prev[i] = 0;
  }
}

void encoders_read(float dt, float* rpm) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    int16_t cur = 0;
    pcnt_get_counter_value(UNIT[i], &cur);

    // The hardware counter resets to 0 at its limit; undo that jump (reads are far more frequent than wraps).
    int32_t d = (int32_t)cur - s_prev[i];
    s_prev[i] = cur;
    if (d >  ENC_COUNT_LIMIT / 2) d -= ENC_COUNT_LIMIT;
    if (d < -ENC_COUNT_LIMIT / 2) d += ENC_COUNT_LIMIT;

    float r = (dt > 0.0f) ? ((float)d / ENC_COUNTS_PER_REV) / dt * 60.0f : 0.0f;
    rpm[i] = ENC_INVERT[i] ? -r : r;
  }
}
