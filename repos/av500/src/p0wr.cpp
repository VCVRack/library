#include "av500.hpp"

struct p0wr : Module {
	enum ParamIds {
		SWITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		RED_LIGHT,
		YEL_LIGHT,
		BLU_LIGHT,
		NUM_LIGHTS
	};
	
	p0wr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
	void step() override;
};

void p0wr::step() 
{
	switch ((int)roundf(params[SWITCH_PARAM].value)) {
		case 0: 
			lights[RED_LIGHT].value = 0;
			lights[YEL_LIGHT].value = 0;
			lights[BLU_LIGHT].value = 0;
			break;
		case 1: 
			lights[RED_LIGHT].value = 1;
			lights[YEL_LIGHT].value = 1;
			lights[BLU_LIGHT].value = 1;
			break;
	}
}

p0wrWidget::p0wrWidget() 
{
	p0wr *module = new p0wr();
	setModule(module);
	box.size = Vec(15 * 4, 380);

	{
		Panel *panel = new LightPanel();
		panel->backgroundImage = Image::load("plugins/av500/res/p0wr.png");
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15,   0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	
	addParam(createParam<NKK>(Vec(14, 129), module, p0wr::SWITCH_PARAM, 0.0, 1.0, 0.0));
	
	addChild(createLight<LargeLight<RedLight>>   (Vec(23,  54), module, p0wr::RED_LIGHT));
	addChild(createLight<LargeLight<YellowLight>>(Vec(23,  78), module, p0wr::YEL_LIGHT));
	addChild(createLight<LargeLight<BlueLight>>  (Vec(23, 102), module, p0wr::BLU_LIGHT));
}
