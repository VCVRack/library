#include "Bidoo.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	p->addModel(modelDTROY);
	p->addModel(modelBORDL);
	p->addModel(modelTOCANTE);
	p->addModel(modelCHUTE);
	p->addModel(modelLATE);
	p->addModel(modelACNE);
	p->addModel(modelOUAIVE);
	p->addModel(modelPANARD);
	p->addModel(modelDUKE);
	p->addModel(modelMOIRE);
	p->addModel(modelFORK);
	p->addModel(modelTIARE);
	p->addModel(modelCLACOS);
	p->addModel(modelANTN);
	p->addModel(modelLIMBO);
	p->addModel(modelPERCO);
	p->addModel(modelBAR);
	p->addModel(modelZINC);
	p->addModel(modelVOID);
}
