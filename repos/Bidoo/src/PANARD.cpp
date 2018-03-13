#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include "osdialog.h"
#include "dep/audiofile/AudioFile.h"
#include <vector>
#include "cmath"
#include <iomanip> // setprecision
#include <sstream> // stringstream

using namespace std;


struct PANARD : Module {
	enum ParamIds {
		RECORD_PARAM,
		SAMPLE_START_PARAM,
		LOOP_LENGTH_PARAM,
		READ_MODE_PARAM,
		SPEED_PARAM,
		FADE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INL_INPUT,
		INR_INPUT,
		TRIG_INPUT,
		SAMPLE_START_INPUT,
		LOOP_LENGTH_INPUT,
		READ_MODE_INPUT,
		SPEED_INPUT,
		RECORD_INPUT,
		FADE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool play = false;
	bool record = false;
	AudioFile<float> playBuffer, recordBuffer;
	float samplePos = 0.0f, sampleStart = 0.0f, loopLength = 0.0f, fadeLenght = 0.0f, fadeCoeff = 1.0f;
	vector<float> displayBuffL;
	vector<float> displayBuffR;
	int readMode = 0; // 0 formward, 1 backward, 2 repeat
	float speed;

	SchmittTrigger readModeTrigger;
	SchmittTrigger recordTrigger;


	PANARD() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		recordBuffer.setBitDepth(16);
		recordBuffer.setSampleRate(engineGetSampleRate());
		recordBuffer.setNumChannels(2);
		recordBuffer.samples[0].resize(0);
		recordBuffer.samples[1].resize(0);
		playBuffer.setBitDepth(16);
		playBuffer.setSampleRate(engineGetSampleRate());
		playBuffer.setNumChannels(2);
		playBuffer.samples[0].resize(0);
		playBuffer.samples[1].resize(0);
	}

	void step() override;

	void displaySample();

	// persistence

	json_t *toJson() override {
		json_t *rootJ = json_object();
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
	}
};

void PANARD::displaySample() {
		vector<float>().swap(displayBuffL);
		vector<float>().swap(displayBuffR);
		for (int i=0; i < playBuffer.getNumSamplesPerChannel(); i = i + floor(playBuffer.getNumSamplesPerChannel()/170)) {
			displayBuffL.push_back(playBuffer.samples[0][i]);
			displayBuffR.push_back(playBuffer.samples[1][i]);
		}
}

