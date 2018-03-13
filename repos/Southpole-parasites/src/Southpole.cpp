

#include "Southpole.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
	// It must only contain letters, numbers, and characters "-" and "_". No spaces.
	// To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
	// The ZIP package must only contain one folder, with the name equal to the plugin's slug.
#ifdef PARASITES
	p->slug = "Southpole-parasites";    
#else
	p->slug = "Southpole";
#endif
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/gbrandt1/southpole-vcvrack";
	p->manual = "https://github.com/gbrandt1/southpole-vcvrack/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	//p->addModel(createModel<MyModuleWidget>("Southpole", "MyModule", "My Module", //OSCILLATOR_TAG));

#ifdef PARASITES
    p->addModel(createModel<SmokeWidget>(	"Southpole Parasites", "Smoke", "Smoke Parasites", GRANULAR_TAG, REVERB_TAG));
    p->addModel(createModel<SplashParasiteWidget>("Southpole Parasites", "SplashParasites", "Splash Parasites", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG));
#else
    p->addModel(createModel<SmokeWidget>( 	 "Southpole", "Smoke", "Smoke - texture synth", GRANULAR_TAG, REVERB_TAG));
    p->addModel(createModel<AnnuliWidget>( 	 "Southpole", "Annuli", "Annuli - resonator"));
    p->addModel(createModel<BalaclavaWidget>("Southpole", "Balaclava", "Balaclava - quad VCA", AMPLIFIER_TAG, MIXER_TAG, ATTENUATOR_TAG));
    p->addModel(createModel<BandanaWidget>(	 "Southpole", "Bandana", "Bandana - quad polarizer", AMPLIFIER_TAG, MIXER_TAG, ATTENUATOR_TAG));

    p->addModel(createModel<FallsWidget>(	"Southpole", "Falls", "Falls - attenumixverter", UTILITY_TAG,AMPLIFIER_TAG, MIXER_TAG, ATTENUATOR_TAG));
    p->addModel(createModel<AuxWidget>(		"Southpole", "Aux", "Aux - effect loop", UTILITY_TAG,AMPLIFIER_TAG, MIXER_TAG));
    p->addModel(createModel<ButWidget>(		"Southpole", "But", "But - A/B buss", SWITCH_TAG, UTILITY_TAG, MIXER_TAG));
    p->addModel(createModel<AbrWidget>(		"Southpole", "Abr", "Abr - A/B switch", SWITCH_TAG, UTILITY_TAG, MIXER_TAG));
    p->addModel(createModel<EtagereWidget>(	"Southpole", "Etagere", "Etagère - EQ", FILTER_TAG));
    p->addModel(createModel<DeuxEtageresWidget>("Southpole", "DeuxEtageres", "Deux Etagères - Stereo EQ", FILTER_TAG));
    p->addModel(createModel<RiemannWidget>(	"Southpole", "Riemann", "Riemann - chord generator", SEQUENCER_TAG));
    p->addModel(createModel<SnsWidget>(		"Southpole", "SNS", "SNS - euclidean sequencer", SEQUENCER_TAG));
    p->addModel(createModel<PisteWidget>(	"Southpole", "Piste", "Piste - drum processor", ENVELOPE_GENERATOR_TAG, EFFECT_TAG, UTILITY_TAG));
    p->addModel(createModel<WriggleWidget>(	"Southpole", "Wriggle", "Wriggle - spring model", LFO_TAG, FUNCTION_GENERATOR_TAG));
    p->addModel(createModel<FuseWidget>(	"Southpole", "Fuse", "Fuse - next pattern", SEQUENCER_TAG));
    p->addModel(createModel<CornrowsWidget>("Southpole", "Cornrows", "Cornrows - macro oscillator", OSCILLATOR_TAG, WAVESHAPER_TAG));
	p->addModel(createModel<SplashWidget>(	"Southpole", "Splash", "Splash / Lambs - tidal modulator", LFO_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG, FUNCTION_GENERATOR_TAG));
	p->addModel(createModel<SsshWidget>(	"Southpole", "Sssh", "Sssh - noise and S+H", UTILITY_TAG));
	p->addModel(createModel<SnakeWidget>(	"Southpole", "Snake", "Snake - multicore", UTILITY_TAG));
	p->addModel(createModel<GnomeWidget>(	"Southpole", "Gnome", "Gnome - synth voice",  LFO_TAG, OSCILLATOR_TAG,AMPLIFIER_TAG, MIXER_TAG));

	p->addModel(createModel<Blank1HPWidget>("Southpole", "Blank1HP", "Blank 1 HP", UTILITY_TAG));
	p->addModel(createModel<Blank2HPWidget>("Southpole", "Blank2HP", "Blank 2 HP", UTILITY_TAG));
	p->addModel(createModel<Blank4HPWidget>("Southpole", "Blank4HP", "Blank 4 HP", UTILITY_TAG));
	p->addModel(createModel<Blank8HPWidget>("Southpole", "Blank8HP", "Blank 8 HP", UTILITY_TAG));
	p->addModel(createModel<Blank16HPWidget>("Southpole", "Blank16HP", "Blank 16 HP", UTILITY_TAG));
	p->addModel(createModel<Blank42HPWidget>("Southpole", "Blank42HP", "Blank 42 HP", UTILITY_TAG));
    
#endif
}


