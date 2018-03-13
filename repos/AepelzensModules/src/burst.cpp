#include "aepelzen.hpp"
#include "dsp/digital.hpp"
#include <math.h>

#define MAX_REPS 8
#define MAX_TIME 1

struct Burst : Module
{
  enum ParamIds
    {
      BUTTON_PARAM,
      TIME_PARAM,
      REP_PARAM,
      ACCEL_PARAM,
      JITTER_PARAM,
      CV_MODE_PARAM,
      GATE_MODE_PARAM,
      REP_ATT_PARAM,
      TIME_ATT_PARAM,
      NUM_PARAMS = TIME_ATT_PARAM
    };
  enum InputIds
    {
      GATE_INPUT,
      CLOCK_INPUT,
      REP_INPUT,
      TIME_INPUT,
      NUM_INPUTS = TIME_INPUT
    };
  enum OutputIds
    {
      GATE_OUTPUT,
      EOC_OUTPUT,
      CV_OUTPUT,
      NUM_OUTPUTS = CV_OUTPUT
    };

  enum LightIds
    {
      NUM_LIGHTS
    };

  enum CvModes
    {
      CV_UP,
      CV_DOWN,
      CV_MODE3,
      CV_MODE4,
      CV_MODE_RANDOMP,
      CV_MODE_RANDOMN,
      CV_MODE_RANDOM,
    };

  const float euler = exp(1);
  float timeParam = 0;
  float clockedTimeParam = 0;
  float pulseParam = 4;
  float timer = 0;
  float seconds = 0;
  int pulseCount = 0;
  int pulses = 4;
  float delta = 0;
  float randomcv = 0;
  float cvOut = 0;

  SchmittTrigger m_buttonTrigger;
  SchmittTrigger gateTrigger;;
  SchmittTrigger clockTrigger;

  PulseGenerator outPulse;
  PulseGenerator eocPulse;

  //toggle for every received clock tick
  float clockTimer = 0;
  float lastClockTime = 0;
  float gateOutLength = 0.01;

  void step() override;

  Burst() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    reset();
  }

  void reset() override
  {
    onSampleRateChange();
  }

  void onSampleRateChange() override {delta = 1.0/engineGetSampleRate();}

  void randomize() override {}
};

void Burst::step()
{
  //ModuleWidget::step();
  float schmittValue = gateTrigger.process(inputs[GATE_INPUT].value);

  //seconds = params[TIME_PARAM].value + (inputs[TIME_INPUT].value / 20.0);
  float accel = params[ACCEL_PARAM].value;
  float jitter = params[JITTER_PARAM].value;
  float randomDelta = 0;

  timeParam = clampf(params[TIME_PARAM].value + (params[TIME_ATT_PARAM].value * inputs[TIME_INPUT].value / 10.0 * MAX_TIME),0.0, MAX_TIME);
  pulseParam = clampf(params[REP_PARAM].value + (inputs[REP_INPUT].value * params[REP_ATT_PARAM].value /10.0 * MAX_REPS), 0.0, MAX_REPS);

  //exponential scaling for timeparam
  timeParam = (exp(timeParam) - 1)/(euler - 1);

  if (inputs[CLOCK_INPUT].active) {
    clockTimer += delta;
    if( clockTrigger.process(inputs[CLOCK_INPUT].value) ) {
      timeParam = params[TIME_PARAM].value;
      int mult = (int)(timeParam*8 - 4);
      //smooth clock (over 8 pulses) to reduce sensitivity
      // float clockDelta = clockTimer - lastClockTime;
      // clockTimer -= clockTimer - (clockDelta/8);
      lastClockTime = clockTimer;

      timeParam = clockTimer * pow(2, mult);
      clockedTimeParam = timeParam;
      clockTimer = 0;
    }
    timeParam = clockedTimeParam;
  }

  if( timer > seconds && pulseCount < pulses ) {
    pulseCount ++;
    timer = 0.0;

    seconds = timeParam;

    if(accel > 0) {
      seconds = timeParam/pow(accel,pulseCount);
    }

    if(jitter > 0) {
      randomDelta = randomf() * jitter * seconds;
      if (randomf() > 0.5) {
	seconds = seconds + randomDelta;
      }
      else {
	seconds = seconds - randomDelta;
      }
    }

    if (pulseCount == pulses) {
      eocPulse.trigger(0.01);
    }
    gateOutLength = (params[GATE_MODE_PARAM].value) ? 0.01 : seconds/2;
    outPulse.trigger(gateOutLength);
    randomcv = randomf();

    //cv
    float cvDelta = 5.0/pulses;
    int mode = roundf(params[CV_MODE_PARAM].value);
    switch (mode) {
    case CV_UP:
      cvOut = pulseCount * cvDelta;
      break;
    case CV_DOWN:
      cvOut = pulseCount * cvDelta * (-1);
      break;
    case CV_MODE3:
      cvOut = trunc((pulseCount + 1)/2) * cvDelta;
      if(pulseCount % 2 == 1) {
	cvOut *= -1;
      }
      break;
    case CV_MODE4:
      cvOut = pulseCount * cvDelta;
      if(pulseCount %2 == 1) {
	cvOut *= -1;
      }
      break;
    case CV_MODE_RANDOMP:
      cvOut = randomcv *5.0;
      break;
    case CV_MODE_RANDOMN:
      cvOut = randomcv * (-5.0);
      break;
    case CV_MODE_RANDOM:
      cvOut = randomcv *10 - 5;
      break;
    }
  }

  if (schmittValue || m_buttonTrigger.process(params[BUTTON_PARAM].value)) {
    pulseCount = 0;
    timer = 0.0;
    //outPulse.trigger(0.01);
    outPulse.trigger(gateOutLength);
    seconds = timeParam;
    pulses = pulseParam;
    //outputs[CV_OUTPUT].value = 0;
    cvOut = 0;
  }

  timer += delta;
  outputs[GATE_OUTPUT].value = outPulse.process(delta) ? 10.0 : 0.0;
  outputs[EOC_OUTPUT].value = eocPulse.process(delta) ? 10.0 : 0.0;
  outputs[CV_OUTPUT].value = (timer < seconds) ? cvOut : 0.0;
}


