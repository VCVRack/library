////////////////////////////////////////////////////////////////////////////////////////////////////
////// Ohmer Modules ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

// The plugin-wide instance of the Plugin class.
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/DomiKamu/Ohmer-Modules/";
	p->manual = "https://github.com/DomiKamu/Ohmer-Modules/blob/master/README.md";
	p->addModel(modelKlokSpid);
	p->addModel(modelSplitter1x9);
	p->addModel(modelBlankPanel1);
	p->addModel(modelBlankPanel2);
	p->addModel(modelBlankPanel4);
	p->addModel(modelBlankPanel8);
	p->addModel(modelBlankPanel16);
	p->addModel(modelBlankPanel32);
}
