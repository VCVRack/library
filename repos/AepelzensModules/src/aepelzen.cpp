#include "aepelzen.hpp"
#include <math.h>

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	p->addModel(modelQuadSeq);
	p->addModel(modelGateSeq);
	p->addModel(modelDice);
	p->addModel(modelBurst);
	p->addModel(modelFolder);
	p->addModel(modelWalker);
	p->addModel(modelErwin);
	p->addModel(modelWerner);
}
