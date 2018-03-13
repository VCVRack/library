#include "../PQ.hpp"
#include "rtl-sdr.h"
#include <stdio.h>
#include <limits.h>
#include <cstring>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "dsp/ringbuffer.hpp"
#define HZ_CEIL 110.0
#define HZ_FLOOR 80.0
#define HZ_SPAN (HZ_CEIL-HZ_FLOOR)
#define HZ_CENTER (HZ_FLOOR+0.5*HZ_SPAN)
#define MAX_VOLTAGE 5.0

struct MyLabel : Widget {
	std::string text;
	int fontSize;
	NVGcolor color = nvgRGB(255,20,20);
	MyLabel(int _fontSize = 18) {
		fontSize = _fontSize;
	}
	void draw(NVGcontext *vg) override {
		nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_BASELINE);
		nvgFillColor(vg, color);
		nvgFontSize(vg, fontSize);
		nvgText(vg, box.pos.x, box.pos.y, text.c_str(), NULL);
	}
};

struct SDR : Module {
	enum ParamIds {
		TUNE_PARAM,
		TUNE_ATT,
		QUANT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TUNE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	RtlSdr radio;
	rack::RingBuffer<int16_t, 16384> buffer;
	long currentFreq;

	SDR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		buffer.clear();
  }
	~SDR() {
		RtlSdr_end(&radio);
	}
	void onSampleRateChange() override;
	void step() override;
	void openFile();
	long getFreq(float);
	float getMegaFreq(long);
	MyLabel* linkedLabel;
	long stepCount;
};
void SDR::step() {

	if (radio.rack_buffer==NULL && stepCount++ % 100000 == 0) {
		RtlSdr_init(&radio, (int)engineGetSampleRate());
		return;
	}

	if (radio.rack_buffer==NULL) {
		return;
	}

	if(buffer.size() < 10 ) { // This seems reasonable
		//printf("📻 ring buffer is getting low (%ld), try mutex\n", buffer.size());
		int error = pthread_mutex_trylock(radio.rack_mutex);
		if (error != 0) {
			if(error==EBUSY) {
				printf("📻 mutex busy\n");
			} else {
				printf("📻 mutex error\n");
			}
		} else {
			if(*(radio.rack_buffer_pos) != 0) {
				for(int i = 0; i < *(radio.rack_buffer_pos); i++) {
					if(buffer.full()) {
						printf("📻 sdr buffer overrun\n");
						break;
					}
					buffer.push(radio.rack_buffer[i]);
				}
				//printf("📻 ring buffer consumed %ld, size is now %d\n", *(radio.rack_buffer_pos), buffer.size());
				*(radio.rack_buffer_pos) = 0;
			}
			pthread_mutex_unlock(radio.rack_mutex);
		}
	}

	float freq = params[TUNE_PARAM].value;
	float freqOff = params[TUNE_ATT].value*inputs[TUNE_INPUT].value/MAX_VOLTAGE;
	float freqComputed = freq + freqOff;
	long longFreq = getFreq(freqComputed) ; // lots of zeros

	enum Quantization {HUNDREDK, TENK, NONE};
  Quantization scale = static_cast<Quantization>(roundf(params[QUANT_PARAM].value));
	long modulo;
	switch(scale) {
		case HUNDREDK:
			modulo = 100000;
			break;
		case TENK:
			modulo = 10000;
			break;
		case NONE:
			modulo = 1;
	}
	long modulus = longFreq%modulo;
	longFreq -= modulus;
	if(modulus>=modulo/2) {
		longFreq+=modulo;
	}


	if (longFreq - currentFreq) {
			RtlSdr_tune(&radio, longFreq);
			currentFreq = longFreq;
			std::stringstream stream;
			stream << std::fixed << std::setprecision(3) << getMegaFreq(longFreq);
			linkedLabel->text = stream.str();
	}

	if(!buffer.empty()) {
		int16_t sample = buffer.shift();
		float value = MAX_VOLTAGE*float(sample)/(float)SHRT_MAX;
		outputs[SDR::AUDIO_OUT].value 	= value;
	} else {
		//printf("📻 awaiting buffer\n");
	}
}

void SDR::onSampleRateChange() {
}

long SDR::getFreq(float knob) {
	return int(knob*1000000.f); // float quantities are in millions so this is a million
}

float SDR::getMegaFreq(long longFreq) {
	return float(longFreq)/ 1000000.f; // float quantities are in millions so this is a million
}

struct SDRWidget : ModuleWidget {
	SDRWidget(SDR *module);
};

SDRWidget::SDRWidget(SDR *module) : ModuleWidget(module) {

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  Panel *panel = new LightPanel();
  panel->box.size = box.size;
  addChild(panel);

	addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		MyLabel* const freqLabel = new MyLabel;
		freqLabel->box.pos = Vec(box.size.x/4,RACK_GRID_WIDTH*3);  // coordinate system is broken FIXME
		freqLabel->text = "0";
		module->linkedLabel = freqLabel;
		addChild(freqLabel);
	}

	{
		MyLabel* const cLabel = new MyLabel(10);
		cLabel->box.pos = Vec(19,5);  // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "rtl-sdr FM";
		addChild(cLabel);
	}

	{
		MyLabel* const cLabel = new MyLabel(14);
		cLabel->box.pos = Vec(18,(RACK_GRID_HEIGHT - RACK_GRID_WIDTH/4)/2); // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "pq";
		addChild(cLabel);
	}

  SVGKnob *knob = dynamic_cast<SVGKnob*>(ParamWidget::create<RoundHugeBlackKnob>(Vec(RACK_GRID_WIDTH/6, 100), module, SDR::TUNE_PARAM, HZ_FLOOR, HZ_CEIL, HZ_CENTER));
	knob->maxAngle += 10*2*M_PI;
	//knob->sensitivity /= 10.f;
	addParam(knob);
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(RACK_GRID_WIDTH, 170), module, SDR::TUNE_ATT, -HZ_SPAN/2.0, +HZ_SPAN/2.0, 0.0));
	addInput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH, 200), Port::INPUT, module, SDR::TUNE_INPUT));
	addParam(ParamWidget::create<CKSSThree>(Vec(RACK_GRID_WIDTH/2, 240), module, SDR::QUANT_PARAM, 0.0, 2.0, 0.0));
	{
		MyLabel* const cLabel = new MyLabel(12);
		cLabel->box.pos = Vec(16,236/2); // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "Stepping";
		addChild(cLabel);
	}
	{
		MyLabel* const cLabel = new MyLabel(12);
		cLabel->box.pos = Vec(20,240/2+4); // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "none ";
		addChild(cLabel);
	}
	{
		MyLabel* const cLabel = new MyLabel(12);
		cLabel->box.pos = Vec(20,240/2+9); // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "10 k ";
		addChild(cLabel);
	}
	{
		MyLabel* const cLabel = new MyLabel(12);
		cLabel->box.pos = Vec(20,240/2+14); // coordinate system is broken FIXME
		cLabel->color = nvgRGB(0,0,0);
		cLabel->text = "100k";
		addChild(cLabel);
	}


	addOutput(Port::create<PJ301MPort>(Vec(RACK_GRID_WIDTH, box.size.y-3*RACK_GRID_WIDTH), Port::OUTPUT, module, SDR::AUDIO_OUT));
}


Model *sdrModule = Model::create<SDR, SDRWidget>(
	"Pulsum Quadratum", "SDRWidget", "rtl-sdr FM Radio Tuner", EXTERNAL_TAG, TUNER_TAG);
