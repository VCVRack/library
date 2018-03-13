#include "../controller/Subtraction.hpp"

struct SubtractionWidget : ModuleWidget {
  SubtractionWidget(SubtractionModule *module);
};

SubtractionWidget::SubtractionWidget(SubtractionModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Subtraction.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                    SubtractionModule::TOP1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 94), Port::INPUT, module,
                                    SubtractionModule::TOP2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     SubtractionModule::TOP_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 203), Port::INPUT, module,
                                    SubtractionModule::BOTTOM1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 252), Port::INPUT, module,
                                    SubtractionModule::BOTTOM2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 301), Port::OUTPUT, module,
                                     SubtractionModule::BOTTOM_OUTPUT));
}

Model *modelSubtraction = Model::create<SubtractionModule, SubtractionWidget>(
    "SynthKit", "Subtraction", "Subtraction", MIXER_TAG);
