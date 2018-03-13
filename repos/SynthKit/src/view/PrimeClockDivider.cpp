#include "../controller/PrimeClockDivider.hpp"

struct PrimeClockDividerWidget : ModuleWidget {
  PrimeClockDividerWidget(PrimeClockDividerModule *module);
};

PrimeClockDividerWidget::PrimeClockDividerWidget(
    PrimeClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/PrimeClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 34), Port::INPUT, module,
                                    PrimeClockDividerModule::TOP_INPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 73), Port::OUTPUT, module,
                                     PrimeClockDividerModule::FIRST_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 108), Port::OUTPUT, module,
                                     PrimeClockDividerModule::SECOND_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 143), Port::OUTPUT, module,
                                     PrimeClockDividerModule::THIRD_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 178), Port::OUTPUT, module,
                                     PrimeClockDividerModule::FOURTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 213), Port::OUTPUT, module,
                                     PrimeClockDividerModule::FIFTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 248), Port::OUTPUT, module,
                                     PrimeClockDividerModule::SIXTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 283), Port::OUTPUT, module,
                                     PrimeClockDividerModule::SEVENTH_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(10, 318), Port::OUTPUT, module,
                                     PrimeClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 81), module, PrimeClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 116), module, PrimeClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 151), module, PrimeClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 186), module, PrimeClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 221), module, PrimeClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 256), module, PrimeClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 291), module, PrimeClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(35, 326), module, PrimeClockDividerModule::EIGHTH_LED));
}

Model *modelPrimeClockDivider =
    Model::create<PrimeClockDividerModule, PrimeClockDividerWidget>(
        "SynthKit", "Prime Clock Divider", "Prime Clock Divider", UTILITY_TAG,
        CLOCK_TAG);
