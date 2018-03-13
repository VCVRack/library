#include "../controller/Addition.hpp"

struct AdditionWidget : ModuleWidget {
  AdditionWidget(AdditionModule *module);
};

AdditionWidget::AdditionWidget(AdditionModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Addition.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                    AdditionModule::TOP1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 94), Port::INPUT, module,
                                    AdditionModule::TOP2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     AdditionModule::TOP_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 203), Port::INPUT, module,
                                    AdditionModule::BOTTOM1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 252), Port::INPUT, module,
                                    AdditionModule::BOTTOM2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 301), Port::OUTPUT, module,
                                     AdditionModule::BOTTOM_OUTPUT));
}

Model *modelAddition = Model::create<AdditionModule, AdditionWidget>(
    "SynthKit", "Addition", "Addition", MIXER_TAG);
