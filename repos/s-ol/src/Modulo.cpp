#include "s-ol.hpp"

struct Modulo : Module {
  enum ParamIds {
    GAIN_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SIGNAL_INPUT,
    GAIN_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    STAIR_OUTPUT,
    WRAP_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  Modulo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;
};


void Modulo::step() {
  float gain = powf(2.0, params[GAIN_PARAM].value) + inputs[GAIN_INPUT].value;
  float val = inputs[SIGNAL_INPUT].value * gain;
  float stair = floorf(val);
  float wrap = val - stair;

  outputs[STAIR_OUTPUT].value = stair / gain;;
  outputs[WRAP_OUTPUT].value = wrap * 5;
}


struct ModuloWidget : ModuleWidget {
  ModuloWidget(Modulo *module);
};

ModuloWidget::ModuloWidget(Modulo *module) : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/Modulo.svg")));

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  PJ301MPort p;
  RoundSmallBlackKnob k;
  Vec center = Vec(box.size.x, 0).minus(p.box.size).div(2);
  Vec kcenter = Vec(box.size.x, 0).minus(k.box.size).div(2);


  addInput(Port::create<PJ301MPort>(center.plus(Vec(0, 120)), Port::INPUT, module, Modulo::SIGNAL_INPUT));
  addInput(Port::create<PJ301MPort>(center.plus(Vec(-15, 180)), Port::INPUT, module, Modulo::GAIN_INPUT));

  addParam(ParamWidget::create<RoundSmallBlackKnob>(kcenter.plus(Vec(15, 180)), module, Modulo::GAIN_PARAM, -3.0, 3.0, 0.0));

  addOutput(Port::create<PJ301MPort>(center.plus(Vec(0, 265)), Port::OUTPUT, module, Modulo::STAIR_OUTPUT));
  addOutput(Port::create<PJ301MPort>(center.plus(Vec(0, 345)), Port::OUTPUT, module, Modulo::WRAP_OUTPUT));
}

Model *modelModulo = Model::create<Modulo, ModuloWidget>("s-ol", "Modulo", "Modulo", LOGIC_TAG);
