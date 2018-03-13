#include "ML_modules.hpp"

#include "dsp/digital.hpp"

struct TrigBuf : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ARM1_INPUT,
		ARM2_INPUT,
		GATE1_INPUT,
		GATE2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ARM1_LIGHT,
		ARM2_LIGHT,
		NUM_LIGHTS
	};


	TrigBuf() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };


	void step() override;

	float arm1=0.0, arm2=0.0;
	float out1=0.0, out2=0.0;

	bool gate1=false, gate2=false;
	bool delayed1=false, delayed2=false;

	SchmittTrigger armTrigger1, armTrigger2;
	SchmittTrigger gateTrigger1, gateTrigger2;

	void reset() override {

		arm1=0.0;
		arm2=0.0;
		out1=0.0;
		out2=0.0;
		gate1=false;
		gate2=false;
		delayed1=false;
		delayed2=false;
	};

private:

	bool neg_slope(bool gate, bool last_gate) { return (gate!=last_gate) && last_gate; }


};




void TrigBuf::step() {


	bool last_gate1 = gate1;
	bool last_gate2 = gate2;
       

	if( inputs[GATE1_INPUT].active) gateTrigger1.process(inputs[GATE1_INPUT].value);
	gate1 = (gateTrigger1.isHigh());

	if( inputs[GATE2_INPUT].active ) {
		gateTrigger2.process(inputs[GATE2_INPUT].value);
		gate2 = gateTrigger2.isHigh();
	} else {
		gate2 = gate1;
	};


	if( inputs[ARM1_INPUT].active ) {
		if( armTrigger1.process(inputs[ARM1_INPUT].value) ) { 
			if (!gate1) {arm1 = 10.0;}
			else { 
				// arm1 = 0.0;
				delayed1 = true;
			};

	    }
	} else {
		arm1 = 0.0;
	};


    if(gate1) {
		if(arm1 > 5.0) {
			out1 = 10.0;
		} else {
			out1 = 0.0;
		};
	}
	else {
		if(out1 > 5.0) {
			arm1 = 0.0;
			out1 = 0.0;
		};
	};

	if( delayed1 && neg_slope(gate1, last_gate1) ) {

		arm1 = 10.0;
		delayed1 = false;

	};



	if( inputs[ARM2_INPUT].active ) {
		if( armTrigger2.process(inputs[ARM2_INPUT].value) ) { 
			if (!gate2) {arm2 = 10.0;}
			else { 
				// arm2 = 0.0;
				delayed2 = true;
			};
		};
	} else {
		arm2 = arm1;
	};


	if (gate2) {

		if(arm2 > 5.0) out2 = 10.0;
		else {
			out2 = 0.0;
		};
	} else {
		if(out2 > 5.0) {
			arm2 = 0.0;
			out2 = 0.0;
		};
	};

	if( delayed2 && neg_slope(gate2, last_gate2) ) {

		arm2 = 10.0;
		delayed2 = false;

	};


	outputs[OUT1_OUTPUT].value = out1;
	outputs[OUT2_OUTPUT].value = out2;

	lights[ARM1_LIGHT].value = arm1;
	lights[ARM2_LIGHT].value = arm2;
};



struct TrigBufWidget : ModuleWidget {
	TrigBufWidget(TrigBuf *module);
};

TrigBufWidget::TrigBufWidget(TrigBuf *module) : ModuleWidget(module) {

	box.size = Vec(15*4, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigBuf.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

	addInput(Port::create<PJ301MPort>(Vec(10, 62), Port::INPUT, module, TrigBuf::ARM1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 105), Port::INPUT, module, TrigBuf::GATE1_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10, 150), Port::OUTPUT, module, TrigBuf::OUT1_OUTPUT));

	addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(46, 66), module, TrigBuf::ARM1_LIGHT));

	addInput(Port::create<PJ301MPort>(Vec(10, 218), Port::INPUT, module, TrigBuf::ARM2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(10, 263), Port::INPUT, module, TrigBuf::GATE2_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(10, 305), Port::OUTPUT, module, TrigBuf::OUT2_OUTPUT));
	addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(46, 222), module, TrigBuf::ARM2_LIGHT));
}

Model *modelTrigBuf = Model::create<TrigBuf, TrigBufWidget>("ML modules", "TrigBuf", "Trigger Buffer", UTILITY_TAG);
