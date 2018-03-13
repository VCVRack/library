#include "trowaSoft.hpp"
#include "Widget_multiScope.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"
//#include "Widget_trowaTrack.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TROWA_PLUGIN_NAME;
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	// Sequencer Modules:
	// Add EXTERNAL_TAG for osc
	// [03/08/2018] Create model objects in module cpp files per forum topic.
	p->addModel(modelTrigSeq);
	p->addModel(modelTrigSeq64);
	p->addModel(modelVoltSeq);
	//p->addModel(Model::create<trigSeq, trigSeqWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "trigSeq", /*name*/ "trigSeq", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG));
	//p->addModel(Model::create<trigSeq64, trigSeq64Widget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "trigSeq64", /*name*/ "trigSeq64", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG));
	//p->addModel(Model::create<voltSeq, voltSeqWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "voltSeq", /*name*/ "voltSeq", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG));

	// Tracker:
	//p->addModel(Model::create<trowaTrack, trowaTrackWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "trowaTrack", /*name*/ "trowaTrack", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG));

	// Scope Modules:
	//p->addModel(Model::create<multiScope, multiScopeWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "multiScope", /*name*/ "multiScope", /*Tags*/ VISUAL_TAG, EFFECT_TAG, UTILITY_TAG));
	p->addModel(modelMultiScope);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