struct BurstWidget : ModuleWidget
{
	SVGPanel *panel;
	BurstWidget(Burst *module);
  //void step() override;
};

BurstWidget::BurstWidget(Burst *module) : ModuleWidget(module)
{
  //box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
  box.size = Vec(6 * 15, RACK_GRID_HEIGHT);

  panel = new SVGPanel();
  panel->box.size = box.size;
  panel->setBackground(SVG::load(assetPlugin(plugin, "res/Burst.svg")));
  addChild(panel);

  // addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  // addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  // addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  // addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

  //note: SmallKnob size = 28px, Trimpot = 17 px
  //addParam(ParamWidget::create<LEDBezel>(Vec(30, 105), module, Burst::BUTTON_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<CKD6>(Vec(30, 105), module, Burst::BUTTON_PARAM, 0.0, 1.0, 0.0));
  //addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(35, 50), module, Burst::REP_PARAM, 1, 8, 4));
  addParam(ParamWidget::create<Davies1900hLargeBlackKnob>(Vec(18, 30), module, Burst::REP_PARAM, 0, 8, 4));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10, 150), module, Burst::TIME_PARAM, 0.02, 1, 0.5));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(52, 150), module, Burst::ACCEL_PARAM, 1.0, 2.0, 1.0));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10, 195), module, Burst::JITTER_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(52, 195), module, Burst::CV_MODE_PARAM, 0, 6, 0));

  addParam(ParamWidget::create<Trimpot>(Vec(15.5, 240), module, Burst::REP_ATT_PARAM, -1.0, 1.0, 0.0));
  addParam(ParamWidget::create<Trimpot>(Vec(54, 240), module, Burst::TIME_ATT_PARAM, -1.0, 1.0, 0.0));
  addInput(Port::create<PJ301MPort>(Vec(13, 265), Port::INPUT, module, Burst::REP_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(50, 265), Port::INPUT, module, Burst::TIME_INPUT));;

  addInput(Port::create<PJ301MPort>(Vec(5, 305), Port::INPUT, module, Burst::GATE_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(60, 305), Port::INPUT, module, Burst::CLOCK_INPUT));;
  addParam(ParamWidget::create<CKSS>(Vec(38, 300), module, Burst::GATE_MODE_PARAM, 0.0, 1.0, 0.0));

  addOutput(Port::create<PJ301MPort>(Vec(5,335), Port::OUTPUT, module, Burst::CV_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(32.5,335), Port::OUTPUT, module, Burst::EOC_OUTPUT));
  addOutput(Port::create<PJ301MPort>(Vec(60,335), Port::OUTPUT, module, Burst::GATE_OUTPUT));
}

Model *modelBurst = Model::create<Burst, BurstWidget>("Aepelzens Modules", "burst", "Burst Generator", SEQUENCER_TAG);
