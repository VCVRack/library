#include "dekstop.hpp"
#include "dsp/digital.hpp"

const int NUM_STEPS = 12;
const int NUM_CHANNELS = 8;
const int NUM_GATES = NUM_STEPS * NUM_CHANNELS;

struct GateSEQ8 : Module {
	
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		GATE1_PARAM,
		NUM_PARAMS = GATE1_PARAM + NUM_GATES
	};
	enum InputIds {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE1_OUTPUT,
		NUM_OUTPUTS = GATE1_OUTPUT + NUM_CHANNELS
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATE_LIGHTS,
		NUM_LIGHTS = GATE_LIGHTS + NUM_GATES
	};

	bool running = true;
	SchmittTrigger clockTrigger; // for external clock
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	float multiplier = 1.0;
	float phase = 0.0;
	int index = 0;
	SchmittTrigger gateTriggers[NUM_GATES];
	bool gateState[NUM_GATES] = {};
	float stepLights[NUM_GATES] = {};

	GateSEQ8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step();

	json_t *toJson() {
		json_t *rootJ = json_object();

		// Clock multiplier
		json_t *multiplierJ = json_real(multiplier);
		json_object_set_new(rootJ, "multiplier", multiplierJ);

		// Gate values
		json_t *gatesJ = json_array();
		for (int i = 0; i < NUM_GATES; i++) {
			json_t *gateJ = json_integer((int) gateState[i]);
			json_array_append_new(gatesJ, gateJ);
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) {
		// Clock multiplier
		json_t *multiplierJ = json_object_get(rootJ, "multiplier");
		if (!multiplierJ) {
			multiplier = 1.0;
		} else {
			multiplier = (float)json_real_value(multiplierJ);
		}

		// Gate values
		json_t *gatesJ = json_object_get(rootJ, "gates");
		for (int i = 0; i < NUM_GATES; i++) {
			json_t *gateJ = json_array_get(gatesJ, i);
			gateState[i] = !!json_integer_value(gateJ);
		}
	}

	void reset() {
		for (int i = 0; i < NUM_GATES; i++) {
			gateState[i] = false;
		}
	}

	void randomize() {
		for (int i = 0; i < NUM_GATES; i++) {
			gateState[i] = (randomf() > 0.5);
		}
	}
};


void GateSEQ8::step() {
	#ifdef v_050_dev
	float gSampleRate = engineGetSampleRate();
	#endif
	const float lightLambda = 0.1;
	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	bool nextStep = false;

	if (running) {
		if (inputs[EXT_CLOCK_INPUT].active) {
			// External clock
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) {
				phase = 0.0;
				nextStep = true;
			}
		}
		else {
			// Internal clock
			float clockTime = powf(2.0, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
			clockTime = clockTime * multiplier;
			phase += clockTime / gSampleRate;
			if (phase >= 1.0) {
				phase -= 1.0;
				nextStep = true;
			}
		}
	}

	// Reset
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) {
		phase = 0.0;
		index = 999;
		nextStep = true;
		lights[RESET_LIGHT].value = 1.0;
	}

	if (nextStep) {
		// Advance step
		int numSteps = clampi(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1, NUM_STEPS);
		index += 1;
		if (index >= numSteps) {
			index = 0;
		}
		for (int y = 0; y < NUM_CHANNELS; y++) {
			stepLights[y*NUM_STEPS + index] = 1.0;
		}
	}

	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / gSampleRate;

	// Gate buttons
	for (int i = 0; i < NUM_GATES; i++) {
		if (gateTriggers[i].process(params[GATE1_PARAM + i].value)) {
			gateState[i] = !gateState[i];
		}
		stepLights[i] -= stepLights[i] / lightLambda / gSampleRate;
		lights[GATE_LIGHTS + i].value = (gateState[i] >= 1.0) ? 1.0 - stepLights[i] : stepLights[i];
	}
	for (int y = 0; y < NUM_CHANNELS; y++) {
		float gate = (gateState[y*NUM_STEPS + index] >= 1.0) ? 10.0 : 0.0;
		outputs[GATE1_OUTPUT + y].value = gate;
	}
}


