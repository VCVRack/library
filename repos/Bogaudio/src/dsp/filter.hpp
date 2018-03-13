#pragma once

namespace bogaudio {
namespace dsp {

struct Filter {
	Filter() {}
	virtual ~Filter() {}

	virtual float next(float sample) = 0;
};

struct BiquadFilter : Filter {
	float _b0 = 0.0;
	float _b1 = 0.0;
	float _b2 = 0.0;
	float _a1 = 0.0;
	float _a2 = 0.0 ;

	float _x[3] {};
	float _y[3] {};

	BiquadFilter() {}

	void setParams(float b0, float b1, float b2, float a0, float a1, float a2);
	virtual float next(float sample) override;
};

struct LowPassFilter : Filter {
	float _sampleRate;
	float _cutoff;
	float _q;

	BiquadFilter _biquad;

	LowPassFilter(float sampleRate, float cutoff, float q)
	: _sampleRate(sampleRate)
	, _cutoff(cutoff)
	, _q(q)
	{
	}

	void setParams(float sampleRate, float cutoff, float q);
	virtual float next(float sample) override {
		return _biquad.next(sample);
	}
};

} // namespace dsp
} // namespace bogaudio
