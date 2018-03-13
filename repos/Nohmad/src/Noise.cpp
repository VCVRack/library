#include "Nohmad.hpp"

#include "dsp/filter.hpp"

#include <random>
#include <cmath>

struct NoiseGenerator {
	std::mt19937 rng;
	std::uniform_real_distribution<float> uniform;

	NoiseGenerator() : uniform(-1.0, 1.0) {
		rng.seed(std::random_device()());
	}

	float white() {
		return uniform(rng);
	}
};

struct PinkFilter {
	float b0, b1, b2, b3, b4, b5, b6; // Coefficients
	float y; // Out

	void process(float x) {
		b0 = 0.99886 * b0 + x * 0.0555179; 
		b1 = 0.99332 * b1 + x * 0.0750759; 
		b2 = 0.96900 * b2 + x * 0.1538520; 
		b3 = 0.86650 * b3 + x * 0.3104856; 
		b4 = 0.55000 * b4 + x * 0.5329522; 
		b5 = -0.7616 * b5 - x * 0.0168980; 
		y = b0 + b1 + b2 + b3 + b4 + b5 + b6 + x * 0.5362;
		b6 = x * 0.115926; 
	}

	float pink() {
		return y;
	}
};

struct NotchFilter {
	float freq, bandwidth; // Params
	float a0, a1, a2, b1, b2; // Coefficients
	float x1, x2; // In
	float y1, y2; // out

	void setFreq(float value) {
		freq = value;
		computeCoefficients();
	}

	void setBandwidth(float value) {
		bandwidth = value;
		computeCoefficients();
	}

	void process(float x) {
		float y = a0 * x + a1 * x1 + a2 * x2 + b1 * y1 + b2 * y2;

		x2 = x1;
		x1 = x;
		y2 = y1;
		y1 = y;
	}

	float notch() {
		return y1;
	}

	void computeCoefficients() {
		float c2pf = cos(2.0 * M_PI * freq);
		float r = 1.0 - 3.0 * bandwidth;
		float r2 = r * r;
		float k = (1.0 - (2.0 * r * c2pf) + r2) / (2.0 - 2.0 * c2pf);

		a0 = k;
		a1 = -2.0 * k * c2pf;
		a2 = k;
		b1 = 2.0 * r * c2pf;
		b2 = -r2;
	}
};

struct Noise : Module {
	enum ParamIds {
		QUANTA_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		NUM_INPUTS
	};

	enum OutputIds {
		WHITE_OUTPUT,
		PINK_OUTPUT,
		RED_OUTPUT,
		GREY_OUTPUT,
		BLUE_OUTPUT,
		PURPLE_OUTPUT,
		QUANTA_OUTPUT,
		NUM_OUTPUTS
	};

	NoiseGenerator noise;

	PinkFilter pinkFilter;
	RCFilter redFilter;
	NotchFilter greyFilter;
	RCFilter blueFilter;
	RCFilter purpleFilter;

	Noise() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
		redFilter.setCutoff(441.0 / engineGetSampleRate());
		purpleFilter.setCutoff(44100.0 / engineGetSampleRate());
		blueFilter.setCutoff(44100.0 / engineGetSampleRate());
		greyFilter.setFreq(1000.0 / engineGetSampleRate());
		greyFilter.setBandwidth(0.3);
	}

	void step() override;
};

void Noise::step() {
	float white = noise.white();
	if (outputs[PINK_OUTPUT].active || outputs[BLUE_OUTPUT].active || outputs[GREY_OUTPUT].active) {
		pinkFilter.process(white);
	}

	if (outputs[WHITE_OUTPUT].active) {
		outputs[WHITE_OUTPUT].value = 5.0 * white;
	}

	if (outputs[RED_OUTPUT].active) {
		redFilter.process(white);
		outputs[RED_OUTPUT].value = 5.0 * clampf(7.8 * redFilter.lowpass(), -1.0, 1.0);
	}

	if (outputs[PINK_OUTPUT].active) {
		outputs[PINK_OUTPUT].value = 5.0 * clampf(0.18 * pinkFilter.pink(), -1.0, 1.0);
	}

	if (outputs[GREY_OUTPUT].active) {
		greyFilter.process(pinkFilter.pink() * 0.034);
		outputs[GREY_OUTPUT].value = 5.0 * clampf(0.23 * (pinkFilter.pink() * 0.5 + greyFilter.notch() * 0.5), -1.0, 1.0);
	}

	if (outputs[BLUE_OUTPUT].active) {
		blueFilter.process(pinkFilter.pink());
		outputs[BLUE_OUTPUT].value = 5.0 * clampf(0.64 * blueFilter.highpass(), -1.0, 1.0);
	}

	if (outputs[PURPLE_OUTPUT].active) {
		purpleFilter.process(white);
		outputs[PURPLE_OUTPUT].value = 5.0 * clampf(0.82 * purpleFilter.highpass(), -1.0, 1.0);
	}

	if (outputs[QUANTA_OUTPUT].active) {
		outputs[QUANTA_OUTPUT].value = abs(white) <= params[QUANTA_PARAM].value ? 5.0 * sgnf(white) : 0.0;
	}
}

struct MiniTrimpot : Trimpot  {
	MiniTrimpot() {
		box.size = Vec(12, 12);
	}
};

NoiseWidget::NoiseWidget() {
	Noise *module = new Noise();
	setModule(module);
	box.size = Vec(15 * 3, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Noise.svg")));
		addChild(panel);
	}

	addOutput(createOutput<PJ301MPort>(Vec(10.5, 55), module, Noise::WHITE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 101), module, Noise::PINK_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 150), module, Noise::RED_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 199), module, Noise::GREY_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 247), module, Noise::BLUE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 295), module, Noise::PURPLE_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(10.5, 343), module, Noise::QUANTA_OUTPUT));

	addParam(createParam<MiniTrimpot>(Vec(30, 365), module, Noise::QUANTA_PARAM, 0.0, 1.0, 0.066));
}
