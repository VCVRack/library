#include "AudibleInstruments.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
#ifdef PARASITES
	plugin->slug = "ParableInstruments";
	//plugin->name = "Parable Instruments";
	//plugin->homepageUrl = "https://github.com/adbrant/ArableInstruments";

	//createModel<CloudsWidget>(plugin, "Neil", "Neil - Texture Synthesizer");
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->addModel(createModel<CloudsWidget>("Arable Instruments", "Neil", "Neil - Texture Synthesizer",GRANULAR_TAG, REVERB_TAG));
#else
	p->slug = "ArableInstruments";
	//plugin->name = "Arable Instruments";
	//plugin->homepageUrl = "https://github.com/adbrant/ArableInstruments";

#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->addModel(createModel<CloudsWidget>("Arable Instruments", "Joni", "Joni - Texture Synthesizer",GRANULAR_TAG, REVERB_TAG));
#endif
}
