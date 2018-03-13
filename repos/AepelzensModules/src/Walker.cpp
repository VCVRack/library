#include "aepelzen.hpp"
#include "dsp/digital.hpp"

struct Walker : Module
{
  enum ParamIds
    {
      STEP_PARAM,
      STEP_ATT_PARAM,
      SYM_PARAM,
      RANGE_PARAM,
      RANGE_ATT_PARAM,
      RANGE_MODE_PARAM,
      NUM_PARAMS
    };
  enum InputIds
    {
      CLOCK_INPUT,
      STEP_INPUT,
      RANGE_INPUT,
      NUM_INPUTS
    };
  enum OutputIds
    {
      CV_OUTPUT,
      NUM_OUTPUTS
    };

  enum LightIds
    {
      NUM_LIGHTS
    };

  void step() override;

  Walker() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  }

  void reset() override {}

  SchmittTrigger clockTrigger;
  float stepsize, range, sym, cvout;
  int mode = 1;
};

void Walker::step()
{
  stepsize = clampf(params[STEP_PARAM].value + inputs[STEP_INPUT].value/5.0 * params[STEP_ATT_PARAM].value, 0.0, 1.0);
  range = clampf(params[RANGE_PARAM].value + inputs[RANGE_INPUT].value * params[RANGE_ATT_PARAM].value, 0.0, 5.0);
  mode = (int)params[RANGE_MODE_PARAM].value;
  
  if( clockTrigger.process(inputs[CLOCK_INPUT].value) ) {
    float rand = randomf();
    cvout += (rand > 0.5 - params[SYM_PARAM].value / 2.0) ? + stepsize : -stepsize;
    if(cvout > range || cvout < -range) {
      switch (mode) {
      case 1:
	cvout = clampf(cvout, -range, range);
	break;
      case 2:
	cvout = 0;
	break;
      case 3:
	cvout = randomf() * range/2;
	if (randomf() < 0.5 - params[SYM_PARAM].value / 2.0)
	  cvout = -cvout;
	break;
      }
    }
  }
  outputs[CV_OUTPUT].value = cvout;
  return;
}

struct WalkerWidget : ModuleWidget
{
	SVGPanel *panel;
	WalkerWidget(Walker *module);
};

WalkerWidget::WalkerWidget(Walker *module) : ModuleWidget(module)
{
  box.size = Vec(4 * 15, RACK_GRID_HEIGHT);

  panel = new SVGPanel();
  panel->box.size = box.size;
  panel->setBackground(SVG::load(assetPlugin(plugin, "res/Walker.svg")));
  addChild(panel);

  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(16, 40), module, Walker::STEP_PARAM, 0.0, 1.0, 0.1));
  addParam(ParamWidget::create<Trimpot>(Vec(21.5, 80), module, Walker::STEP_ATT_PARAM, -1.0, 1.0, 0));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(16, 110), module, Walker::RANGE_PARAM, 0, 5.0, 2.0));
  addParam(ParamWidget::create<Trimpot>(Vec(21.5, 150), module, Walker::RANGE_ATT_PARAM, -1.0, 1.0, 0.0));
  addParam(ParamWidget::create<BefacoSwitch>(Vec(16, 172.5), module, Walker::RANGE_MODE_PARAM, 1, 3, 1));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(16, 210), module, Walker::SYM_PARAM, -1.0, 1.0, 0));

  addInput(Port::create<PJ301MPort>(Vec(3, 320), Port::INPUT, module, Walker::CLOCK_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(3, 276), Port::INPUT, module, Walker::STEP_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(30, 276), Port::INPUT, module, Walker::RANGE_INPUT));;
  addOutput(Port::create<PJ301MPort>(Vec(30,320), Port::OUTPUT, module, Walker::CV_OUTPUT));
}

Model *modelWalker = Model::create<Walker, WalkerWidget>("Aepelzens Modules", "Walker", "Random Walk", UTILITY_TAG, RANDOM_TAG);
