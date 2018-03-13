#include "av500.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "av500";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->addModel(createModel<MultipleWidget>    ("av500", "Multiple", "Multiple", MULTIPLE_TAG));
	p->addModel(createModel<Blank8hpWidget>    ("av500", "Blank", "8hp Blank", BLANK_TAG));
	p->addModel(createModel<p0wrWidget>        ("av500", "Power", "p0wr", VISUAL_TAG));
	p->addModel(createModel<TR808CowbellWidget>("av500", "Drum", "808Cowbell", DRUM_TAG));
}

//plugin->homepageUrl = "https://github.com/av500/vcvrackplugins_av500";
