#include "PQ.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "PulsumQuadratum-SDR";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/WIZARDISHUNGRY/vcvrack-plugins";
	p->manual = "https://github.com/WIZARDISHUNGRY/vcvrack-plugins/README.md";

	p->addModel(sdrModule);

}
