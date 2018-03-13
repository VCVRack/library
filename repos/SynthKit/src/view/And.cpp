#include "../controller/And.hpp"

struct AndWidget : ModuleWidget {
  AndWidget(AndModule *module);
};

AndWidget::AndWidget(AndModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/And.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 45), Port::INPUT, module,
                                    AndModule::TOP1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 95), Port::INPUT, module,
                                    AndModule::TOP2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     AndModule::TOP_OUTPUT));

  addInput(Port::create<PJ301MPort>(Vec(10, 203), Port::INPUT, module,
                                    AndModule::BOTTOM1_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(10, 252), Port::INPUT, module,
                                    AndModule::BOTTOM2_INPUT));

  addOutput(Port::create<PJ301MPort>(Vec(10, 301), Port::OUTPUT, module,
                                     AndModule::BOTTOM_OUTPUT));
}

Model *modelAnd =
    Model::create<AndModule, AndWidget>("SynthKit", "And", "And", MIXER_TAG);
