#include "../controller/ClockDivider.hpp"

struct ClockDividerWidget : ModuleWidget {
  ClockDividerWidget(ClockDividerModule *module);
};

ClockDividerWidget::ClockDividerWidget(ClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/ClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 34), Port::INPUT, module,
                                    ClockDividerModule::TOP_INPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 73), Port::OUTPUT, module,
                                     ClockDividerModule::FIRST_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 108), Port::OUTPUT, module,
                                     ClockDividerModule::SECOND_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     ClockDividerModule::THIRD_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 178), Port::OUTPUT, module,
                                     ClockDividerModule::FOURTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 213), Port::OUTPUT, module,
                                     ClockDividerModule::FIFTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 248), Port::OUTPUT, module,
                                     ClockDividerModule::SIXTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 283), Port::OUTPUT, module,
                                     ClockDividerModule::SEVENTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 318), Port::OUTPUT, module,
                                     ClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 81), module, ClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 116), module, ClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 151), module, ClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 186), module, ClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 221), module, ClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 256), module, ClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 291), module, ClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 326), module, ClockDividerModule::EIGHTH_LED));
}

Model *modelClockDivider =
    Model::create<ClockDividerModule, ClockDividerWidget>(
        "SynthKit", "Clock Divider", "Clock Divider", UTILITY_TAG, CLOCK_TAG);
