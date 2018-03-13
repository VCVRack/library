#pragma once

#include "bogaudio.hpp"

extern Model* modelTest;

// #define LPF 1
// #define LPFNOISE 1
#define SINE 1
// #define SQUARE 1
// #define SAW 1
// #define TRIANGLE 1
// #define SINEBANK 1
// #define OVERSAMPLING 1

#include "pitch.hpp"
#ifdef LPF
#include "dsp/filter.hpp"
#elif LPFNOISE
#include "dsp/filter.hpp"
#include "dsp/noise.hpp"
#elif SINE
#include "dsp/oscillator.hpp"
#elif SQUARE
#include "dsp/oscillator.hpp"
#elif SAW
#include "dsp/oscillator.hpp"
#elif TRIANGLE
#include "dsp/oscillator.hpp"
#elif SINEBANK
#include "dsp/oscillator.hpp"
#elif OVERSAMPLING
#include "dsp/oscillator.hpp"
#include "dsp/decimator.hpp" // rack
#include "dsp/filter.hpp"
#define OVERSAMPLEN 16
#else
#error what
#endif

using namespace bogaudio::dsp;

namespace bogaudio {


struct Test : Module {
	enum ParamsIds {
		PARAM1_PARAM,
		PARAM2_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		CV2_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

#ifdef LPF
	LowPassFilter _lpf;
#elif LPFNOISE
	WhiteNoiseGenerator _noise;
	LowPassFilter _lpf;
#elif SINE
	SineOscillator _sine;
	SineOscillator _sine2;
#elif SQUARE
	SquareOscillator _square;
#elif SAW
	SawOscillator _saw;
#elif TRIANGLE
	TriangleOscillator _triangle;
#elif SINEBANK
	SineBankOscillator _sineBank;
#elif OVERSAMPLING
	SawOscillator _saw1;
	rack::Decimator<OVERSAMPLEN, OVERSAMPLEN> _rackDecimator;
	SawOscillator _saw2;
	LowPassFilter _lpf;
	LowPassFilter _lpf2;
#endif

	Test()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
#ifdef LPF
	, _lpf(44100.0, 1000.0, 1.0)
#elif LPFNOISE
	, _lpf(44100.0, 1000.0, 1.0)
#elif SINE
	, _sine(44100.0, 1000.0, 5.0)
	, _sine2(44100.0, 1000.0, 1.0, M_PI/1.0f)
#elif SQUARE
	, _square(44100.0, 1000.0, 5.0)
#elif SAW
	, _saw(44100.0, 1000.0, 5.0)
#elif TRIANGLE
	, _triangle(44100.0, 1000.0, 5.0)
#elif SINEBANK
	, _sineBank(44101.0, 1000.0, 50)
#elif OVERSAMPLING
	, _saw1(44100.0, 1000.0, 5.0)
	, _saw2(44100.0, 1000.0, 1.0)
	, _lpf(44100.0, 1000.0, 1.0)
	, _lpf2(44100.0, 1000.0, 1.0)
#endif
	{
		onReset();

#ifdef SINEBANK
		const float baseAmplitude = 5.0;
		switch (5) {
			case 1: {
				// saw
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, baseAmplitude / (float)i);
				}
				_sineBank.syncToPhase(M_PI);
				break;
			}

			case 2: {
				// square
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)i : 0.0);
				}
				break;
			}

			case 3: {
				// triangle
				if (false) {
					for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
						_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)(i * i) : 0.0);
					}
					_sineBank.syncToPhase(M_PI / 2.0);
				}
				else {
					_sineBank.setPartial(1, 1.0f, baseAmplitude);
					for (int i = 2, n = _sineBank.partialCount(); i < n; ++i) {
						float k = 2*i - 1;
						_sineBank.setPartial(i, k, powf(-1.0f, k) * baseAmplitude/(k * k));
					}
				}
				break;
			}

			case 4: {
				// saw-square
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, i, i % 2 == 1 ? baseAmplitude / (float)i : baseAmplitude / (float)(2 * i));
				}
				break;
			}

			case 5: {
				// ?
				float factor = 0.717;
				float factor2 = factor;
				float multiple = 1.0;
				for (int i = 1, n = _sineBank.partialCount(); i <= n; ++i) {
					_sineBank.setPartial(i, multiple, baseAmplitude / multiple);
					multiple += i % 2 == 1 ? factor : factor2;
				}
				break;
			}
		}
#endif
	}

	virtual void onReset() override;
	virtual void step() override;
	float oscillatorPitch();
};

} // namespace bogaudio
