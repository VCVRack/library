

#include <iostream>

#include "Southpole.hpp"

#include "dsp/digital.hpp"


struct Bounce : Module {
	enum ParamIds {
		MASS_PARAM,
		DAMP_PARAM,
		TENS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		MASS_INPUT,
		DAMP_INPUT,
		TENS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		DECAY1_LIGHT,
		NUM_LIGHTS
	};

	float a0 = 0.0;
	float v0 = 0.0;
	float x0 = 0.0;
	float f0;

	unsigned c = 0;

	Bounce() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};



void Bounce::step() {

	float mass = clampf(params[MASS_PARAM].value + inputs[MASS_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
	float damp = clampf(params[DAMP_PARAM].value + inputs[DAMP_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
	float tens = clampf(params[TENS_PARAM].value + inputs[TENS_INPUT].normalize(0.) / 10.0, 0.0, 1.0);

	float dt = 1. / engineGetSampleRate();
	float m  = 1000000.*mass + 1.;
	float b  = 1000000.*damp + 1.;
	float k  = 1000000.*tens + 1.;
	float xin = inputs[IN_INPUT].value; 

	x0 = x0 + dt*v0; 
	v0 = v0 + dt*a0;
	a0 = - ( k * (x0-xin) + b * v0 ) / m;

	outputs[OUT_OUTPUT].value = clampf(x0, -10., 10. );

}


BounceWidget::BounceWidget() {
	Bounce *module = new Bounce();
	setModule(module);
	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Bounce.svg")));
		addChild(panel);
	}

	const float x1 = 4.;	
	const float x2 = 30.;

	const float y1 = 40.;
	const float yh = 31.;
	
	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+1*yh), module, Bounce::MASS_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+1*yh), module, Bounce::DAMP_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1+2*yh), module, Bounce::TENS_PARAM, 0.0, 1.0, 0.5));

	addInput(createInput<sp_Port>(Vec(x1, y1+7*yh), module, Bounce::IN_INPUT));
	addOutput(createOutput<sp_Port>(Vec(x2, y1+7*yh), module, Bounce::OUT_OUTPUT));

//	addChild(createLight<SmallLight<RedLight>>(Vec(94, 109), module, Bounce::DECAY_LIGHT));
}