void PANARD::step() {
	if (recordTrigger.process(inputs[RECORD_INPUT].value + params[RECORD_PARAM].value))
	{
		if(record) {
			playBuffer.setAudioBuffer(recordBuffer.samples);
			recordBuffer.samples[0].resize(0);
			recordBuffer.samples[1].resize(0);
			displaySample();
		}
		record = !record;
	}

	if (record) {
		recordBuffer.samples[0].push_back(inputs[INL_INPUT].value);
		recordBuffer.samples[1].push_back(inputs[INR_INPUT].value);
	}

	loopLength = rescale(clamp(inputs[LOOP_LENGTH_INPUT].value + params[LOOP_LENGTH_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f, 0.0f, playBuffer.getNumSamplesPerChannel() > 0.0f ? playBuffer.getNumSamplesPerChannel() : 0.0f);
	sampleStart = rescale(clamp(inputs[SAMPLE_START_INPUT].value + params[SAMPLE_START_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f, 0.0f, playBuffer.getNumSamplesPerChannel() > 0.0f ? playBuffer.getNumSamplesPerChannel() - loopLength - 1.0f : 0.0f);
	fadeLenght = rescale(clamp(inputs[FADE_INPUT].value + params[FADE_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f,0.0f, floor(loopLength/2));

	if ((inputs[TRIG_INPUT].active) && (inputs[TRIG_INPUT].value>0)) {
		if (samplePos<playBuffer.getNumSamplesPerChannel()) {
			if (fadeLenght>0) {
				if ((samplePos-sampleStart)<fadeLenght)
					fadeCoeff = rescale(samplePos-sampleStart,0.0f,fadeLenght-1,0.0f,1.0f);
				else if (((sampleStart+loopLength)-samplePos)<fadeLenght)
					fadeCoeff = rescale((sampleStart+loopLength)-samplePos,fadeLenght-1,0.0f,1.0f,0.0f);
				else
					fadeCoeff = 1.0f;
			}
			else
				fadeCoeff = 1.0f;

			outputs[OUTL_OUTPUT].value = playBuffer.samples[0][floor(samplePos)]*fadeCoeff;
			outputs[OUTR_OUTPUT].value = playBuffer.samples[1][floor(samplePos)]*fadeCoeff;
		}
		if (samplePos >= (sampleStart+loopLength))
			samplePos = sampleStart;
	  else
		  samplePos = samplePos + inputs[SPEED_INPUT].value + params[SPEED_PARAM].value;

		if (samplePos<sampleStart)
			samplePos = sampleStart + loopLength - 1;
	}
	else {
		outputs[OUTL_OUTPUT].value = 0.0f;
		outputs[OUTR_OUTPUT].value = 0.0f;
	}
}

struct PANARDDisplay : TransparentWidget {
	PANARD *module;
	int frame = 0;
	shared_ptr<Font> font;
	string displayParams;

	PANARDDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgStrokeWidth(vg, 1);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, YELLOW_BIDOO);

		const int width = 175;
		const int height = 50;

		// Draw play line
		nvgStrokeColor(vg, LIGHTBLUE_BIDOO);
		{
			nvgBeginPath(vg);
			nvgStrokeWidth(vg, 2);
			if (module->playBuffer.getNumSamplesPerChannel()>0) {
				nvgMoveTo(vg, (int)(module->samplePos * width / module->playBuffer.getNumSamplesPerChannel()) , 0);
				nvgLineTo(vg, (int)(module->samplePos * width / module->playBuffer.getNumSamplesPerChannel()) , 2*height+10);
			}
			else {
				nvgMoveTo(vg, 0, 0);
				nvgLineTo(vg, 0, 2*height+10);
			}

			nvgClosePath(vg);
		}
		nvgStroke(vg);


		// Draw ref line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, height/2);
			nvgLineTo(vg, width, height/2);
			nvgMoveTo(vg, 0, 3*height/2+10);
			nvgLineTo(vg, width, 3*height/2+10);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		// Draw loop
		nvgFillColor(vg, YELLOW_BIDOO_LIGHT);
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, (int)((module->sampleStart + module->fadeLenght) * width / module->playBuffer.getNumSamplesPerChannel()) , 0);
			nvgLineTo(vg, (int)(module->sampleStart * width / module->playBuffer.getNumSamplesPerChannel()) , 2*height+10);
			nvgLineTo(vg, (int)((module->sampleStart + module->loopLength) * width / module->playBuffer.getNumSamplesPerChannel()) , 2*height+10);
			nvgLineTo(vg, (int)((module->sampleStart + module->loopLength - module->fadeLenght) * width / module->playBuffer.getNumSamplesPerChannel()) , 0);
			nvgLineTo(vg, (int)((module->sampleStart + module->fadeLenght) * width / module->playBuffer.getNumSamplesPerChannel()) , 0);
			nvgClosePath(vg);
		}
		nvgFill(vg);

		// Draw waveform
		if (module->displayBuffL.size()>0) {
			nvgStrokeColor(vg, PINK_BIDOO);
			nvgSave(vg);
			Rect b = Rect(Vec(0, 0), Vec(width, height));
			nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
			nvgBeginPath(vg);
			for (unsigned int i = 0; i < module->displayBuffL.size(); i++) {
				float x, y;
				x = (float)i / (module->displayBuffL.size());
				y = module->displayBuffL[i] / 2.0f + 0.5f;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0f - y);
				if (i == 0)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}
			nvgLineCap(vg, NVG_ROUND);
			nvgMiterLimit(vg, 2.0);
			nvgStrokeWidth(vg, 1);
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgStroke(vg);

			b = Rect(Vec(0, height+10), Vec(width, height));
			nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
			nvgBeginPath(vg);
			for (unsigned int i = 0; i < module->displayBuffR.size(); i++) {
				float x, y;
				x = (float)i / (module->displayBuffR.size());
				y = module->displayBuffR[i] / 2.0f + 0.5f;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0f - y);
				if (i == 0)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}
			nvgLineCap(vg, NVG_ROUND);
			nvgMiterLimit(vg, 2.0f);
			nvgStrokeWidth(vg, 1);
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgStroke(vg);
			nvgResetScissor(vg);
			nvgRestore(vg);
		}
	}
};

struct PANARDWidget : ModuleWidget {
	PANARDWidget(PANARD *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PANARD.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			PANARDDisplay *display = new PANARDDisplay();
			display->module = module;
			display->box.pos = Vec(10, 35);
			display->box.size = Vec(175, 150);
			addChild(display);
		}

		static const float portX0[4] = {32, 71, 110, 149};

		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[0]-5, 170), module, PANARD::RECORD_PARAM, 0.0f, 1.0f, 0.0f));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[2]-6, 170), module, PANARD::SAMPLE_START_PARAM, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[3]-6, 170), module, PANARD::LOOP_LENGTH_PARAM, 0.0f, 10.0f, 10.0f));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-4, 202), Port::INPUT, module, PANARD::RECORD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-4, 202), Port::INPUT, module, PANARD::TRIG_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[2]-4, 202), Port::INPUT, module, PANARD::SAMPLE_START_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[3]-4, 202), Port::INPUT, module, PANARD::LOOP_LENGTH_INPUT));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[0]-6, 245), module, PANARD::SPEED_PARAM, -10.0f, 10.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]-6, 245), module, PANARD::FADE_PARAM, 0.0f, 10.0f, 0.0f));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-4, 277), Port::INPUT, module, PANARD::SPEED_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-4, 277), Port::INPUT, module, PANARD::FADE_INPUT));

		addInput(Port::create<TinyPJ301MPort>(Vec(19, 331), Port::INPUT, module, PANARD::INL_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(19+24, 331), Port::INPUT, module, PANARD::INR_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(138, 331), Port::OUTPUT, module, PANARD::OUTL_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(138+24, 331), Port::OUTPUT, module, PANARD::OUTR_OUTPUT));
	}
};

Model *modelPANARD = Model::create<PANARD, PANARDWidget>("Bidoo","pANARd", "pANARd sampler", SAMPLER_TAG);
