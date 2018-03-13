#include "aepelzen.hpp"
#include "dsp/digital.hpp"

#define NUM_CHANNELS 4
#define NUM_STEPS 8

struct Dice : Module {
    enum ParamIds {
	RESET_PARAM,
	CHANNEL_STEPS_PARAM,
	CHANNEL_MODE_PARAM = CHANNEL_STEPS_PARAM + NUM_CHANNELS,
	COLUMN1_PARAM = CHANNEL_MODE_PARAM + NUM_CHANNELS,
	COLUMN2_PARAM = COLUMN1_PARAM + NUM_STEPS,
	COLUMN3_PARAM = COLUMN2_PARAM + NUM_STEPS,
	COLUMN4_PARAM = COLUMN3_PARAM + NUM_STEPS,
	NUM_PARAMS = COLUMN4_PARAM + NUM_STEPS
    };
    enum InputIds {
	CHANNEL_CLOCK_INPUT,
	NUM_INPUTS = CHANNEL_CLOCK_INPUT + NUM_CHANNELS
    };
    enum OutputIds {
	GATE_OUTPUT,
	NUM_OUTPUTS = GATE_OUTPUT + NUM_CHANNELS
    };
    enum LightIds {
	STEP_LIGHT,
	NUM_LIGHTS = STEP_LIGHT + NUM_CHANNELS * NUM_STEPS
    };

    enum PlaybackModes {
	MODE_FORWARD,
	MODE_BACKWARD,
	MODE_ALTERNATING,
	MODE_RANDOM_NEIGHBOUR,
	MODE_RANDOM
    };

    Dice() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	//initialize RNG
	//randomSeedTime();
    }
    void step() override;

    SchmittTrigger channelClockTrigger[NUM_CHANNELS]; // for external clock
    SchmittTrigger resetTrigger;
    PulseGenerator gatePulse[NUM_CHANNELS];
    SchmittTrigger clockTrigger;
    int channel_index[NUM_CHANNELS] = {0};
    bool direction[NUM_CHANNELS] = {};
    float ColumnValue[NUM_CHANNELS] = {0};
    float randomValue;
};


void Dice::step() {

    //normalize to first clock input
    for(int y=1;y<NUM_CHANNELS;y++) {
	if(!inputs[CHANNEL_CLOCK_INPUT + y].active) {
	    inputs[CHANNEL_CLOCK_INPUT + y].value = inputs[CHANNEL_CLOCK_INPUT].value;
	}
    }

    for(int y=0;y<NUM_CHANNELS;y++) {

	bool pulse = false;
	bool channelStep = false;

	if (channelClockTrigger[y].process(inputs[CHANNEL_CLOCK_INPUT + y].value)) {
	    channelStep = true;
	}

	if (channelStep) {
	    int numSteps = clampi(roundf(params[CHANNEL_STEPS_PARAM + y].value), 1, 8);
	    int mode = clampi(roundf(params[CHANNEL_MODE_PARAM + y].value),0,5);
	    gatePulse[y].trigger(1e-3);
	    randomValue = randomf();
	    
	    if (mode == MODE_RANDOM_NEIGHBOUR) {
		mode = (randomf() > 0.5) ? MODE_FORWARD : MODE_BACKWARD;
	    }

	    switch(mode) {
	    case MODE_FORWARD:
		channel_index[y] += 1;
		channel_index[y] = (channel_index[y] >= numSteps) ? 0 : channel_index[y];
		break;
	    case MODE_BACKWARD:
		channel_index[y] -= 1;
		channel_index[y] = (channel_index[y] < 0) ? numSteps - 1 : channel_index[y];
		break;
	    case MODE_ALTERNATING:
		if (direction[y]) {
		    channel_index[y] += 1;
		    if (channel_index[y] >= numSteps) {
			channel_index[y] = numSteps - 2;
			direction[y] = !direction[y];
		    }
		}
		else {
		    channel_index[y] -= 1;
		    if (channel_index[y] <= 0) {
			channel_index[y] = 0;
			direction[y] = !direction[y];
		    }
		}
		break;
	    case MODE_RANDOM:
		channel_index[y] = rand() % numSteps;
		break;
	    }	    
	}
	
	pulse = gatePulse[y].process(1.0 / engineGetSampleRate());
	bool gateOn = (randomValue < (params[COLUMN1_PARAM + channel_index[y] + y * 8].value)) ? 1.0 : 0.0;
	gateOn = gateOn && !pulse;
	outputs[GATE_OUTPUT + y].value = (gateOn) ? 10.0 : 0.0;
	
	for(int i=0;i<NUM_STEPS;i++) {
	    lights[STEP_LIGHT + y*NUM_STEPS + i].value = (i == channel_index[y] ? 1.0 : 0.0);   
	}
    }

    // Reset
    if (resetTrigger.process(params[RESET_PARAM].value)) {
	//nextStep = true;
	for (int i=0;i<NUM_CHANNELS;i++) {
	    channel_index[i] = 0;
	}
    }
}


struct DiceWidget : ModuleWidget {
	DiceWidget(Dice *module);
};

DiceWidget::DiceWidget(Dice *module) : ModuleWidget(module) {
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Dice.svg")));
	addChild(panel);
    }

    addParam(ParamWidget::create<LEDButton>(Vec(10, 5), module, Dice::RESET_PARAM, 0.0, 1.0, 0.0));
    
    for(int y=0;y<NUM_CHANNELS;y++) {
	for(int i=0;i<NUM_STEPS;i++) {
	    addParam(ParamWidget::create<Trimpot>(Vec(10 + y*27, 30 + i*28), module, Dice::COLUMN1_PARAM + y *NUM_STEPS + i, 0.0, 1.0, 0.0));
	    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(16 + y*27, 50 + i*28), module, Dice::STEP_LIGHT + y *NUM_STEPS + i));
	}
	addParam(ParamWidget::create<Trimpot>(Vec(10 + y*27, 265), module, Dice::CHANNEL_STEPS_PARAM + y, 1.0, 8.0, 8.0));
	addParam(ParamWidget::create<Trimpot>(Vec(10 + y*27, 290), module, Dice::CHANNEL_MODE_PARAM + y, 0, 5, 0));
	addInput(Port::create<PJ301MPort>(Vec(7+y*27, 310), Port::INPUT, module, Dice::CHANNEL_CLOCK_INPUT + y));
	addOutput(Port::create<PJ301MPort>(Vec(7 + y*27, 345), Port::OUTPUT, module, Dice::GATE_OUTPUT + y));
    }
}

Model *modelDice = Model::create<Dice, DiceWidget>("Aepelzens Modules", "Dice", "Probability Sequencer", SEQUENCER_TAG);
