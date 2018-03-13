#include "FrozenWasteland.hpp"
#include "dsp/decimator.hpp"
#include "dsp/digital.hpp"
#include "filters/biquad.h"

using namespace std;

#define BANDS 4
#define FREQUENCIES 3

struct DamianLillard : Module {
	enum ParamIds {
		FREQ_1_CUTOFF_PARAM,
		FREQ_2_CUTOFF_PARAM,
		FREQ_3_CUTOFF_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SIGNAL_IN,
		FREQ_1_CUTOFF_INPUT,		
		FREQ_2_CUTOFF_INPUT,		
		FREQ_3_CUTOFF_INPUT,		
		BAND_1_RETURN_INPUT,		
		BAND_2_RETURN_INPUT,
		BAND_3_RETURN_INPUT,
		BAND_4_RETURN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		BAND_1_OUTPUT,
		BAND_2_OUTPUT,
		BAND_3_OUTPUT,
		BAND_4_OUTPUT,
		MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};
	Biquad* iFilter[3*BANDS];
	float freq[FREQUENCIES] = {0};
	float lastFreq[FREQUENCIES] = {0};
	float output[BANDS] = {0};

	int bandOffset = 0;

	DamianLillard() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		iFilter[0] = new Biquad(bq_type_highshelf, 0 / engineGetSampleRate(), 5, 6);
		iFilter[1] = new Biquad(bq_type_highshelf, 0 / engineGetSampleRate(), 5, 6);

		iFilter[2] = new Biquad(bq_type_highpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[3] = new Biquad(bq_type_highpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[4] = new Biquad(bq_type_lowpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[5] = new Biquad(bq_type_lowpass, 0 / engineGetSampleRate(), 5, 6);

		iFilter[6] = new Biquad(bq_type_highpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[7] = new Biquad(bq_type_highpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[8] = new Biquad(bq_type_lowpass, 0 / engineGetSampleRate(), 5, 6);
		iFilter[9] = new Biquad(bq_type_lowpass, 0 / engineGetSampleRate(), 5, 6);

		iFilter[10] = new Biquad(bq_type_lowshelf, 0 / engineGetSampleRate(), 5, 6);
		iFilter[11] = new Biquad(bq_type_lowshelf, 0 / engineGetSampleRate(), 5, 6);
	}

	void step() override;
};

void DamianLillard::step() {
	
	float signalIn = inputs[SIGNAL_IN].value/5;
	float out = 0.0;

	const float minCutoff = 15.0;
	const float maxCutoff = 8400.0;
	
	for (int i=0; i<FREQUENCIES;i++) {
		float cutoffExp = params[FREQ_1_CUTOFF_PARAM+i].value + inputs[FREQ_1_CUTOFF_INPUT+i].value / 10.0; //I'm reducing range of CV to make it more useful
		cutoffExp = clamp(cutoffExp, 0.0, 1.0);
		freq[i] = minCutoff * powf(maxCutoff / minCutoff, cutoffExp);

		//Prevent band overlap
		if(i>0 && freq[i] < lastFreq[i-1]) {
			freq[i] = lastFreq[i-1]+1;
		}
		if(i<FREQUENCIES-1 && freq[i] > lastFreq[i+1]) {
			freq[i] = lastFreq[i+1]-1;
		}

		if(freq[i] != lastFreq[i]) {
			float Fc = freq[i] / engineGetSampleRate();
			for(int j=0;j<BANDS;j++) {
				iFilter[(i*BANDS)+j]->setFc(Fc);
			}
			lastFreq[i] = freq[i];
		}
	}

	output[0] = iFilter[0]->process(iFilter[1]->process(signalIn)) * 5;
	output[1] = iFilter[2]->process(iFilter[3]->process(iFilter[4]->process(iFilter[5]->process(signalIn)))) * 5;	
	output[2] = iFilter[6]->process(iFilter[7]->process(iFilter[8]->process(iFilter[9]->process(signalIn)))) * 5;	
	output[3] = iFilter[10]->process(iFilter[11]->process(signalIn)) * 5;	

	for(int i=0; i<BANDS; i++) {		
		outputs[BAND_1_OUTPUT+i].value = output[i];

		if(inputs[BAND_1_RETURN_INPUT+i].active) {
			out += inputs[BAND_1_RETURN_INPUT+i].value;
		} else {
			out += output[i];
		}
	}

	outputs[MIX_OUTPUT].value = out / 4.0;
	
}


struct DamianLillardBandDisplay : TransparentWidget {
	DamianLillard *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	DamianLillardBandDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/01 Digit.ttf"));
	}

	void drawFrequency(NVGcontext *vg, Vec pos, float cutoffFrequency) {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0x00, 0xff, 0x00, 0xff));
		char text[128];
		snprintf(text, sizeof(text), " % 4.0f", cutoffFrequency);
		nvgText(vg, pos.x + 8, pos.y, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		for(int i=0;i<FREQUENCIES;i++) {
			drawFrequency(vg, Vec(i * 46.0, box.size.y - 75), module->freq[i]);
		}
	}
};

struct DamianLillardWidget : ModuleWidget {
	DamianLillardWidget(DamianLillard *module);
};

DamianLillardWidget::DamianLillardWidget(DamianLillard *module) : ModuleWidget(module) {
	box.size = Vec(15*11, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DamianLillard.svg")));
		addChild(panel);
	}
	
	{
		DamianLillardBandDisplay *offsetDisplay = new DamianLillardBandDisplay();
		offsetDisplay->module = module;
		offsetDisplay->box.pos = Vec(15, 10);
		offsetDisplay->box.size = Vec(box.size.x, 140);
		addChild(offsetDisplay);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(14, 84), module, DamianLillard::FREQ_1_CUTOFF_PARAM, 0, 1.0, .25));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(64, 84), module, DamianLillard::FREQ_2_CUTOFF_PARAM, 0, 1.0, .5));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(115, 84), module, DamianLillard::FREQ_3_CUTOFF_PARAM, 0, 1.0, .75));


	addInput(Port::create<PJ301MPort>(Vec(20, 125), Port::INPUT, module, DamianLillard::FREQ_1_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(71, 125), Port::INPUT, module, DamianLillard::FREQ_2_CUTOFF_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(123, 125), Port::INPUT, module, DamianLillard::FREQ_3_CUTOFF_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(10, 170), Port::INPUT, module, DamianLillard::SIGNAL_IN));


	addInput(Port::create<PJ301MPort>(Vec(10, 265), Port::INPUT, module, DamianLillard::BAND_1_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 265), Port::INPUT, module, DamianLillard::BAND_2_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(90, 265), Port::INPUT, module, DamianLillard::BAND_3_RETURN_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(130, 265), Port::INPUT, module, DamianLillard::BAND_4_RETURN_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(10, 225), Port::OUTPUT, module, DamianLillard::BAND_1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(50, 225), Port::OUTPUT, module, DamianLillard::BAND_2_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(90, 225), Port::OUTPUT, module, DamianLillard::BAND_3_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(130, 225), Port::OUTPUT, module, DamianLillard::BAND_4_OUTPUT));

	addOutput(Port::create<PJ301MPort>(Vec(10, 305), Port::OUTPUT, module, DamianLillard::MIX_OUTPUT));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
}

Model *modelDamianLillard = Model::create<DamianLillard, DamianLillardWidget>("Frozen Wasteland", "DamianLillard", "Damian Lillard", FILTER_TAG);
