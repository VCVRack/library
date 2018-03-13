#include <cstdint>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "../deps/SynthDevKit/src/DTMF.hpp"
#include "CharredDesert.hpp"
#include "components/custom.hpp"

struct DTMFModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { VOCT_INPUT, CV_INPUT, NUM_INPUTS };
  enum OutputIds { AUDIO_OUTPUT, NUM_OUTPUTS };
  enum LightIds { ON_LED, NUM_LIGHTS };

  DTMFModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    cv = new SynthDevKit::CV(1.7f);
    dtmf = new SynthDevKit::DTMF(44100);
  }

  void step() override;

  char getTone(float);
  SynthDevKit::CV *cv;
  SynthDevKit::DTMF *dtmf;
  float notes[16] = {0,    0.08, 0.17, 0.25, 0.33, 0.42, 0.5,  0.58,
                     0.67, 0.75, 0.83, 0.92, 1.0,  1.08, 1.17, 1.25};
  char tones[16] = {'1', '2', '3', 'A', '4', '5', '6', 'B',
                    '7', '8', '9', 'C', '*', '0', '#', 'D'};
};

char DTMFModule::getTone(float current) {
  for (int i = 0; i < 12; i++) {
    if ((notes[i] - 0.02) <= current && (notes[i] + 0.02) >= current) {
      return tones[i];
    }
  }

  return ' ';
}

void DTMFModule::step() {
  float cv_in = inputs[CV_INPUT].value;
  float voct_in = inputs[VOCT_INPUT].value;

  cv->update(cv_in);

  if (cv->newTrigger()) {
    dtmf->reset();
  }

  if (cv->isHigh()) {
    char tone = getTone(voct_in);
    dtmf->setTone(tone);

    outputs[AUDIO_OUTPUT].value = dtmf->stepValue();

    if (outputs[AUDIO_OUTPUT].value == 0) {
      lights[ON_LED].value = 0;
    } else {
      lights[ON_LED].value = 1;
    }
  } else {
    outputs[AUDIO_OUTPUT].value = 0;
    lights[ON_LED].value = 0;
  }
}

struct DTMFWidget : ModuleWidget {
  DTMFWidget(DTMFModule *module);
};

DTMFWidget::DTMFWidget(DTMFModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/DTMF.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<Jack>(Vec(10, 45), Port::INPUT, module, DTMFModule::CV_INPUT));
  addInput(Port::create<Jack>(Vec(10, 100), Port::INPUT, module, DTMFModule::VOCT_INPUT));
  addOutput(Port::create<Jack>(Vec(10, 165), Port::OUTPUT, module, DTMFModule::AUDIO_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(18, 220), module,
                                              DTMFModule::ON_LED));
}

Model *modelDTMF = Model::create<DTMFModule, DTMFWidget>("CharredDesert", "DTMF", "DTMF");
