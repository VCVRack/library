

#include "Southpole.hpp"

#include <string.h>


struct Balaclava : Module {
	enum ParamIds {
		GAIN1_PARAM,
		GAIN2_PARAM,
		GAIN3_PARAM,
		GAIN4_PARAM,
		RESPONSE1_PARAM,
		RESPONSE2_PARAM,
		RESPONSE3_PARAM,
		RESPONSE4_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		CV1_INPUT,
		CV2_INPUT,
		CV3_INPUT,
		CV4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT1_POS_LIGHT, OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT, OUT2_NEG_LIGHT,
		OUT3_POS_LIGHT, OUT3_NEG_LIGHT,
		OUT4_POS_LIGHT, OUT4_NEG_LIGHT,
		NUM_LIGHTS
	};

	Balaclava() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Balaclava::step() {
	float out = 0.0;

	for (int i = 0; i < 4; i++) {
		float in = inputs[IN1_INPUT + i].value * params[GAIN1_PARAM + i].value;
		if (inputs[CV1_INPUT + i].active) {
			float linear = fmaxf(inputs[CV1_INPUT + i].value / 5.0, 0.0);
			linear = clampf(linear, 0.0, 2.0);
			const float base = 200.0;
			float exponential = rescalef(powf(base, linear / 2.0), 1.0, base, 0.0, 10.0);
			in *= crossf(exponential, linear, params[RESPONSE1_PARAM + i].value);
		}
		out += in;
		lights[OUT1_POS_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, out / 5.0));
		lights[OUT1_NEG_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, -out / 5.0));
		if (outputs[OUT1_OUTPUT + i].active) {
			outputs[OUT1_OUTPUT + i].value = out;
			out = 0.0;
		}
	}
}


BalaclavaWidget::BalaclavaWidget() {
	Balaclava *module = new Balaclava();
	setModule(module);
	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Balaclava.svg")));
		panel->box.size = box.size;
		addChild(panel);	
	}

    const float x1 = 5.;
    const float x2 = 20.;
    const float x3 = 36.;	

	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 52+8), module, Balaclava::GAIN1_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 131+8), module, Balaclava::GAIN2_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 210+8), module, Balaclava::GAIN3_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 288+8), module, Balaclava::GAIN4_PARAM, 0.0, 1.0, 0.0));

	addParam(createParam<sp_Trimpot>(Vec(x3,  80), module, Balaclava::RESPONSE1_PARAM, 0.0, 1.0, 1.0));
	addParam(createParam<sp_Trimpot>(Vec(x3, 159), module, Balaclava::RESPONSE2_PARAM, 0.0, 1.0, 1.0));
	addParam(createParam<sp_Trimpot>(Vec(x3, 238), module, Balaclava::RESPONSE3_PARAM, 0.0, 1.0, 1.0));
	addParam(createParam<sp_Trimpot>(Vec(x3, 316), module, Balaclava::RESPONSE4_PARAM, 0.0, 1.0, 1.0));

	addInput(createInput<sp_Port>(Vec(x1, 41), module, Balaclava::IN1_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 120), module, Balaclava::IN2_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 198), module, Balaclava::IN3_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 277), module, Balaclava::IN4_INPUT));

	addInput(createInput<sp_Port>(Vec(x1, 80), module, Balaclava::CV1_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 159), module, Balaclava::CV2_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 238), module, Balaclava::CV3_INPUT));
	addInput(createInput<sp_Port>(Vec(x1, 316), module, Balaclava::CV4_INPUT));

	addOutput(createOutput<sp_Port>(Vec(x3, 41), module, Balaclava::OUT1_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x3, 120), module, Balaclava::OUT2_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x3, 198), module, Balaclava::OUT3_OUTPUT));
	addOutput(createOutput<sp_Port>(Vec(x3, 277), module, Balaclava::OUT4_OUTPUT));

	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 87-22), module, Balaclava::OUT1_POS_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 166-22), module, Balaclava::OUT2_POS_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 245-22), module, Balaclava::OUT3_POS_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 324-22), module, Balaclava::OUT4_POS_LIGHT));
}
