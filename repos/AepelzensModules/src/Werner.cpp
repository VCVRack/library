#include "aepelzen.hpp"
#include "dsp/digital.hpp"

#define NUM_CHANNELS 4

struct Werner : Module {
    enum ParamIds {
	TIME_PARAM,
	DELTA_PARAM,
	NUM_PARAMS
    };
    enum InputIds {
	CV_INPUT,
	NUM_INPUTS = NUM_CHANNELS
    };
    enum OutputIds {
	GATE_OUTPUT,
	NUM_OUTPUTS = NUM_CHANNELS
    };
    enum LightIds {
	NUM_LIGHTS
    };

    Werner() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    PulseGenerator gatePulse[NUM_CHANNELS];
    float lastValue[NUM_CHANNELS];
    int res = 16;
    float minDelta = 0;
    int frame = 0;
};


void Werner::step() {
    //max time is about 100ms at 44kHz
    res = (int)clamp(params[TIME_PARAM].value * 4400.0f, 16.0f, 4400.0f);
    minDelta = params[DELTA_PARAM].value * 2.0f;

    if(++frame > res) {
	for(int i=0; i<NUM_CHANNELS;i++) {
	    float value = inputs[CV_INPUT + i].value;

	    if(abs(value - lastValue[i]) > minDelta) {
		gatePulse[i].trigger(0.01);
	    }

	    lastValue[i] = value;
	}
	frame = 0;
    }

    for(int i=0; i<NUM_CHANNELS;i++) {
	outputs[GATE_OUTPUT + i].value = gatePulse[i].process(1.0 / engineGetSampleRate()) ? 10.0 : 0.0;
    }
}


struct WernerWidget : ModuleWidget
{
    WernerWidget(Werner *module);
};

WernerWidget::WernerWidget(Werner *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Werner.svg")));

    addParam(ParamWidget::create<Trimpot>(Vec(6, 30), module, Werner::TIME_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<Trimpot>(Vec(6, 75), module, Werner::DELTA_PARAM, 0.0, 1.0, 0.0));

    for(int i=0;i<NUM_CHANNELS;i++) {
	addInput(Port::create<PJ301MPort>(Vec(3, 125 + i*30), Port::INPUT, module, Werner::CV_INPUT + i));
	addOutput(Port::create<PJ301MPort>(Vec(3, 250 + i*30), Port::OUTPUT, module, Werner::GATE_OUTPUT + i));
    }
}

Model *modelWerner = Model::create<Werner, WernerWidget>("Aepelzens Modules", "Werner", "CV-to-Trigger", UTILITY_TAG);
