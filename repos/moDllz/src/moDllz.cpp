#include "moDllz.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    // This is the unique identifier for your plugin
	p->slug = TOSTRING(SLUG);
//#ifdef VERSION
	p->version = TOSTRING(VERSION);
//#endif

    
    p->addModel(modelMIDIPoly);
    p->addModel(modelTwinGlider);
    p->addModel(modelMIDIdualCV);
    // For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
//	p->addModel(createModel<MIDIdualCVWidget>  ("moDllz", "MIDIdualCV",   "MIDI to dual CV interface", MIDI_TAG, DUAL_TAG, UTILITY_TAG));
//    p->addModel(createModel<MIDIPolyWidget>  ("moDllz", "MIDI-Poly",   "MIDI Poly16 Seq Arpeggiator", MIDI_TAG, UTILITY_TAG, SEQUENCER_TAG));
//    p->addModel(createModel<MIDIMPE8Widget>  ("moDllz", "MIDI_MPE8",   "MIDI-MPE 8ch interface", MIDI_TAG, UTILITY_TAG));
 //   p->addModel(createModel<TwinGliderWidget>  ("moDllz", "TwinGlider",   "TwinGlider Dual Portamento", SLEW_LIMITER_TAG,  DUAL_TAG, ENVELOPE_FOLLOWER_TAG, UTILITY_TAG));
}
