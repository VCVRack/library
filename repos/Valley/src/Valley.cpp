#include "Valley.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/ValleyAudio/ValleyRackFree";
	p->manual = "https://github.com/ValleyAudio/ValleyRackFree/blob/master/README.md";
    p->addModel(modelTopograph);
}
