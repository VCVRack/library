#include "DevKit.hpp"

DevKitModule::DevKitModule()
    : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  min = 0;
  max = 0;
  minimum = new TextField();
  maximum = new TextField();
  count = 0;
  cvcount = new TextField();
  cv = new SynthDevKit::CV(1.5f);
  interval = new TextField();
}

void DevKitModule::step() {
  float in = inputs[DEV_INPUT].value;

  if (min > in) {
    min = in;
  }

  if (max < in) {
    max = in;
  }

  char buf1[25];
  sprintf(buf1, "%0.5f", min);

  minimum->text = buf1;

  char buf2[25];
  sprintf(buf2, "%0.5f", max);

  maximum->text = buf2;

  cv->update(in);
  lights[BLINK_LIGHT].value = cv->isHigh() ? 1.0 : 0.0;

  if (cv->newTrigger()) {
    count++;
    char buf3[25];
    sprintf(buf3, "%d", count);
    cvcount->text = buf3;

    sprintf(buf3, "%d", cv->triggerInterval());
    interval->text = buf3;
  }
}
