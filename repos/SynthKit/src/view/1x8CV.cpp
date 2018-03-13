#include "../controller/1x8CV.hpp"

struct M1x8CVWidget : ModuleWidget {
  M1x8CVWidget(M1x8CVModule *module);
};

M1x8CVWidget::M1x8CVWidget(M1x8CVModule *module) : ModuleWidget(module) {
  box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/1x8CV.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(
      Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(Vec(
      box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(33, 34), Port::INPUT, module,
                                    M1x8CVModule::TOP_INPUT));

  addInput(Port::create<PJ301MPort>(Vec(13, 73), Port::INPUT, module,
                                    M1x8CVModule::FIRST_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 73), Port::OUTPUT, module,
                                     M1x8CVModule::FIRST_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 81), module, M1x8CVModule::FIRST_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 108), Port::INPUT, module,
                                    M1x8CVModule::SECOND_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 108), Port::OUTPUT, module,
                                     M1x8CVModule::SECOND_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 116), module, M1x8CVModule::SECOND_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 143), Port::INPUT, module,
                                    M1x8CVModule::THIRD_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 143), Port::OUTPUT, module,
                                     M1x8CVModule::THIRD_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 151), module, M1x8CVModule::THIRD_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 178), Port::INPUT, module,
                                    M1x8CVModule::FOURTH_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 178), Port::OUTPUT, module,
                                     M1x8CVModule::FOURTH_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 186), module, M1x8CVModule::FOURTH_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 213), Port::INPUT, module,
                                    M1x8CVModule::FIFTH_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 213), Port::OUTPUT, module,
                                     M1x8CVModule::FIFTH_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 221), module, M1x8CVModule::FIFTH_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 248), Port::INPUT, module,
                                    M1x8CVModule::SIXTH_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 248), Port::OUTPUT, module,
                                     M1x8CVModule::SIXTH_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 256), module, M1x8CVModule::SIXTH_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 283), Port::INPUT, module,
                                    M1x8CVModule::SEVENTH_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 283), Port::OUTPUT, module,
                                     M1x8CVModule::SEVENTH_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 291), module, M1x8CVModule::SEVENTH_LED));

  addInput(Port::create<PJ301MPort>(Vec(13, 318), Port::INPUT, module,
                                    M1x8CVModule::EIGHTH_CV));
  addOutput(Port::create<PJ301MPort>(Vec(53, 318), Port::OUTPUT, module,
                                     M1x8CVModule::EIGHTH_OUTPUT));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(41, 326), module, M1x8CVModule::EIGHTH_LED));
}

Model *modelM1x8CV = Model::create<M1x8CVModule, M1x8CVWidget>(
    "SynthKit", "1x8 Splitter (CV)", "1x8 Splitter (CV)", MIXER_TAG);
