#include "Seq8.hpp"

Seq8Module::Seq8Module()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  cv = new SynthDevKit::CV(1.7f);
  currentStep = 0;
}

void Seq8Module::step() {
  float in = inputs[CLOCK_INPUT].value;
  cv->update(in);

  if (cv->newTrigger()) {
    for (int i = 0; i < 8; i++) {
      if (i == currentStep) {
        float note = octives[(int)params[i].value] +
                     notes[(int)params[i + currentStep + 1].value];
        outputs[GATE_OUTPUT].value = note;
        lights[i].value = 1.0;
      } else {
        lights[i].value = 0.0;
      }
    }
    currentStep++;

    if (currentStep == 8) {
      currentStep = 0;
    }
  }
}
