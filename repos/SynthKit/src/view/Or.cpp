#include "../controller/Or.hpp"

struct OrWidget : ModuleWidget {
  OrWidget(OrModule *module);
};

OrWidget::OrWidget(OrModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Or.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                    OrModule::TOP1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 94), Port::INPUT, module,
                                    OrModule::TOP2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     OrModule::TOP_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 203), Port::INPUT, module,
                                    OrModule::BOTTOM1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 252), Port::INPUT, module,
                                    OrModule::BOTTOM2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 301), Port::OUTPUT, module,
                                     OrModule::BOTTOM_OUTPUT));
}

Model *modelOr =
    Model::create<OrModule, OrWidget>("SynthKit", "Or", "Or", MIXER_TAG);
