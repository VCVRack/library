#include "s-ol.hpp"

struct WrapComp : Module {
  enum ParamIds {
    A_OUT_PARAM,
    B_OUT_PARAM,
    OFFSET_PARAM,
    RANGE_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SIGNAL_INPUT,
    OFFSET_INPUT,
    RANGE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    MAIN_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    A_LIGHT,
    B_LIGHT,
    NUM_LIGHTS
  };

  WrapComp() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;
};


void WrapComp::step() {
  float input = inputs[SIGNAL_INPUT].value + inputs[OFFSET_INPUT].normalize(0.0) + params[OFFSET_PARAM].value;
  float width = clamp((inputs[RANGE_INPUT].normalize(0.0f) + params[RANGE_PARAM].value) / 2.0f, 0.0f, 5.0f);

  while (input > 5.0)
    input -= 5.0;
  while (input < 0.0)
    input += 5.0;

  bool a = input < width || input > (5.0 - width);

  outputs[MAIN_OUTPUT].value = a ? params[A_OUT_PARAM].value : params[B_OUT_PARAM].value;
  lights[A_LIGHT].value = a ? 1.0 : 0.0;
  lights[B_LIGHT].value = a ? 0.0 : 1.0;
}


struct WrapCompWidget : ModuleWidget {
  WrapCompWidget(WrapComp *module);
};

WrapCompWidget::WrapCompWidget(WrapComp *module) : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/WrapComp.svg")));

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  PJ301MPort p;
  RoundSmallBlackKnob k;
  MediumLight<RedLight> l;
  Vec center = Vec(box.size.x, 0).minus(p.box.size).div(2);
  Vec kcenter = Vec(box.size.x, 0).minus(k.box.size).div(2);
  Vec lcenter = Vec(box.size.x, 0).minus(l.box.size).div(2);

  addInput(Port::create<PJ301MPort>(center.plus(Vec(0, 70)), Port::INPUT, module, WrapComp::SIGNAL_INPUT));

  addParam(ParamWidget::create<RoundSmallBlackKnob>(kcenter.plus(Vec(25, 135)), module, WrapComp::OFFSET_PARAM, -5.0, 5.0, 0.0));
  addInput(Port::create<PJ301MPort>(center.plus(Vec(-25, 135)), Port::INPUT, module, WrapComp::OFFSET_INPUT));

  addParam(ParamWidget::create<RoundSmallBlackKnob>(kcenter.plus(Vec(25, 210)), module, WrapComp::RANGE_PARAM, -5.0, 5.0, 0.0));
  addInput(Port::create<PJ301MPort>(center.plus(Vec(-25, 210)), Port::INPUT, module, WrapComp::RANGE_INPUT));

  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(lcenter.plus(Vec(-15, 255)), module, WrapComp::A_LIGHT));
  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(lcenter.plus(Vec( 15, 255)), module, WrapComp::B_LIGHT));

  addParam(ParamWidget::create<RoundSmallBlackKnob>(kcenter.plus(Vec(-25, 280)), module, WrapComp::A_OUT_PARAM, -8.0, 8.0, 1.0));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(kcenter.plus(Vec( 25, 280)), module, WrapComp::B_OUT_PARAM, -8.0, 8.0, 0.0));

  addOutput(Port::create<PJ301MPort>(center.plus(Vec(0, 315)), Port::OUTPUT, module, WrapComp::MAIN_OUTPUT));
}

Model *modelWrapComp = Model::create<WrapComp, WrapCompWidget>("s-ol", "WrapComp", "Wrapping Comparator", LOGIC_TAG);
