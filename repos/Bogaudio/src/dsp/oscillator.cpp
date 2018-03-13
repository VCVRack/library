#include <math.h>

#include "oscillator.hpp"

using namespace bogaudio::dsp;

void Phasor::setPhase(float radians) {
	_phase = radiansToPhase(radians);
}

float Phasor::nextFromPhasor(const Phasor& phasor, float offset) {
	float p = phasor._phase + offset;
	while (p >= maxPhase) {
		p -= maxPhase;
	}
	while (p < 0.0f) {
		p += maxPhase;
	}
	return _nextForPhase(p);
}

void Phasor::_updateDelta() {
	_delta = (_frequency / _sampleRate) * maxPhase;
}

float Phasor::_next() {
	_phase += _delta;
	if (_phase >= maxPhase) {
		_phase -= maxPhase;
	}
	else if (_phase <= 0.0f && _delta != 0.0f) {
		_phase += maxPhase;
	}
	return _nextForPhase(_phase);
}

float Phasor::_nextForPhase(float phase) {
	return phase;
}


// A New Recursive Quadrature Oscillator, Martin Vicanek, 2015 - http://vicanek.de/articles/QuadOsc.pdf
void SineOscillator::setPhase(double phase) {
	_x = cos(phase);
	_y = sin(phase);
}

void SineOscillator::update() {
	double w = (_frequency / _sampleRate) * 2.0 * M_PI;
	_k1 = tan(w / 2.0);
	_k2 = 2.0 * _k1 / (1 + _k1*_k1);
}

float SineOscillator::_next() {
	double t = _x - _k1*_y;
	_y = _y + _k2*t;
	_x = t - _k1*_y;
	return _amplitude * _y;
}


float SawOscillator::_nextForPhase(float phase) {
	return _amplitude * (phase - halfMaxPhase);
}


void SquareOscillator::setPulseWidth(float pw) {
	if (pw >= 0.97f) {
		_pulseWidth = 0.97f;
	}
	else if (pw <= 0.03f) {
		_pulseWidth = 0.03f;
	}
	else {
		_pulseWidth = pw;
	}
	_pulseWidth *= maxPhase;
}

float SquareOscillator::_nextForPhase(float phase) {
	if (positive) {
		if (phase >= _pulseWidth) {
			positive = false;
			return _negativeAmplitude;
		}
		return _amplitude;
	}
	if (phase < _pulseWidth) {
		positive = true;
		return _amplitude;
	}
	return _negativeAmplitude;
}


float TriangleOscillator::_nextForPhase(float phase) {
	float p = maxPhase * phase;
	if (phase < quarterMaxPhase) {
		return _amplitude * p;
	}
	if (phase < threeQuartersMaxPhase) {
		return _amplitude * (maxPhase - p);
	}
	return _amplitude * (p - twiceMaxPhase);
}


void SineBankOscillator::setPartial(int i, float frequencyRatio, float amplitude) {
	setPartialFrequencyRatio(i, frequencyRatio);
	setPartialAmplitude(i, amplitude);
}

void SineBankOscillator::setPartialFrequencyRatio(int i, float frequencyRatio) {
	if (i <= (int)_partials.size()) {
		Partial& p = _partials[i - 1];
		p.frequencyRatio = frequencyRatio;
		p.frequency = _frequency * frequencyRatio;
		p.sine.setFrequency((double)_frequency * (double)frequencyRatio);
	}
}

void SineBankOscillator::setPartialAmplitude(int i, float amplitude, bool envelope) {
	if (i <= (int)_partials.size()) {
		Partial& p = _partials[i - 1];
		if (envelope) {
			p.amplitudeTarget = amplitude;
			p.amplitudeStepDelta = (amplitude - p.amplitude) / (float)_amplitudeEnvelopeSamples;
			p.amplitudeSteps = _amplitudeEnvelopeSamples;
		}
		else if (p.amplitudeSteps > 0) {
			p.amplitudeTarget = amplitude;
			p.amplitudeStepDelta = (amplitude - p.amplitude) / (float)p.amplitudeSteps;
		}
		else {
			p.amplitude = amplitude;
		}
	}
}

void SineBankOscillator::syncToPhase(float phase) {
	for (Partial& p : _partials) {
		p.sine.setPhase(phase);
	}
}

void SineBankOscillator::_sampleRateChanged() {
	_maxPartialFrequency = _maxPartialFrequencySRRatio * _sampleRate;
	_amplitudeEnvelopeSamples = _sampleRate * (_amplitudeEnvelopeMS / 1000.0f);
	for (Partial& p : _partials) {
		p.sine.setSampleRate(_sampleRate);
	}
}

void SineBankOscillator::_frequencyChanged() {
	for (Partial& p : _partials) {
		p.frequency = _frequency * p.frequencyRatio;
		p.sine.setFrequency(_frequency * p.frequencyRatio);
	}
}

float SineBankOscillator::_next() {
	float next = 0.0;
	for (Partial& p : _partials) {
		if (p.frequency < _maxPartialFrequency && (p.amplitude > 0.001 || p.amplitude < -0.001 || p.amplitudeSteps > 0)) {
			if (p.amplitudeSteps > 0) {
				if (p.amplitudeSteps == 1) {
					p.amplitude = p.amplitudeTarget;
				}
				else {
					p.amplitude += p.amplitudeStepDelta;
				}
				--p.amplitudeSteps;
			}
			next += p.sine.next() * p.amplitude;
		}
		else {
			p.sine.next(); // keep spinning, maintain phase.
		}
	}
	return next;
}
