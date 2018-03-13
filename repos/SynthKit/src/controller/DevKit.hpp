#include <cstdio>

#include "../../deps/SynthDevKit/src/CV.hpp"
#include "../SynthKit.hpp"

struct DevKitModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { DEV_INPUT, NUM_INPUTS };
  enum OutputIds { NUM_OUTPUTS };
  enum LightIds { BLINK_LIGHT, NUM_LIGHTS };

  DevKitModule();
  void step() override;
  TextField *minimum;
  TextField *maximum;
  float min, max;
  int count;
  TextField *cvcount;
  SynthDevKit::CV *cv;
  TextField *interval;
};
