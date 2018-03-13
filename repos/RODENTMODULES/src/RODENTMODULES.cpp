#include "RODENTMODULES.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "RODENTMODULES";
#ifdef VERSION
	p->version = TOSTRING(heck);
#endif
	p->website = "https://rodent-cat.bandcamp.com/";
	p->manual = "NONE YET";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(createModel<MessedUpOscWidget>("RODENTMODULES", "MessedUpOsc", "Messed Up Oscillator", OSCILLATOR_TAG));

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
