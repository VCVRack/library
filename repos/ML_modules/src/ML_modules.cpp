#include "ML_modules.hpp"
#include <math.h>

Plugin *plugin;



void init(rack::Plugin *p) {
 	plugin = p;
 	plugin->slug = TOSTRING(SLUG);
	plugin->version = TOSTRING(VERSION);


 	plugin->addModel(modelQuantizer);
 	plugin->addModel(modelQuantum);
 	plugin->addModel(modelTrigBuf);
	plugin->addModel(modelSeqSwitch);
	plugin->addModel(modelSeqSwitch2);
	plugin->addModel(modelShiftRegister);
	plugin->addModel(modelShiftRegister2);
	plugin->addModel(modelFreeVerb);
	plugin->addModel(modelSum8);
	plugin->addModel(modelSum8mk2);
	plugin->addModel(modelSH8);
	plugin->addModel(modelConstants);
	plugin->addModel(modelCounter);
	plugin->addModel(modelTrigDelay);
	plugin->addModel(modelBPMdetect);
	plugin->addModel(modelVoltMeter);
	plugin->addModel(modelOctaFlop);
	plugin->addModel(modelOctaTrig);
	plugin->addModel(modelOctaSwitch);
	plugin->addModel(modelTrigSwitch);
 	plugin->addModel(modelTrigSwitch2);
}






