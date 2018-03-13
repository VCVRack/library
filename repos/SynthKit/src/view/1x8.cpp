#include "../controller/1x8.hpp"

struct M1x8Widget : ModuleWidget {
  M1x8Widget(M1x8Module *module);
};

M1x8Widget::M1x8Widget(M1x8Module *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/1x8.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 34), Port::INPUT, module,
                                    M1x8Module::TOP_INPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 73), Port::OUTPUT, module,
                                     M1x8Module::FIRST_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 108), Port::OUTPUT, module,
                                     M1x8Module::SECOND_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     M1x8Module::THIRD_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 178), Port::OUTPUT, module,
                                     M1x8Module::FOURTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 213), Port::OUTPUT, module,
                                     M1x8Module::FIFTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 248), Port::OUTPUT, module,
                                     M1x8Module::SIXTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 283), Port::OUTPUT, module,
                                     M1x8Module::SEVENTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 318), Port::OUTPUT, module,
                                     M1x8Module::EIGHTH_OUTPUT));
}

Model *modelM1x8 = Model::create<M1x8Module, M1x8Widget>(
    "SynthKit", "1x8 Splitter", "1x8 Splitter", MIXER_TAG);
