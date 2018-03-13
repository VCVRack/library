#include "LOGinstruments.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/leopard86/LOGinstruments";
	p->manual = "https://github.com/leopard86/LOGinstruments/blob/master/README.md";

	p->addModel(modelconstant);
	p->addModel(modelconstant2);
	p->addModel(modelSpeck);
	p->addModel(modelBritix);
	p->addModel(modelCompa);
	p->addModel(modelLessMess);
	p->addModel(modelVelvet);
	p->addModel(modelCrystal);
}
