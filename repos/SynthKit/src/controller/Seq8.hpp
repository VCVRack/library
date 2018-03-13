#include <cstdint>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

struct Seq8Module : Module {
  enum ParamIds {
    OCTAVE1_PARAM,
    OCTAVE2_PARAM,
    OCTAVE3_PARAM,
    OCTAVE4_PARAM,
    OCTAVE5_PARAM,
    OCTAVE6_PARAM,
    OCTAVE7_PARAM,
    OCTAVE8_PARAM,
    SEQ1_PARAM,
    SEQ2_PARAM,
    SEQ3_PARAM,
    SEQ4_PARAM,
    SEQ5_PARAM,
    SEQ6_PARAM,
    SEQ7_PARAM,
    SEQ8_PARAM,
    NUM_PARAMS
  };
  enum InputIds { CLOCK_INPUT, NUM_INPUTS };
  enum OutputIds { GATE_OUTPUT, NUM_OUTPUTS };
  enum LightIds {
    FIRST_LED,
    SECOND_LED,
    THIRD_LED,
    FOURTH_LED,
    FIFTH_LED,
    SIXTH_LED,
    SEVENTH_LED,
    EIGHTH_LED,
    NUM_LIGHTS
  };

  Seq8Module();
  void step() override;

  SynthDevKit::CV *cv;

  float notes[12] = {0,   0.08, 0.17, 0.25, 0.33, 0.42,
                     0.5, 0.58, 0.67, 0.75, 0.83, 0.92};
  int octives[9] = {-5, -4, -3, -2, -1, 0, 1, 2, 3};
  int currentStep;
};
