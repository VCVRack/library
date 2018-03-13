#include "AudibleInstruments.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = "ParableInstruments";
	plugin->name = "Parable Instruments";
	plugin->homepageUrl = "https://github.com/adbrant/ArableInstruments";

	createModel<CloudsWidget>(plugin, "Neil", "Neil - Texture Synthesizer");
}
