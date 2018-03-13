#include "PrimeClockDivider.hpp"

PrimeClockDividerModule::PrimeClockDividerModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  clock = new SynthDevKit::PrimeClock(8, 1.7f);
}

void PrimeClockDividerModule::step() {
  float in = inputs[TOP_INPUT].value;
  bool *states = clock->update(in);

  for (int i = 0; i < 8; i++) {
    if (states[i] == true) {
      outputs[i].value = in;
      lights[i].value = 1.0f;
    } else {
      outputs[i].value = 0;
      lights[i].value = 0;
    }
  }
}
