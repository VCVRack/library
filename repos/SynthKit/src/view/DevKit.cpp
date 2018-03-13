#include "../controller/DevKit.hpp"

struct DevKitWidget : ModuleWidget {
  DevKitWidget(DevKitModule *module);
};

DevKitWidget::DevKitWidget(DevKitModule *module) : ModuleWidget(module) {
  box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/DevKit.svg")));
    addChild(panel);
  }

  module->minimum->box.size = Vec(60, 20);
  module->maximum->box.size = Vec(60, 20);
  module->minimum->box.pos = Vec(14, 78);
  module->maximum->box.pos = Vec(14, 126);

  module->minimum->text = "minimum";
  module->maximum->text = "maximum";

  addChild(module->minimum);
  addChild(module->maximum);

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(
      Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(
      Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(Vec(
      box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  addChild(ModuleLightWidget::create<MediumLight<RedLight>>(
      Vec(40, 173), module, DevKitModule::BLINK_LIGHT));

  module->cvcount->box.size = Vec(60, 20);
  module->cvcount->box.pos = Vec(14, 214);
  module->cvcount->text = "count";

  addChild(module->cvcount);

  module->interval->box.size = Vec(60, 20);
  module->interval->box.pos = Vec(14, 264);
  module->interval->text = "interval";

  addChild(module->interval);

  addInput(Port::create<PJ301MPort>(Vec(33, 34), Port::INPUT, module,
                                    DevKitModule::DEV_INPUT));
}

Model *modelDevKit = Model::create<DevKitModule, DevKitWidget>(
    "SynthKit", "DevKit", "DevKit", UTILITY_TAG);
