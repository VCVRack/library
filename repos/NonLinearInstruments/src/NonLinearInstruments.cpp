#include "NonLinearInstruments.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "NonLinearInstruments";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins";
	p->manual = "https://github.com/NonLinearInstruments/NLNRI_VCVRackPlugins/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(createModel<QU4DiTWidget>("NonLinearInstruments", "QUADiT", "Quadratic Iterator", OSCILLATOR_TAG));
	p->addModel(createModel<BallisticENVWidget>("NonLinearInstruments", "BallisticENV", "Ballistic ENV", ENVELOPE_GENERATOR_TAG));
	p->addModel(createModel<LuciCellWidget>("NonLinearInstruments", "LuciCell", "Luci Cell", OSCILLATOR_TAG));
	p->addModel(createModel<Luci4AudioSumWidget>("NonLinearInstruments", "Luci4AudioSum", "Luci 4 Audio Sum", MIXER_TAG));
	p->addModel(createModel<Luci4ParamDistrWidget>("NonLinearInstruments", "Luci4ParamDistr", "Luci 4 Param Distr", MULTIPLE_TAG));		
	p->addModel(createModel<LuciControlRNDWidget>("NonLinearInstruments", "LuciControlRND", "Luci Ctrl RAND", CONTROLLER_TAG));		
	p->addModel(createModel<LuciControlFREQWidget>("NonLinearInstruments", "LuciControlFREQ", "Luci Ctrl FREQ", CONTROLLER_TAG));	
	p->addModel(createModel<LuciControlINFcoarseWidget>("NonLinearInstruments", "LuciControlINFL", "Luci Ctrl INFLUENCE", CONTROLLER_TAG));
	//p->addModel(createModel<LuciControlINFfineWidget>("NonLinearInstruments", "LuciContrINFLfine", "Luci Cont.Infl.Fine", CONTROLLER_TAG));
			
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

}
