//**************************************************************************************
//1x8 Multimple Module for VCV Rack by Autodafe http://www.autodafe.net
//
//**************************************************************************************

#include "Autodafe.hpp"

struct Multiple18 : Module{
	enum ParamIds {	
		NUM_PARAMS
	};
	enum InputIds {
		INPUT1,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT11,
		OUT12,
		OUT13,
		OUT14,
		OUT15,
		OUT16,
		OUT17,
		OUT18,
		NUM_OUTPUTS
	};

	
	Multiple18();
	void step();
};


Multiple18::Multiple18() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
}


void Multiple18::step() {
	
	float IN1 = inputs[INPUT1].value;
	

	// Set outputs
	if (outputs[OUT11].active) {
		outputs[OUT11].value = IN1;
	}
	
	if (outputs[OUT12].active) {
		outputs[OUT12].value = IN1;
	}

	if (outputs[OUT13].active) {
		outputs[OUT13].value= IN1;
	}

	if (outputs[OUT14].active) {
		outputs[OUT14].value = IN1;
	}

	if (outputs[OUT15].active) {
		outputs[OUT15].value = IN1;
	}

	if (outputs[OUT16].active) {
		outputs[OUT16].value = IN1;
	}

	if (outputs[OUT17].active) {
		outputs[OUT17].value = IN1;
	}

	if (outputs[OUT18].active) {
		outputs[OUT18].value = IN1;
	}


	
}

Multiple18Widget::Multiple18Widget() {
	Multiple18 *module = new Multiple18();
	setModule(module);
	box.size = Vec(15*3, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Multiple18.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(1, 0)));
	addChild(createScrew<ScrewSilver>(Vec(1, 365)));
	
	addInput(createInput<PJ3410Port>(Vec(10, 20), module, Multiple18::INPUT1));
	
	addOutput(createOutput<PJ3410Port>(Vec(10, 60), module, Multiple18::OUT11));
	addOutput(createOutput<PJ3410Port>(Vec(10, 95), module, Multiple18::OUT12));
	addOutput(createOutput<PJ3410Port>(Vec(10, 130), module, Multiple18::OUT13));
	addOutput(createOutput<PJ3410Port>(Vec(10, 165), module, Multiple18::OUT14));
	addOutput(createOutput<PJ3410Port>(Vec(10, 200), module, Multiple18::OUT15));
	addOutput(createOutput<PJ3410Port>(Vec(10, 235), module, Multiple18::OUT16));
	addOutput(createOutput<PJ3410Port>(Vec(10, 270), module, Multiple18::OUT17));
	addOutput(createOutput<PJ3410Port>(Vec(10, 305), module, Multiple18::OUT18));

} 
