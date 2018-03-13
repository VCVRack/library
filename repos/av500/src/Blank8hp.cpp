#include "av500.hpp"

struct Blank8hp : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	
	enum InputIds {
		NUM_INPUTS
	};
	
	enum OutputIds {
		NUM_OUTPUTS
	};

	Blank8hp();
	void step() override;
};

Blank8hp::Blank8hp() 
{
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
}

void Blank8hp::step() 
{
}

Blank8hpWidget::Blank8hpWidget() 
{
	Blank8hp *module = new Blank8hp();
	setModule(module);
	box.size = Vec(15 * 8, 380);

	{
		Panel *panel = new LightPanel();
		panel->backgroundImage = Image::load("plugins/av500/res/Blank8hp.png");
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15,   0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));

}
