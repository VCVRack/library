#include "MrLumps.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "MrLumps";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/djpeterso23662/MrLumps";
	p->manual = "https://github.com/djpeterso23662/MrLumps/blob/master/README.md";
	p->addModel(createModel<SEQEuclidWidget>(plugin->slug, "SEQE", "SEQ-Euclid",SEQUENCER_TAG));
	p->addModel(createModel<VCS1x8Widget>(plugin->slug, "VCS1", "1x8 Voltage Controlled Switch",SWITCH_TAG));
	p->addModel(createModel<VCS2x4Widget>(plugin->slug, "VCS2", "2x4 Voltage Controlled Switch",SWITCH_TAG));
}
