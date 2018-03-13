#include "ML_modules.hpp"

#include "dsp/digital.hpp"


#define minLength 0.001

struct TrigDelay : Module {
	enum ParamIds {
		DELAY1_PARAM,
		DELAY2_PARAM,
		LENGTH1_PARAM,
		LENGTH2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE1_INPUT,
		GATE2_INPUT,
		DELAY1_INPUT,
		DELAY2_INPUT,
		LENGTH1_INPUT,
		LENGTH2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	TrigDelay() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {

		gSampleRate = engineGetSampleRate();
		// minLength = 0.001;

	};


	void step() override;

	bool gate1=false, gate2=false;

	SchmittTrigger gateTrigger1, gateTrigger2;
	PulseGenerator delay1,  delay2;
	PulseGenerator on1, on2;



	float gSampleRate;

	void onSampleRateChange() { gSampleRate = engineGetSampleRate(); }

	void reset() override {

		gate1=false;
		gate2=false;
	};

private:



};




void TrigDelay::step() {


	float delayTime1 = params[DELAY1_PARAM].value;
	float delayTime2 = params[DELAY2_PARAM].value;
	float length1    = params[LENGTH1_PARAM].value;
	float length2    = params[LENGTH2_PARAM].value;


	if( inputs[DELAY1_INPUT].active )  { delayTime1 *= clampf( inputs[DELAY1_INPUT].value / 10.0, 0.0, 1.0 );};
	if( inputs[DELAY2_INPUT].active )  { delayTime2 *= clampf( inputs[DELAY2_INPUT].value / 10.0, 0.0, 1.0 );};

	if( inputs[LENGTH1_INPUT].active ) { length1    *= clampf( inputs[LENGTH1_INPUT].value / 10.0, minLength, 1.0 );};
	if( inputs[LENGTH2_INPUT].active ) { length2    *= clampf( inputs[LENGTH2_INPUT].value / 10.0, minLength, 1.0 );};




	if( inputs[GATE1_INPUT].active ) {
	       
		if(gateTrigger1.process(inputs[GATE1_INPUT].value)) {
			delay1.trigger(delayTime1);
			gate1 = true;
		};

	};

	if( inputs[GATE2_INPUT].active ) {
	       
		if(gateTrigger2.process(inputs[GATE2_INPUT].value)) {
			delay2.trigger(delayTime2);
			gate2 = true;
		};

	};


	if(  gate1 && !delay1.process(1.0/gSampleRate) ) {
			
		on1.trigger(length1);
		gate1 = false;

	};

	if(  gate2 && !delay2.process(1.0/gSampleRate) ) {
			
		on2.trigger(length2);
		gate2 = false;

	};

	outputs[OUT1_OUTPUT].value = on1.process(1.0/gSampleRate) ? 10.0 : 0.0;
	outputs[OUT2_OUTPUT].value = on2.process(1.0/gSampleRate) ? 10.0 : 0.0;

};



struct TrigDelayWidget : ModuleWidget {
	TrigDelayWidget(TrigDelay *module);
};

TrigDelayWidget::TrigDelayWidget(TrigDelay *module) : ModuleWidget(module) {

	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigDelay.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    addParam(ParamWidget::create<SmallMLKnob>(Vec(12,  69), module, TrigDelay::DELAY1_PARAM, 0.0, 2.0, 0.0));
	addInput(Port::create<PJ301MPort>(Vec(53, 71), Port::INPUT, module, TrigDelay::DELAY1_INPUT));

    addParam(ParamWidget::create<SmallMLKnob>(Vec(12,  112), module, TrigDelay::LENGTH1_PARAM, minLength, 2.0, 0.1));
	addInput(Port::create<PJ301MPort>(Vec(53, 114), Port::INPUT, module, TrigDelay::LENGTH1_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(13, 165), Port::INPUT, module, TrigDelay::GATE1_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(53, 165), Port::OUTPUT, module, TrigDelay::OUT1_OUTPUT));
//	addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(46, 66), &module->arm1));

    addParam(ParamWidget::create<SmallMLKnob>(Vec(12,  153 + 69),  module, TrigDelay::DELAY2_PARAM, 0.0, 2.0, 0.0));
	addInput(Port::create<PJ301MPort>(Vec(53, 153 + 71), Port::INPUT, module, TrigDelay::DELAY2_INPUT));

    addParam(ParamWidget::create<SmallMLKnob>(Vec(12,  153 + 112), module, TrigDelay::LENGTH2_PARAM, minLength, 2.0, 0.1));
	addInput(Port::create<PJ301MPort>(Vec(53, 153 + 114), Port::INPUT, module, TrigDelay::LENGTH2_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(13, 153 + 165), Port::INPUT, module, TrigDelay::GATE2_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(53, 153 + 165), Port::OUTPUT, module, TrigDelay::OUT2_OUTPUT));
//	addChild(createValueLight<SmallLight<GreenValueLight>>(Vec(46, 66), &module->arm1));

}

Model *modelTrigDelay = Model::create<TrigDelay, TrigDelayWidget>("ML modules", "TrigDelay", "Trigger Delay", UTILITY_TAG, DELAY_TAG);
