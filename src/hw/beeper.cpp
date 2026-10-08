#include <Arduino.h>
#include "config.h"
#include "hw/beeper.h"

static uint8_t  s_count = 0;        // beeps per burst, 0 = silent
static bool     s_repeat = false;   // burst repeats forever (alarm)
static bool     s_alarm = false;    // alarm requested; resumes after a one-shot finishes
static bool     s_on = false;
static uint32_t s_startMs = 0;

static void writeBeeper(bool on) {
  s_on = on;
  digitalWrite(BEEPER_PIN, (on == BEEPER_ACTIVE_HIGH) ? HIGH : LOW);
}

static void start(uint8_t n, bool repeat) {
  s_count = n;
  s_repeat = repeat;
  s_startMs = millis();
  writeBeeper(true);                // first beep starts immediately
}

static void stop() {
  s_count = 0;
  s_repeat = false;
  writeBeeper(false);
}

void beeper_begin() {
  pinMode(BEEPER_PIN, OUTPUT);
  writeBeeper(false);
}

void beeper_once()   { start(1, false); }
void beeper_twice()  { start(2, false); }
void beeper_thrice() { start(3, false); }

void beeper_set_alarm(bool active) {
  if (active && !s_alarm) {
    s_alarm = true;
    if (!s_count) start(3, true);
  } else if (!active && s_alarm) {
    s_alarm = false;
    if (s_repeat) stop();
  }
}

bool beeper_busy() { return s_count && !s_repeat; }

void beeper_update() {
  if (!s_count) return;
  const uint32_t period = BEEP_ON_MS + BEEP_GAP_MS;
  const uint32_t burst  = s_count * period;
  uint32_t t = millis() - s_startMs;

  if (s_repeat) {
    t %= burst + BEEP_ALARM_PAUSE_MS;
  } else if (t >= burst) {          // one-shot finished
    if (s_alarm) start(3, true);
    else stop();
    return;
  }

  const bool want = t < burst && (t % period) < BEEP_ON_MS;
  if (want != s_on) writeBeeper(want);
}
