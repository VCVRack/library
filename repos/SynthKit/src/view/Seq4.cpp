#include "../controller/Seq4.hpp"
#include "../components/knobs.hpp"

struct Seq4Widget : ModuleWidget {
  Seq4Widget(Seq4Module *module);
};

Seq4Widget::Seq4Widget(Seq4Module *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Seq4.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addInput(Port::create<PJ301MPort>(Vec(10, 34), Port::INPUT, module,
                                    Seq4Module::CLOCK_INPUT));

  addParam(ParamWidget::create<SKRoundSmallWhiteSnapKnob>(
      Vec(9, 80), module, Seq4Module::OCTAVE_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<SKRoundSmallWhiteSnapKnob>(
      Vec(9, 130), module, Seq4Module::SEQ1_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<SKRoundSmallWhiteSnapKnob>(
      Vec(9, 170), module, Seq4Module::SEQ2_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<SKRoundSmallWhiteSnapKnob>(
      Vec(9, 210), module, Seq4Module::SEQ3_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<SKRoundSmallWhiteSnapKnob>(
      Vec(9, 250), module, Seq4Module::SEQ4_PARAM, 0.0, 11.0, 5.0));

  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(29, 157), module, Seq4Module::FIRST_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(29, 197), module, Seq4Module::SECOND_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(29, 237), module, Seq4Module::THIRD_LED));
  addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
      Vec(29, 277), module, Seq4Module::FOURTH_LED));

  addOutput(Port::create<PJ301MPort>(Vec(10, 310), Port::OUTPUT, module,
                                     Seq4Module::GATE_OUTPUT));
}

Model *modelSeq4 = Model::create<Seq4Module, Seq4Widget>(
    "SynthKit", "4-Step Sequencer", "4-Step Sequencer", SEQUENCER_TAG);
