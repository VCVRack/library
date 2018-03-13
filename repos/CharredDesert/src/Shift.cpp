#include <cstdint>

#include "CharredDesert.hpp"
#include "components/custom.hpp"
#include "rack.hpp"

struct ShiftModule : Module {
  enum ParamIds { SWITCH, KNOB, NUM_PARAMS };
  enum InputIds { INPUT, NUM_INPUTS };
  enum OutputIds { OUTPUT, NUM_OUTPUTS };
  enum LightIds { NUM_LIGHTS };

  ShiftModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

  void step() override;
};

void ShiftModule::step() {
  float in = inputs[INPUT].value;

  float shift = params[KNOB].value;

  if (params[SWITCH].value) {
    outputs[OUTPUT].value = clamp(in + shift, -5.0f, 5.0f);
  } else {
    outputs[OUTPUT].value = in + shift;
  }
}

struct ShiftWidget : ModuleWidget {
  ShiftWidget(ShiftModule *module);
};

ShiftWidget::ShiftWidget(ShiftModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Shift.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<Jack>(Vec(10, 45), Port::INPUT, module, ShiftModule::INPUT));
  addParam(ParamWidget::create<CKSS>(Vec(15, 112), module, ShiftModule::SWITCH,
                             0.0f, 1.0f, 0.0f));
  addParam(ParamWidget::create<Davies1900hRedKnob>(Vec(5, 165), module,
                                          ShiftModule::KNOB, -5.0f, 5.0f, 0.0f));
  addOutput(
      Port::create<Jack>(Vec(10, 230), Port::OUTPUT, module, ShiftModule::OUTPUT));
}

Model *modelShift = Model::create<ShiftModule, ShiftWidget>("CharredDesert", "Shift", "Shift");
