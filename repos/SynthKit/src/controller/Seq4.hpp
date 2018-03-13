#include <cstdint>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

struct Seq4Module : Module {
  enum ParamIds {
    OCTAVE_PARAM,
    SEQ1_PARAM,
    SEQ2_PARAM,
    SEQ3_PARAM,
    SEQ4_PARAM,
    NUM_PARAMS
  };
  enum InputIds { CLOCK_INPUT, NUM_INPUTS };
  enum OutputIds { GATE_OUTPUT, NUM_OUTPUTS };
  enum LightIds { FIRST_LED, SECOND_LED, THIRD_LED, FOURTH_LED, NUM_LIGHTS };

  Seq4Module();
  void step() override;

  SynthDevKit::CV *cv;

  float notes[12] = {0,   0.08, 0.17, 0.25, 0.33, 0.42,
                     0.5, 0.58, 0.67, 0.75, 0.83, 0.92};
  int octives[9] = {-5, -4, -3, -2, -1, 0, 1, 2, 3};
  int currentStep;
};
