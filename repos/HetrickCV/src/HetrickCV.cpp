#include "HetrickCV.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	plugin->version = TOSTRING(VERSION);

	p->website = "https://github.com/mhetrick/hetrickcv";
 	p->manual = "https://github.com/mhetrick/hetrickcv/blob/master/README.md";

	p->addModel(modelTwoToFour);
	p->addModel(modelAnalogToDigital);
	p->addModel(modelASR);
	p->addModel(modelBitshift);
	p->addModel(modelBlankPanel);
	p->addModel(modelBoolean3);
	p->addModel(modelComparator);
	p->addModel(modelContrast);
	p->addModel(modelCrackle);
	p->addModel(modelDelta);
	p->addModel(modelDigitalToAnalog);
	p->addModel(modelDust);
	p->addModel(modelExponent);
	p->addModel(modelFlipFlop);
	p->addModel(modelFlipPan);
	p->addModel(modelGateJunction);
	p->addModel(modelLogicCombine);
	p->addModel(modelRandomGates);
	p->addModel(modelRotator);
	p->addModel(modelScanner);
	p->addModel(modelWaveshape);
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.
}
