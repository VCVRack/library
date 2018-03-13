#include "RotatingClockDivider.hpp"

RotatingClockDividerModule::RotatingClockDividerModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  clock = new SynthDevKit::Clock(8, 1.7f);
  cv = new SynthDevKit::CV(1.7f);
  count = 0;
}

void RotatingClockDividerModule::step() {
  float in = inputs[TOP_INPUT].value;
  float trigger = inputs[ROTATE_INPUT].value;

  bool *states = clock->update(in);
  cv->update(trigger);

  bool triggered = cv->newTrigger();
  if (triggered) {
    count++;
  }

  for (int i = 0; i < 8; i++) {
    int j = i + count;
    if (j >= 8) {
      j -= 8;
    }

    if (states[i] == true) {
      outputs[j].value = in;
      lights[j].value = 1.0f;
    } else {
      outputs[j].value = 0;
      lights[j].value = 0;
    }
  }

  if (count == 8) {
    count = 0;
  }
}