struct ClockMultiplierItem : MenuItem {
	GateSEQ8 *gateSEQ8;
	float multiplier;
	void onAction(EventAction &e) override {
		gateSEQ8->multiplier = multiplier;
	}
};

struct ClockMultiplierChoice : ChoiceButton {
	GateSEQ8 *gateSEQ8;
	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		const float multipliers[12] = {0.25, 0.3, 0.5, 0.75, 
																	1.0, 1.5, 2.0, 3.0,
																	4.0, 6.0, 8.0, 12.0};
		const std::string labels[12] = {"1/4", "1/3", "1/2", "3/4", 
																	 "1/1", "3/2", "2/1", "3/1",
																 	 "4/1", "6/1", "8/1", "12/1"};
		int multipliersLen = sizeof(multipliers) / sizeof(multipliers[0]);
		for (int i = 0; i < multipliersLen; i++) {
			ClockMultiplierItem *item = new ClockMultiplierItem();
			item->gateSEQ8 = gateSEQ8;
			item->multiplier = multipliers[i];
			item->text = stringf("%.2f (%s)", multipliers[i], labels[i].c_str());
			menu->addChild(item);
		}
	}
	void step() override {
		this->text = stringf("%.2f", gateSEQ8->multiplier);
	}
};

GateSEQ8Widget::GateSEQ8Widget() {
	GateSEQ8 *module = new GateSEQ8();
	setModule(module);
	box.size = Vec(360, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/GateSEQ8.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(createParam<RoundSmallBlackKnob>(Vec(17, 56), module, GateSEQ8::CLOCK_PARAM, -2.0, 10.0, 2.0));
	addParam(createParam<LEDButton>(Vec(60, 61-1), module, GateSEQ8::RUN_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(60+6, 61+5), module, GateSEQ8::RUNNING_LIGHT));
	addParam(createParam<LEDButton>(Vec(98, 61-1), module, GateSEQ8::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(98+6, 61+5), module, GateSEQ8::RESET_LIGHT));
	addParam(createParam<RoundSmallBlackSnapKnob>(Vec(132, 56), module, GateSEQ8::STEPS_PARAM, 1.0, NUM_STEPS, NUM_STEPS));

	static const float portX[8] = {19, 57, 96, 134, 173, 211, 250, 288};
	addInput(createInput<PJ301MPort>(Vec(portX[0]-1, 99-1), module, GateSEQ8::CLOCK_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[1]-1, 99-1), module, GateSEQ8::EXT_CLOCK_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[2]-1, 99-1), module, GateSEQ8::RESET_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[3]-1, 99-1), module, GateSEQ8::STEPS_INPUT));

	{
		Label *label = new Label();
		label->box.pos = Vec(200, 55);
		label->text = "Clock multiplier";
		addChild(label);

		ClockMultiplierChoice *choice = new ClockMultiplierChoice();
		choice->gateSEQ8 = dynamic_cast<GateSEQ8*>(module);
		choice->box.pos = Vec(200, 70);
		choice->box.size.x = 100;
		addChild(choice);
	}

	for (int y = 0; y < NUM_CHANNELS; y++) {
		for (int x = 0; x < NUM_STEPS; x++) {
			int i = y*NUM_STEPS+x;
			addParam(createParam<LEDButton>(Vec(22 + x*25, 155+y*25+3), module, GateSEQ8::GATE1_PARAM + i, 0.0, 1.0, 0.0));
			addChild(createLight<SmallLight<GreenLight>>(Vec(28 + x*25, 156+y*25+8), module, GateSEQ8::GATE_LIGHTS + i));
		}
		addOutput(createOutput<PJ301MPort>(Vec(320, 155+y*25), module, GateSEQ8::GATE1_OUTPUT + y));
	}
}
