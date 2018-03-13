#include "dekstop.hpp"
#include "dsp/digital.hpp"

struct TriSEQ3 : Module {
	enum ParamIds {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		ROW1_PARAM,
		ROW2_PARAM = ROW1_PARAM + 8,
		ROW3_PARAM = ROW2_PARAM + 8,
		GATE_PARAM = ROW3_PARAM + 8,
		NUM_PARAMS = GATE_PARAM + 8
	};
	enum InputIds {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATES_OUTPUT,
		ROW1_OUTPUT,
		ROW2_OUTPUT,
		ROW3_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS = GATE_OUTPUT + 8
	};
	enum LightIds {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATES_LIGHT,
		ROW_LIGHTS,
		GATE_LIGHTS = ROW_LIGHTS + 3,
		NUM_LIGHTS = GATE_LIGHTS + 8
	};

	bool running = true;
	SchmittTrigger clockTrigger; // for external clock
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	float phase = 0.0;
	int index = 0;
	SchmittTrigger gateTriggers[8];
	bool gateState[8] = {};
	float stepLights[8] = {};

	TriSEQ3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step();

	json_t *toJson() {
		json_t *rootJ = json_object();

		json_t *gatesJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_t *gateJ = json_integer((int) gateState[i]);
			json_array_append_new(gatesJ, gateJ);
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) {
		json_t *gatesJ = json_object_get(rootJ, "gates");
		for (int i = 0; i < 8; i++) {
			json_t *gateJ = json_array_get(gatesJ, i);
			gateState[i] = !!json_integer_value(gateJ);
		}
	}

#ifdef v_050_dev
	void reset() {
#else
	void initialize() {
#endif
		for (int i = 0; i < 8; i++) {
			gateState[i] = false;
		}
	}

	void randomize() {
		for (int i = 0; i < 8; i++) {
			gateState[i] = (randomf() > 0.5);
		}
	}
};


void TriSEQ3::step() {
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
		int numSteps = clampi(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1, 8);
		index += 1;
		if (index >= numSteps) {
			index = 0;
		}
		stepLights[index] = 1.0;
	}

	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / gSampleRate;

	// Gate buttons
	for (int i = 0; i < 8; i++) {
		if (gateTriggers[i].process(params[GATE_PARAM + i].value)) {
			gateState[i] = !gateState[i];
		}
		float gate = (i == index && gateState[i] >= 1.0) ? 10.0 : 0.0;
		outputs[GATE_OUTPUT + i].value = gate;
		stepLights[i] -= stepLights[i] / lightLambda / gSampleRate;
		lights[GATE_LIGHTS + i].value = (gateState[i] >= 1.0) ? 1.0 - stepLights[i] : stepLights[i];
	}

	// Rows
	float row1 = params[ROW1_PARAM + index].value;
	float row2 = params[ROW2_PARAM + index].value;
	float row3 = params[ROW3_PARAM + index].value;
	float gates = (gateState[index] >= 1.0) && !nextStep ? 10.0 : 0.0;
	outputs[ROW1_OUTPUT].value = row1;
	outputs[ROW2_OUTPUT].value = row2;
	outputs[ROW3_OUTPUT].value = row3;
	outputs[GATES_OUTPUT].value = gates;
	lights[GATES_LIGHT].value = (gateState[index] >= 1.0) ? 1.0 : 0.0;
	lights[ROW_LIGHTS + 0].value = row1;
	lights[ROW_LIGHTS + 1].value = row2;
	lights[ROW_LIGHTS + 2].value = row3;
}


TriSEQ3Widget::TriSEQ3Widget() {
	TriSEQ3 *module = new TriSEQ3();
	setModule(module);
	box.size = Vec(15*22, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TriSEQ3.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(createParam<RoundSmallBlackKnob>(Vec(17, 56), module, TriSEQ3::CLOCK_PARAM, -2.0, 10.0, 2.0));
	addParam(createParam<LEDButton>(Vec(60, 61-1), module, TriSEQ3::RUN_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(60+6, 61+5), module, TriSEQ3::RUNNING_LIGHT));
	addParam(createParam<LEDButton>(Vec(98, 61-1), module, TriSEQ3::RESET_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(98+6, 61+5), module, TriSEQ3::RESET_LIGHT));
	addParam(createParam<RoundSmallBlackSnapKnob>(Vec(132, 56), module, TriSEQ3::STEPS_PARAM, 1.0, 8.0, 8.0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(181.5, 66), module, TriSEQ3::GATES_LIGHT));
	addChild(createLight<SmallLight<GreenLight>>(Vec(219.5, 66), module, TriSEQ3::ROW_LIGHTS + 0));
	addChild(createLight<SmallLight<GreenLight>>(Vec(258, 66), module, TriSEQ3::ROW_LIGHTS + 1));
	addChild(createLight<SmallLight<GreenLight>>(Vec(296.5, 66), module, TriSEQ3::ROW_LIGHTS + 2));

	static const float portX[8] = {19, 57, 96, 134, 173, 211, 250, 288};
	addInput(createInput<PJ301MPort>(Vec(portX[0]-1, 99-1), module, TriSEQ3::CLOCK_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[1]-1, 99-1), module, TriSEQ3::EXT_CLOCK_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[2]-1, 99-1), module, TriSEQ3::RESET_INPUT));
	addInput(createInput<PJ301MPort>(Vec(portX[3]-1, 99-1), module, TriSEQ3::STEPS_INPUT));
	addOutput(createOutput<PJ301MPort>(Vec(portX[4]-1, 99-1), module, TriSEQ3::GATES_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(portX[5]-1, 99-1), module, TriSEQ3::ROW1_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(portX[6]-1, 99-1), module, TriSEQ3::ROW2_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(portX[7]-1, 99-1), module, TriSEQ3::ROW3_OUTPUT));

	for (int i = 0; i < 8; i++) {
		addParam(createParam<NKK>(Vec(portX[i]-3, 152), module, TriSEQ3::ROW1_PARAM + i, 0.0, 2.0, 0.0));
		addParam(createParam<NKK>(Vec(portX[i]-3, 190), module, TriSEQ3::ROW2_PARAM + i, 0.0, 2.0, 0.0));
		addParam(createParam<NKK>(Vec(portX[i]-3, 229), module, TriSEQ3::ROW3_PARAM + i, 0.0, 2.0, 0.0));
		addParam(createParam<LEDButton>(Vec(portX[i]+2, 278-1), module, TriSEQ3::GATE_PARAM + i, 0.0, 1.0, 0.0));
		addChild(createLight<SmallLight<GreenLight>>(Vec(portX[i]+8, 278+5), module, TriSEQ3::GATE_LIGHTS + i));
		addOutput(createOutput<PJ301MPort>(Vec(portX[i]-1, 308-1), module, TriSEQ3::GATE_OUTPUT + i));
	}
}
