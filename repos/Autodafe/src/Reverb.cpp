//**************************************************************************************
//Reverb Module for VCV Rack by Autodafe http://www.autodafe.net
//
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//And part of code on musicdsp.org: http://musicdsp.org/showArchiveComment.php?ArchiveID=78
//**************************************************************************************


#include "Autodafe.hpp"
#include <stdlib.h>



#include "stk/include/NRev.h"


using namespace stk;


struct ReverbFx : Module{
	enum ParamIds {
		PARAM_TIME,
		PARAM_DRY_WET,
		
			NUM_PARAMS
	};
	enum InputIds {
		
		INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		
		NUM_OUTPUTS
	};


	ReverbFx();

	  

	//Reverb *cho; 

	NRev *reverb = new NRev(); // OK

	



	void step();
};





ReverbFx::ReverbFx() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);




}










void ReverbFx::step() {

	
	StkFloat  time = params[PARAM_TIME].value;
	
	

	StkFloat  input = inputs[INPUT].value / 5.0;


		reverb->setT60(time);
	
	
	reverb->tick(input, 0);
	//reverb->tick(input, 1);



	outputs[OUT].value= (input + reverb->lastOut(0)*params[PARAM_DRY_WET].value)* 5;
	//outputs[OUTR].value= (input +reverb->lastOut(1) *params[PARAM_DRY_WET].value)* 5;
	


}

ReverbFxWidget::ReverbFxWidget() {
	ReverbFx *module = new ReverbFx();
	setModule(module);
	box.size = Vec(15 * 6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Reverb.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(1, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
	addChild(createScrew<ScrewSilver>(Vec(1, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

		
	addParam(createParam<AutodafeKnobGreenBig>(Vec(20, 60), module, ReverbFx::PARAM_TIME, 0.01, 10, 0.01));

	addParam(createParam<AutodafeKnobGreen>(Vec(27, 140), module, ReverbFx::PARAM_DRY_WET, 0, 1, 0));

	

	addInput(createInput<PJ301MPort>(Vec(10, 320), module, ReverbFx::INPUT));
	addOutput(createOutput<PJ301MPort>(Vec(48, 320), module, ReverbFx::OUT));
	//addOutput(createOutput<PJ301MPort>(Vec(78, 320), module, ReverbFx::OUTR));
	
}
