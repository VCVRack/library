#include <cstdint>

#include "CharredDesert.hpp"
#include "components/custom.hpp"

struct PanModule : Module {
  enum ParamIds { NUM_PARAMS };
  enum InputIds { AUDIO_INPUT, PAN_INPUT, NUM_INPUTS };
  enum OutputIds { AUDIO_OUTPUT1, AUDIO_OUTPUT2, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  PanModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

  void step() override;
};

void PanModule::step() {
  float audio_in = inputs[AUDIO_INPUT].value;
  float pan_in = inputs[PAN_INPUT].value;

  // figure out the percentages to apply
  float apply1 = (clamp(pan_in, -5.0f, 5.0f) + 5.0f) * 10;
  float apply2 = 100.0f - apply1;

  outputs[AUDIO_OUTPUT1].value = ((audio_in * apply1) / 100.0f);
  outputs[AUDIO_OUTPUT2].value = ((audio_in * apply2) / 100.0f);
}

struct PanWidget : ModuleWidget {
  PanWidget(PanModule *module);
};

PanWidget::PanWidget(PanModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Pan.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<Jack>(Vec(10, 45), Port::INPUT, module, PanModule::AUDIO_INPUT));
  addInput(Port::create<Jack>(Vec(10, 100), Port::INPUT, module, PanModule::PAN_INPUT));
  addOutput(Port::create<Jack>(Vec(10, 165), Port::OUTPUT, module, PanModule::AUDIO_OUTPUT1));
  addOutput(Port::create<Jack>(Vec(10, 230), Port::OUTPUT, module, PanModule::AUDIO_OUTPUT2));
}

Model *modelPan = Model::create<PanModule, PanWidget>("CharredDesert", "Pan", "Pan");
