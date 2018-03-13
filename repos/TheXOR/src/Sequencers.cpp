#include "common.hpp"
#include "Klee.hpp"
#include "M581.hpp"
#include "Z8K.hpp"
#include "Renato.hpp"
#include "Spiralone.hpp"
#include "pwmClock.hpp"
#include "quantizer.hpp"

#ifdef LPTEST_MODULE
#include "lpTestModule.hpp"
#endif 

#ifdef OSCTEST_MODULE
#include "oscTestModule.hpp"
#endif 

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "TheXOR";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/The-XOR/VCV-Sequencers";
	p->manual = "https://github.com/The-XOR/VCV-Sequencers/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(Model::create<Klee, KleeWidget>("TheXOR", "Klee", "Klee Sequencer", SEQUENCER_TAG));
	p->addModel(Model::create<M581, M581Widget>("TheXOR", "M581", "581 Sequencer", SEQUENCER_TAG));
	p->addModel(Model::create<Z8K, Z8KWidget>("TheXOR", "Z8K", "Z8K Sequencer", SEQUENCER_TAG));
	p->addModel(Model::create<Renato, RenatoWidget>("TheXOR", "Renato", "Renato Sequencer", SEQUENCER_TAG));
	p->addModel(Model::create<Spiralone, SpiraloneWidget>("TheXOR", "Spiralone", "Spiralone Sequencer", SEQUENCER_TAG));
	p->addModel(Model::create<PwmClock, PwmClockWidget>("TheXOR", "PWMClock", "PWM Clock", UTILITY_TAG, CLOCK_TAG));
	p->addModel(Model::create<Quantizer, QuantizerWidget>("TheXOR", "Quantizer", "Quantizer", UTILITY_TAG, QUANTIZER_TAG));

#ifdef LPTEST_MODULE
	p->addModel(Model::create<LaunchpadTest, LaunchpadTestWidget>("TheXOR", "LaunchpadTest", "Launchpad Test", DIGITAL_TAG));
#endif

#ifdef OSCTEST_MODULE
	p->addModel(Model::create<OscTest, OscTestWidget>("TheXOR", "OSCTest", "OSC Test", DIGITAL_TAG));
#endif

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
