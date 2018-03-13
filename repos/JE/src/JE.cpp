#include "JE.hpp"

rack::Plugin* plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	plugin->slug = "JE";
	plugin->version = TOSTRING(JE_VERSION);
	plugin->website = "https://github.com/eres-j/VCVRack-plugin-JE";
	plugin->manual = "https://github.com/eres-j/VCVRack-plugin-JE";

	p->addModel(rack::createModel<RingModulatorWidget>(
		TOSTRING(JE_MANUFACTURER),
		"RingModulator", "Ring Modulator",
		rack::EFFECT_TAG, rack::RING_MODULATOR_TAG
	));

	p->addModel(rack::createModel<WaveFolderWidget>(
		TOSTRING(JE_MANUFACTURER),
		"SimpleWaveFolder", "Simple Wave Folder",
		rack::EFFECT_TAG, rack::WAVESHAPER_TAG
	));
}
