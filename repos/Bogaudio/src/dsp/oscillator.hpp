#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <vector>

#include "base.hpp"
#include "noise.hpp"

namespace bogaudio {
namespace dsp {

struct OscillatorGenerator : Generator {
	float _sampleRate;
	float _frequency;

	OscillatorGenerator(
		float sampleRate,
		float frequency
	)
	: _sampleRate(sampleRate > 1.0 ? sampleRate : 1.0)
	, _frequency(frequency)
	{
	}

	void setSampleRate(float sampleRate) {
		if (_sampleRate != sampleRate && sampleRate >= 1.0) {
			_sampleRate = sampleRate;
			_sampleRateChanged();
		}
	}

	virtual void _sampleRateChanged() {}

	void setFrequency(float frequency) {
		if (_frequency != frequency) {
			_frequency = frequency;
			_frequencyChanged();
		}
	}

	virtual void _frequencyChanged() {}
};

struct Phasor : OscillatorGenerator {
	static constexpr float maxPhase = 2.0f;

	float _delta;
	double _phase = 0.0;

	Phasor(
		float sampleRate,
		float frequency,
		float initialPhase = 0.0
	)
	: OscillatorGenerator(sampleRate, frequency)
	{
		setPhase(initialPhase);
		_updateDelta();
	}

	virtual void _sampleRateChanged() override {
		_updateDelta();
	}

	virtual void _frequencyChanged() override {
		_updateDelta();
	}

	void setPhase(float radians);
	float nextFromPhasor(const Phasor& phasor, float offset = 0.0f); // offset is not radians, but local phase.
	void _updateDelta();
	virtual float _next() override final;
	virtual float _nextForPhase(float phase);

	static float radiansToPhase(float radians) { return radians / M_PI; }
};

struct SineOscillator : OscillatorGenerator {
	double _k1, _k2;
	double _x;
	double _y;
	double _amplitude;

	SineOscillator(
		double sampleRate,
		double frequency,
		double amplitude = 1.0,
		double initialPhase = 0.0
	)
	: OscillatorGenerator(sampleRate, frequency)
	, _amplitude(amplitude)
	{
		setPhase(initialPhase);
		update();
	}

	virtual void _sampleRateChanged() override {
		update();
	}

	virtual void _frequencyChanged() override {
		update();
	}

	void setPhase(double phase);
	void update();
	virtual float _next() override;
};

struct SawOscillator : Phasor {
	const float halfMaxPhase = 0.5f * maxPhase;
	float _amplitude;

	SawOscillator(
		float sampleRate,
		float frequency,
		float amplitude = 1.0
	)
	: Phasor(sampleRate, frequency)
	, _amplitude(amplitude)
	{
	}

	virtual float _nextForPhase(float phase) override;
};

struct SquareOscillator : Phasor {
	float _amplitude;
	float _negativeAmplitude;
	float _pulseWidth = 0.5;
	bool positive = true;

	SquareOscillator(
		float sampleRate,
		float frequency,
		float amplitude = 1.0
	)
	: Phasor(sampleRate, frequency)
	, _amplitude(amplitude)
	, _negativeAmplitude(-amplitude)
	{
	}

	void setPulseWidth(float pw);

	virtual float _nextForPhase(float phase) override;
};

struct TriangleOscillator : Phasor {
	const float quarterMaxPhase = 0.25f * maxPhase;
	const float threeQuartersMaxPhase = 0.75f * maxPhase;
	const float twiceMaxPhase = 2.0f * maxPhase;
	float _amplitude;

	TriangleOscillator(
		float sampleRate,
		float frequency,
		float amplitude = 1.0
	)
	: Phasor(sampleRate, frequency)
	, _amplitude(amplitude)
	{
	}

	virtual float _nextForPhase(float phase) override;
};

struct SineBankOscillator : OscillatorGenerator {
	struct Partial {
		float frequency;
		float frequencyRatio;
		float amplitude;
		float amplitudeTarget;
		float amplitudeStepDelta;
		int amplitudeSteps;
		SineOscillator sine;

		Partial()
		: frequency(0.0)
		, frequencyRatio(0.0)
		, amplitude(0.0)
		, amplitudeTarget(0.0)
		, amplitudeStepDelta(0.0)
		, amplitudeSteps(0)
		, sine(0.0, 0.0, 1.0)
		{}
	};

	const float _maxPartialFrequencySRRatio = 0.48;
	float _maxPartialFrequency = 0.0;
	const int _amplitudeEnvelopeMS = 10;
	int _amplitudeEnvelopeSamples = 0;
	std::vector<Partial> _partials;

	SineBankOscillator(
		float sampleRate,
		float frequency,
		int partialCount
	)
	: OscillatorGenerator(sampleRate, frequency)
	, _partials(partialCount)
	{
		_sampleRateChanged();
		_frequencyChanged();
	}

	int partialCount() {
		return _partials.size();
	}

	// one-based indexes.
	void setPartial(int i, float frequencyRatio, float amplitude);
	void setPartialFrequencyRatio(int i, float frequencyRatio);
	void setPartialAmplitude(int i, float amplitude, bool envelope = false);

	void syncToPhase(float phase);

	virtual void _sampleRateChanged() override;
	virtual void _frequencyChanged() override;
	virtual float _next() override;
};

} // namespace dsp
} // namespace bogaudio
