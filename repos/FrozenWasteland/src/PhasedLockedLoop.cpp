/*
The filter DSP code has been derived from
Miller Puckette's code hosted at
https://github.com/ddiakopoulos/MoogLadders/blob/master/src/RKSimulationModel.h
which is licensed for use under the following terms (MIT license):


Copyright (c) 2015, Miller Puckette. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"
#include "dsp/decimator.hpp"
#include "dsp/filter.hpp"

// The clipping function of a transistor pair is approximately tanh(x)
// TODO: Put this in a lookup table. 5th order approx doesn't seem to cut it
inline float clip(float x) {
	return tanhf(x);
}

template <int OVERSAMPLE, int QUALITY>
struct VoltageControlledOscillator {
	float phase = 0.0;
	float freq;
	float pw = 0.5;
	float pitch;

	Decimator<OVERSAMPLE, QUALITY> sqrDecimator;
	RCFilter sqrFilter;


	float sqrBuffer[OVERSAMPLE] = {};

	void setPitch(float pitchKnob, float pitchCv) {
		// Compute frequency
		pitch = pitchKnob;
		pitch = roundf(pitch);
		pitch += pitchCv;
		// Note C3
		freq = 261.626 * powf(2.0, pitch / 12.0);
	}
	void setPulseWidth(float pulseWidth) {
		const float pwMin = 0.01;
		pw = clamp(pulseWidth, pwMin, 1.0 - pwMin);
	}

	void process(float deltaTime) {

		// Advance phase
		float deltaPhase = clamp(freq * deltaTime, 1e-6, 0.5);

		
		sqrFilter.setCutoff(40.0 * deltaTime);

		for (int i = 0; i < OVERSAMPLE; i++) {

			sqrBuffer[i] = (phase < pw) ? 1.f : -1.f;

			// Advance phase
			phase += deltaPhase / OVERSAMPLE;
			phase = eucmod(phase, 1.0f);
		}
	}

	
	float sqr() {
		return sqrDecimator.process(sqrBuffer);
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};

struct PhaseComparator {
	bool clock = false;
	bool data = false;
	bool nandGate1 = false;
	bool nandGate2 = false;
	bool nandGate3 = false;
	bool nandGate4 = false;


	void setClock(float clockInput)  {
		clock = clockInput >= 0;
	}

	void setData(float dataInput)  {
		data = dataInput >= 0;
	}

	float XORoutput()  {
		return (clock ^ data) ? 5.0 : -5.0;
	}

	float FlipFlopOutput()  {
		bool invertedData = !data;
		nandGate1 = !(data && clock);
		nandGate2 = !(clock && invertedData);
		nandGate3 = !(nandGate1 && nandGate4);
		nandGate4 = !(nandGate3 && nandGate2);
		
		return nandGate3 ? 5.0 : -5.0;
	}
};




struct LadderFilter {
	float cutoff = 1000.0;
	float resonance = 0.0;
	float state[4] = {};

	void calculateDerivatives(float input, float *dstate, const float *state) {
		float cutoff2Pi = 2*M_PI * cutoff;

		float satstate0 = clip(state[0]);
		float satstate1 = clip(state[1]);
		float satstate2 = clip(state[2]);

		dstate[0] = cutoff2Pi * (clip(input - resonance * state[3]) - satstate0);
		dstate[1] = cutoff2Pi * (satstate0 - satstate1);
		dstate[2] = cutoff2Pi * (satstate1 - satstate2);
		dstate[3] = cutoff2Pi * (satstate2 - clip(state[3]));
	}

	void process(float input, float dt) {
		float deriv1[4], deriv2[4], deriv3[4], deriv4[4], tempState[4];

		calculateDerivatives(input, deriv1, state);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + 0.5 * dt * deriv1[i];

		calculateDerivatives(input, deriv2, tempState);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + 0.5 * dt * deriv2[i];

		calculateDerivatives(input, deriv3, tempState);
		for (int i = 0; i < 4; i++)
			tempState[i] = state[i] + dt * deriv3[i];

		calculateDerivatives(input, deriv4, tempState);
		for (int i = 0; i < 4; i++)
			state[i] += (1.0 / 6.0) * dt * (deriv1[i] + 2.0 * deriv2[i] + 2.0 * deriv3[i] + deriv4[i]);
	}
	void reset() {
		for (int i = 0; i < 4; i++) {
			state[i] = 0.0;
		}
	}
};


struct PhasedLockedLoop : Module {
	enum ParamIds {
		VCO_FREQ_PARAM,
		LPF_FREQ_PARAM,
		COMPARATOR_TYPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		VCO_CV_INPUT,
		PHASE_COMPARATOR_INPUT,
		SIGNAL_INPUT,
		LPF_FREQ_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SQUARE_OUTPUT,
		COMPARATOR_OUTPUT,
		LPF_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_LOCKED_LIGHT,
		XOR_COMPARATOR_LIGHT,
		FLIP_FLOP_COMPARATOR_LIGHT,
		NUM_LIGHTS
	};
	enum ComparatorTypes {
		XOR_COMPARATOR,
		FLIP_FLOP_COMARATOR
	};

	VoltageControlledOscillator<16,16> oscillator;
	PhaseComparator comparator;
	LadderFilter filter;

	SchmittTrigger modeTrigger;
	float filterOutput = 0;
	int currentComparatorType = XOR_COMPARATOR;

	
	PhasedLockedLoop() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "comparatorType", json_integer((int) currentComparatorType));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "comparatorType");
		if (sumJ)
			currentComparatorType = json_integer_value(sumJ);

	}


	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void PhasedLockedLoop::step() {
	// Modes
	if (modeTrigger.process(params[COMPARATOR_TYPE_PARAM].value)) {
		currentComparatorType = (currentComparatorType + 1) % 2; //only 2...for now!!!
	}
	lights[XOR_COMPARATOR_LIGHT].value = currentComparatorType == XOR_COMPARATOR ? 1.0 : 0.0;
	lights[FLIP_FLOP_COMPARATOR_LIGHT].value = currentComparatorType == FLIP_FLOP_COMARATOR ? 1.0 : 0.0;

	float pitchCv;
	if (inputs[VCO_CV_INPUT].active) {
		pitchCv =  12.0 * inputs[VCO_CV_INPUT].value;
	} else {
		pitchCv = 12.0 * filterOutput;
	}
	//pitchCv = 0.0;// Test

	oscillator.setPitch(params[VCO_FREQ_PARAM].value, pitchCv);

	oscillator.process(1.0 / engineGetSampleRate());


	float squareOutput = 5.0 * oscillator.sqr(); //Used a lot :)
	outputs[SQUARE_OUTPUT].value = squareOutput;

	//normally use internally genrated square wave, unless the input is being used
	float phaseComparatorData; //
	if(inputs[PHASE_COMPARATOR_INPUT].active) {
		phaseComparatorData = inputs[PHASE_COMPARATOR_INPUT].value;
	} else {
		phaseComparatorData = squareOutput;
	}
	comparator.setData(phaseComparatorData);

	//This is what we compare either the internal square wave, or alternate input too
	if(inputs[SIGNAL_INPUT].active) {
		comparator.setClock(inputs[SIGNAL_INPUT].value);
	}

	float comparatorOutput;
	switch (currentComparatorType) {
		case XOR_COMPARATOR :
			comparatorOutput = comparator.XORoutput();
			break;
		case FLIP_FLOP_COMARATOR :
			comparatorOutput = comparator.FlipFlopOutput();
			break;
		default:
			comparatorOutput = comparator.XORoutput();
			break;
	}
	outputs[COMPARATOR_OUTPUT].value = comparatorOutput;
	lights[PHASE_LOCKED_LIGHT].value = ((comparatorOutput >= 0.0  && phaseComparatorData >= 0.0) || (comparatorOutput < 0.0  && phaseComparatorData < 0.0));

	//feed comparator into the filter
	float filterInput = comparatorOutput / 5.0;

	
	// Set cutoff frequency
	float cutoffExp = params[LPF_FREQ_PARAM].value;
	if (inputs[LPF_FREQ_INPUT].active) {
		cutoffExp += (inputs[LPF_FREQ_INPUT].value / 5);
	}
	cutoffExp = clamp(cutoffExp, 0.0f, 1.0f);
	const float minCutoff = 15.0;
	const float maxCutoff = 8400.0;
	filter.cutoff = minCutoff * powf(maxCutoff / minCutoff, cutoffExp);

	// Push a sample to the state filter
	filter.process(filterInput, 1.0/engineGetSampleRate());

	// Set outputs
	filterOutput = 5.0 * filter.state[3];
	outputs[LPF_OUTPUT].value = filterOutput;


}

struct PhasedLockedLoopWidget : ModuleWidget {
	PhasedLockedLoopWidget(PhasedLockedLoop *module);
};

PhasedLockedLoopWidget::PhasedLockedLoopWidget(PhasedLockedLoop *module) : ModuleWidget(module) {
	box.size = Vec(15*10, RACK_GRID_HEIGHT);
	

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/PhasedLockedLoop.svg")));	
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(97, 47), module, PhasedLockedLoop::VCO_FREQ_PARAM, -54.0, 54.0, 0.0));
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(97, 288), module, PhasedLockedLoop::LPF_FREQ_PARAM, 0, 1, 0.5));
	addParam(ParamWidget::create<CKD6>(Vec(19, 196), module, PhasedLockedLoop::COMPARATOR_TYPE_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(10, 50), Port::INPUT, module, PhasedLockedLoop::VCO_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 137), Port::INPUT, module, PhasedLockedLoop::PHASE_COMPARATOR_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 166), Port::INPUT, module, PhasedLockedLoop::SIGNAL_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 289), Port::INPUT, module, PhasedLockedLoop::LPF_FREQ_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(10, 78), Port::OUTPUT, module, PhasedLockedLoop::SQUARE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10, 239), Port::OUTPUT, module, PhasedLockedLoop::COMPARATOR_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10, 319), Port::OUTPUT, module, PhasedLockedLoop::LPF_OUTPUT));

	addChild(ModuleLightWidget::create<LargeLight<BlueLight>>(Vec(112, 154), module, PhasedLockedLoop::PHASE_LOCKED_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(62, 210), module, PhasedLockedLoop::XOR_COMPARATOR_LIGHT));
	addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(62, 226), module, PhasedLockedLoop::FLIP_FLOP_COMPARATOR_LIGHT));
}

Model *modelPhasedLockedLoop = Model::create<PhasedLockedLoop, PhasedLockedLoopWidget>("Frozen Wasteland", "PhasedLockedLoop", "Phased Locked Loop", OSCILLATOR_TAG);
