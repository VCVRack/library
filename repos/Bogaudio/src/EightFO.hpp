#pragma once

#include "bogaudio.hpp"
#include "dsp/oscillator.hpp"

using namespace bogaudio::dsp;

extern Model* modelEightFO;

namespace bogaudio {

struct EightFO : Module {
	enum ParamsIds {
		FREQUENCY_PARAM,
		WAVE_PARAM,
		SAMPLE_PWM_PARAM,
		PHASE7_PARAM,
		PHASE6_PARAM,
		PHASE5_PARAM,
		PHASE4_PARAM,
		PHASE3_PARAM,
		PHASE2_PARAM,
		PHASE1_PARAM,
		PHASE0_PARAM,
		SLOW_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		SAMPLE_PWM_INPUT,
		PHASE7_INPUT,
		PHASE6_INPUT,
		PHASE5_INPUT,
		PHASE4_INPUT,
		PHASE3_INPUT,
		PHASE2_INPUT,
		PHASE1_INPUT,
		PHASE0_INPUT,
		PITCH_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		PHASE7_OUTPUT,
		PHASE6_OUTPUT,
		PHASE5_OUTPUT,
		PHASE4_OUTPUT,
		PHASE3_OUTPUT,
		PHASE2_OUTPUT,
		PHASE1_OUTPUT,
		PHASE0_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		SLOW_LIGHT,
		NUM_LIGHTS
	};

	enum Wave {
		NO_WAVE,
		SINE_WAVE,
		TRIANGLE_WAVE,
		RAMP_UP_WAVE,
		RAMP_DOWN_WAVE,
		SQUARE_WAVE
	};

	const int modulationSteps = 100;
	const float maxFrequency = 1000.0f;
	const float minFrequency = 1.0f;
	const float slowModeFactor = 0.01f;
	const float amplitude = 5.0f;
	const float basePhase7Offset = Phasor::radiansToPhase(1.75f * M_PI);
	const float basePhase6Offset = Phasor::radiansToPhase(1.5f * M_PI);
	const float basePhase5Offset = Phasor::radiansToPhase(1.25f * M_PI);
	const float basePhase4Offset = Phasor::radiansToPhase(M_PI);
	const float basePhase3Offset = Phasor::radiansToPhase(0.75f * M_PI);
	const float basePhase2Offset = Phasor::radiansToPhase(0.5f * M_PI);
	const float basePhase1Offset = Phasor::radiansToPhase(0.25f * M_PI);
	const float basePhase0Offset = Phasor::radiansToPhase(0.0f);

	int _modulationStep = 0;
	Wave _wave = NO_WAVE;
	bool _slowMode = false;
	int _sampleSteps = 1;
	int _sampleStep = 0;
	SchmittTrigger _resetTrigger;

	Phasor _phasor;
	TriangleOscillator _triangle;
	SawOscillator _ramp;
	SquareOscillator _square;

	float _phase7Offset = 0.0f;
	float _phase6Offset = 0.0f;
	float _phase5Offset = 0.0f;
	float _phase4Offset = 0.0f;
	float _phase3Offset = 0.0f;
	float _phase2Offset = 0.0f;
	float _phase1Offset = 0.0f;
	float _phase0Offset = 0.0f;

	float _phase7Sample = 0.0f;
	float _phase6Sample = 0.0f;
	float _phase5Sample = 0.0f;
	float _phase4Sample = 0.0f;
	float _phase3Sample = 0.0f;
	float _phase2Sample = 0.0f;
	float _phase1Sample = 0.0f;
	float _phase0Sample = 0.0f;

	bool _phase7Active = false;
	bool _phase6Active = false;
	bool _phase5Active = false;
	bool _phase4Active = false;
	bool _phase3Active = false;
	bool _phase2Active = false;
	bool _phase1Active = false;
	bool _phase0Active = false;

	EightFO()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	, _phasor(engineGetSampleRate(), 1.0f)
	, _triangle(0.0f, 0.0f)
	, _ramp(0.0f, 0.0f)
	, _square(0.0f, 0.0f)
	{
		onReset();
	}

	virtual void onReset() override;
	virtual void onSampleRateChange() override;
	virtual void step() override;
	float phaseOffset(Param& p, Input& i, float baseOffset);
	void updateOutput(bool useSample, Output& output, float& offset, float& sample, bool& active);
};

} // namespace bogaudio
