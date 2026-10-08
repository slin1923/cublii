#pragma once
#include "control/controller.h"

// Commands nothing. The safe default.
class IdleController : public Controller {
 public:
  const char* name() const override { return "idle"; }
  void update(const ControlInput&, MotorCommand& out) override {
    for (int i = 0; i < NUM_MOTORS; i++) out.m[i] = 0.0f;
  }
};
