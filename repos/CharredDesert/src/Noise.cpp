#include <cstdint>

#include "../deps/SynthDevKit/src/CV.hpp"
#include "../deps/SynthDevKit/src/PinkNoise.hpp"
#include "../deps/SynthDevKit/src/WhiteNoise.hpp"
#include "CharredDesert.hpp"
#include "components/custom.hpp"
#include "rack.hpp"

struct NoiseModule : Module {
  enum ParamIds { NOISE_SWITCH, NUM_PARAMS };
  enum InputIds { CV_INPUT, NUM_INPUTS };
  enum OutputIds { AUDIO_OUTPUT, NUM_OUTPUTS };
  enum LightIds { ON_LED, NUM_LIGHTS };

  NoiseModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    wn = new SynthDevKit::WhiteNoise(0);
    pn = new SynthDevKit::PinkNoise(0);
    cv = new SynthDevKit::CV(1.7f);
  }

  void step() override;

  SynthDevKit::WhiteNoise *wn;
  SynthDevKit::PinkNoise *pn;
  SynthDevKit::CV *cv;
};

void NoiseModule::step() {
  float cv_in = inputs[CV_INPUT].value;

  cv->update(cv_in);

  if (cv->isHigh()) {
    if (params[NOISE_SWITCH].value) {
      outputs[AUDIO_OUTPUT].value = pn->stepValue();
    } else {
      outputs[AUDIO_OUTPUT].value = wn->stepValue();
    }

    lights[ON_LED].value = 1;
  } else {
    outputs[AUDIO_OUTPUT].value = 0;
    lights[ON_LED].value = 0;
  }
}

struct NoiseWidget : ModuleWidget {
  NoiseWidget(NoiseModule *module);
};

NoiseWidget::NoiseWidget(NoiseModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Noise.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<Jack>(Vec(10, 45), Port::INPUT, module, NoiseModule::CV_INPUT));
  addParam(ParamWidget::create<CKSS>(Vec(15, 112), module, NoiseModule::NOISE_SWITCH,
                             0.0, 1.0, 1.0));
  addOutput(
      Port::create<Jack>(Vec(10, 165), Port::OUTPUT, module, NoiseModule::AUDIO_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(18, 220), module,
                                              NoiseModule::ON_LED));
}

Model *modelNoise = Model::create<NoiseModule, NoiseWidget>("CharredDesert", "Noise", "Noise");
