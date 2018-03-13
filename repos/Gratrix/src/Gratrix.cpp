#include "Gratrix.hpp"


Plugin *plugin;

void init(rack::Plugin *p)
{
	plugin     = p;
	p->slug    = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

//	p->addModel(GTX::MIDI_C1  ::model);
//	p->addModel(GTX::MIDI_G1  ::model);
	p->addModel(GTX::VCO_F1   ::model);
	p->addModel(GTX::VCO_F2   ::model);
	p->addModel(GTX::VCF_F1   ::model);
	p->addModel(GTX::VCA_F1   ::model);
	p->addModel(GTX::ADSR_F1  ::model);
	p->addModel(GTX::Chord_G1 ::model);
	p->addModel(GTX::Octave_G1::model);
	p->addModel(GTX::Fade_G1  ::model);
	p->addModel(GTX::Fade_G2  ::model);
	p->addModel(GTX::Binary_G1::model);
	p->addModel(GTX::Seq_G1   ::model);
	p->addModel(GTX::Seq_G2   ::model);
	p->addModel(GTX::Keys_G1  ::model);
	p->addModel(GTX::VU_G1    ::model);
	p->addModel(GTX::Scope_G1 ::model);
	p->addModel(GTX::Blank_03 ::model);
	p->addModel(GTX::Blank_06 ::model);
	p->addModel(GTX::Blank_09 ::model);
	p->addModel(GTX::Blank_12 ::model);
}

/*
	p->addModel(createModel<GTX::MIDI_C1::Widget>("Gratrix", "MIDI-C1", "MIDI-C1", MIDI_TAG, EXTERNAL_TAG));
	p->addModel(createModel<GTX::MIDI_G1::Widget>("Gratrix", "MIDI-G1", "MIDI-G1", MIDI_TAG, EXTERNAL_TAG));
*/
