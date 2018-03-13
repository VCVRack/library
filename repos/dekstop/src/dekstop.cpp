#include "dekstop.hpp"
#include <math.h>

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = "dekstop";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/dekstop/vcvrackplugins_dekstop";

	p->addModel(createModel<TriSEQ3Widget>("dekstop", "TriSEQ3", "Tri-state SEQ-3", SEQUENCER_TAG));
	p->addModel(createModel<GateSEQ8Widget>("dekstop", "GateSEQ8", "Gate SEQ-8", SEQUENCER_TAG));
	p->addModel(createModel<Recorder2Widget>("dekstop", "Recorder2", "Recorder 2", UTILITY_TAG));
	p->addModel(createModel<Recorder8Widget>("dekstop", "Recorder8", "Recorder 8", UTILITY_TAG));
}
