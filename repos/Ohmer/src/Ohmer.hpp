#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

extern Model *modelKlokSpid;
extern Model *modelSplitter1x9;
extern Model *modelBlankPanel1;
extern Model *modelBlankPanel2;
extern Model *modelBlankPanel4;
extern Model *modelBlankPanel8;
extern Model *modelBlankPanel16;
extern Model *modelBlankPanel32;

//// CUSTOM COMPONENTS (SCREWS, PORTS/JACKS, KNOB, BUTTONS, LEDS).

// Custom silver Torx screw.
struct Torx_Silver : SVGScrew {
	Torx_Silver() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/Torx_Silver.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

// Custom gols Torx screw.
struct Torx_Gold : SVGScrew {
	Torx_Gold() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/Torx_Gold.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

// Silver momentary button (used by standard-line KlokSpid modules).
// This button is used for:
// - BPM start/stop toggle (KlokSpid module acting as standalone BPM clock generator).
// - entering Setup (by holding this button).
// - advance to next Setup parameter (and exit Setup).
struct KS_ButtonSilver : SVGSwitch, MomentarySwitch {
	KS_ButtonSilver() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Up_Silver.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Down_Silver.svg")));
	}
};

// Gold momentary button (used by Signature-line KlokSpid modules).
struct KS_ButtonGold : SVGSwitch, MomentarySwitch {
	KS_ButtonGold() {
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Up_Gold.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/components/KS_Button_Down_Gold.svg")));
	}
};

// Custom port, with red in-ring (input port), gold.
struct PJ301M_In : SVGPort {
	PJ301M_In() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/PJ301M_In.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Custom port, with green in-ring (output port), gold.
struct PJ301M_Out : SVGPort {
	PJ301M_Out() {
		background->svg = SVG::load(assetPlugin(plugin,"res/components/PJ301M_Out.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

// Freeware "Moog-style" knob, used for any KlokSpid model.
struct KlokSpid_Knob : SVGKnob {
	KlokSpid_Knob() {
  	minAngle = -0.86 * M_PI;
		maxAngle = 0.86 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/components/KS_Knob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

// Custom orange color used by two small LEDs (CV-RATIO, start/stop), KlokSpid module.
// Also, this color is used for medium LED located below CV/TRIG port (KlokSpid module).
struct KlokSpidOrangeLight : GrayModuleLightWidget {
	KlokSpidOrangeLight() {
		addBaseColor(nvgRGB(0xe8, 0xad, 0x10));
	}
};
