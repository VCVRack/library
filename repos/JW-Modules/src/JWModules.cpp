#include "JWModules.hpp"

Plugin *plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	p->slug = "JW-Modules";
	p->version = TOSTRING(VERSION);
	p->addModel(modelCat);
	p->addModel(modelBouncyBalls);
	p->addModel(modelFullScope);
	p->addModel(modelGridSeq);
	p->addModel(modelQuantizer);
	p->addModel(modelMinMax);
	p->addModel(modelNoteSeq);
	p->addModel(modelSimpleClock);
	p->addModel(modelThingThing);
	p->addModel(modelWavHead);
	p->addModel(modelXYPad);
}
