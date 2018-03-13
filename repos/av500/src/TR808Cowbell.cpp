#include "av500.hpp"
#include "CB_data.h"
#include "dsp/digital.hpp"

struct TR808Cowbell : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT,
		NUM_OUTPUTS
	};

	SchmittTrigger trigger;
	
	unsigned int count = -1;
	
	TR808Cowbell() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		trigger.setThresholds(0.0, 1.0);
	
		// disarm
		count  = CB_s16_raw_len;
	}
	void step() override;
};

void TR808Cowbell::step() 
{
	if (trigger.process(inputs[TRIG_INPUT].normalize(0.0))) {
		count = 0;
	}
		
	if( count < CB_s16_raw_len ) {
		int16_t sample;
		sample  = CB_s16_raw[count++];
		sample |= CB_s16_raw[count++] << 8;
		
		outputs[AUDIO_OUTPUT].value = 5.0 * (float)sample / 65536;
	} else {
		outputs[AUDIO_OUTPUT].value = 0.0;
	}
}

TR808CowbellWidget::TR808CowbellWidget() 
{
	TR808Cowbell *module = new TR808Cowbell();
	setModule(module);
	box.size = Vec(15 * 4, 380);

	{
		Panel *panel = new LightPanel();
		panel->backgroundImage = Image::load("plugins/av500/res/TR808Cowbell.png");
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15,   0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	
	addInput (createInput <PJ3410Port>(Vec( 0, 300), module, TR808Cowbell::TRIG_INPUT));
	addOutput(createOutput<PJ3410Port>(Vec(30, 300), module, TR808Cowbell::AUDIO_OUTPUT));

}
