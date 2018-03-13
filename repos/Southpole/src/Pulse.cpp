#include "Southpole.hpp"
#include "dsp/digital.hpp"

struct Pulse : Module {
	enum ParamIds {
		TRIG_PARAM,
		REPEAT_PARAM,
		RESET_PARAM,
		RANGE_PARAM,
		DELAY_PARAM,
		TIME_PARAM,
		AMP_PARAM,
		OFFSET_PARAM,
//		C_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		CLOCK_INPUT,
//		REPEAT_INPUT,
//		RESET_INPUT,
		DELAY_INPUT,
		TIME_INPUT,
		AMP_INPUT,
		OFFSET_INPUT,
//		C_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		GATE_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
//		DECAY1_LIGHT,
		NUM_LIGHTS
	};

	SchmittTrigger clock;
	SchmittTrigger trigger;
	SchmittTrigger triggerBtn;
	PulseGenerator clkPulse;
	PulseGenerator eocPulse;

	unsigned long delayt = 0;
	unsigned long gatet = 0;
	unsigned long clockt = 0;
	unsigned long clockp = 0;

	unsigned long delayTarget = 0;
	unsigned long gateTarget = 0;

	bool reset  = true;
	bool repeat = false;
	bool range  = false;
	bool gateOn = false;
	bool delayOn = false;

	float amp;
	float offset;

	static const int ndurations = 12;
	const float durations[ndurations] = {
		1/256.,1/128.,1/64.,1/32.,1/16.,1/8.,3./16.,1/4.,1/3.,1/2.,3./4.,.99
	 	//,2.,3.,4. //,5.,6.,7.,8.,12.,16.
	};

	Pulse() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;
};



void Pulse::step() {

	bool triggered = false;

	reset  = params[RESET_PARAM].value;
	repeat = params[REPEAT_PARAM].value;
	range  = params[RANGE_PARAM].value;

    if (triggerBtn.process(params[TRIG_PARAM].value)) {
		triggered = true;	
	}

	if (trigger.process(inputs[TRIG_INPUT].normalize(0.))) {
		triggered = true;
		//printf("%lu\n", gateTarget);
	}

	if (clock.process(inputs[CLOCK_INPUT].normalize(0.))) {
		triggered = true;
		clkPulse.trigger(1e-3);
		clockp = clockt;
		clockt = 0;
	}

	float dt = 1e-3*engineGetSampleRate();
	float sr = engineGetSampleRate();

	amp    = clampf(params[AMP_PARAM].value + inputs[AMP_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
	offset = clampf(params[OFFSET_PARAM].value + inputs[OFFSET_INPUT].normalize(0.) / 10.0, -1.0, 1.0);

	float delayTarget_ = clampf(params[DELAY_PARAM].value + inputs[DELAY_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
	float gateTarget_  = clampf(params[TIME_PARAM].value + inputs[TIME_INPUT].normalize(0.) / 10.0, 0.0, 1.0);

	if (inputs[CLOCK_INPUT].active) {
		clockt++;

		delayTarget = clockp*durations[int((ndurations-1)*delayTarget_)];
		gateTarget  = clockp*durations[int((ndurations-1)*gateTarget_)];
		if (gateTarget < dt) gateTarget = dt;
		
	} else {
		unsigned int r = range ? 10 : 1;
		delayTarget = r * delayTarget_*sr;
		gateTarget  = r * gateTarget_*sr + dt;
	}

	if (triggered && (reset || !gateOn || !delayOn)) {
		delayt = 0;
		delayOn = true;		
		gateOn = false;		
	}

	if (delayOn) {
		if (delayt < delayTarget) {
			delayt++;
		} else {
			delayOn = false;		
			gateOn = true;
			gatet = 0;
		}
	}

	if (gateOn) {

		if (gatet < gateTarget) {
			gatet++;
		} else {
			eocPulse.trigger(1e-3);
			gateOn = false;
			if (repeat) {
				delayt = 0;
				delayOn = true;		
			}
		}
	}

	outputs[CLOCK_OUTPUT].value = 10.*clkPulse.process(1.0 / engineGetSampleRate());
	outputs[EOC_OUTPUT].value   = 10.*eocPulse.process(1.0 / engineGetSampleRate());
	outputs[GATE_OUTPUT].value  = clampf( gateOn * 10. * amp + 5.*offset, -10., 10. );
}


PulseWidget::PulseWidget() {
	Pulse *module = new Pulse();
	setModule(module);
	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Pulse.svg")));
		addChild(panel);
	}

	const float x1 = 5.;
	const float x2 = 35.;
	const float y1 = 40.;
	const float yh = 35.;
	 
	addInput(createInput<sp_Port>			(Vec(x1, y1+0*yh), module, Pulse::CLOCK_INPUT));
	addOutput(createOutput<sp_Port>			(Vec(x2, y1+0*yh), module, Pulse::CLOCK_OUTPUT));
	
	addInput(createInput<sp_Port>			(Vec(x1, y1+1*yh), module, Pulse::TRIG_INPUT));
	addParam(createParam<TL1105>			(Vec(x2, y1+1*yh), module, Pulse::TRIG_PARAM, 0.0, 1.0, 0.));

    addParam(createParam<sp_Switch>			(Vec(x1, y1+1.75*yh), module, Pulse::RESET_PARAM, 0.0, 1.0, 0.0));
    addParam(createParam<sp_Switch>		 	(Vec(x1, y1+2.25*yh), module, Pulse::REPEAT_PARAM, 0.0, 1.0, 0.0));
    addParam(createParam<sp_Switch>		 	(Vec(x1, y1+2.75*yh), module, Pulse::RANGE_PARAM, 0.0, 1.0, 0.0));

	addInput(createInput<sp_Port>		   (Vec(x1, y1+4*yh), module, Pulse::TIME_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+4*yh), module, Pulse::TIME_PARAM, 0.0, 1.0, 0.0));

	addInput(createInput<sp_Port>		   (Vec(x1, y1+5*yh), module, Pulse::DELAY_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+5*yh), module, Pulse::DELAY_PARAM, 0.0, 1.0, 0.0));

	addInput(createInput<sp_Port>		   (Vec(x1, y1+6*yh), module, Pulse::AMP_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+6*yh), module, Pulse::AMP_PARAM, 0.0, 1.0, 1.0));

	addInput(createInput<sp_Port>		   (Vec(x1, y1+7*yh), module, Pulse::OFFSET_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+7*yh), module, Pulse::OFFSET_PARAM, -1.0, 1.0, 0.));

	//addInput(createInput<sp_Port>		   (Vec(x1, y1+8*yh), module, Pulse::C_INPUT));
	//addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+8*yh), module, Pulse::C_PARAM, 0.0, 1.0, 0.5));

	addOutput(createOutput<sp_Port>        (Vec(x1, y1+8.25*yh), module, Pulse::EOC_OUTPUT));
	addOutput(createOutput<sp_Port>        (Vec(x2, y1+8.25*yh), module, Pulse::GATE_OUTPUT));

//	addChild(createLight<SmallLight<RedLight>>(Vec(94, 109), module, Pulse::DECAY_LIGHT));
}
