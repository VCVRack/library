#include <cstdint>

#include "CharredDesert.hpp"
#include "components/custom.hpp"
#include "rack.hpp"

struct NotModule : Module {
  enum ParamIds { SWITCH, NUM_PARAMS };
  enum InputIds { INPUT, NUM_INPUTS };
  enum OutputIds { OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  NotModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

  void step() override;
};

void NotModule::step() {
  float in = inputs[INPUT].value;

  if (params[SWITCH].value == 0) {
    outputs[OUTPUT].value = -in;
  } else {
    if (in >= 1.7f) {
      outputs[OUTPUT].value = 0;
    } else {
      outputs[OUTPUT].value = 1.7f;
    }
  }
}

struct NotWidget : ModuleWidget {
  NotWidget(NotModule *module);
};

NotWidget::NotWidget(NotModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Not.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<Jack>(Vec(10, 45), Port::INPUT, module, NotModule::INPUT));
  addParam(ParamWidget::create<CKSS>(Vec(15, 112), module, NotModule::SWITCH,
                             0.0, 1.0, 1.0));
  addOutput(
      Port::create<Jack>(Vec(10, 165), Port::OUTPUT, module, NotModule::OUTPUT));
}

Model *modelNot = Model::create<NotModule, NotWidget>("CharredDesert", "Not", "Not");
