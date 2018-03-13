#pragma once

#include "bogaudio.hpp"

extern Model* modelVCA;

namespace bogaudio {

struct VCA : Module {
	enum ParamsIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	VCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	virtual void step() override;
	void channelStep(Input& input, Output& output, Param& knob, Input& cv);
};

} // namespace bogaudio
