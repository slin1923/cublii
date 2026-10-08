#pragma once
// Non-blocking beeper. Patterns are advanced by beeper_update(), call it every loop().
//   once / twice / thrice : play that many beeps one time (setup-time signals)
//   set_alarm(true)       : repeat the triple beep until set_alarm(false) (low battery)

void beeper_begin();
void beeper_once();
void beeper_twice();
void beeper_thrice();
void beeper_set_alarm(bool active);
bool beeper_busy();       // true while a one-shot pattern is still playing
void beeper_update();
