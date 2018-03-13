#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct QU4DiTWidget : ModuleWidget { QU4DiTWidget(); };
struct BallisticENVWidget : ModuleWidget { BallisticENVWidget(); };
struct LuciCellWidget : ModuleWidget { LuciCellWidget(); };
struct Luci4AudioSumWidget : ModuleWidget { Luci4AudioSumWidget(); };
struct Luci4ParamDistrWidget : ModuleWidget { Luci4ParamDistrWidget(); };
struct LuciControlRNDWidget : ModuleWidget { LuciControlRNDWidget(); };
struct LuciControlFREQWidget : ModuleWidget { LuciControlFREQWidget(); };
struct LuciControlINFcoarseWidget : ModuleWidget { LuciControlINFcoarseWidget(); };
//struct LuciControlINFfineWidget : ModuleWidget { LuciControlINFfineWidget(); };


////////////////////////
// module components
////////////////////////

struct LuciVeryLargeBlueKnob : SVGKnob {
	LuciVeryLargeBlueKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/LuciVeryLargeBlueKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct KorgLargeGoldKnob : SVGKnob {
	KorgLargeGoldKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgLargeGoldKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct KorgLargeBlueKnob : SVGKnob {
	KorgLargeBlueKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgLargeBlueKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct KorgLargeGreyKnob : SVGKnob {
	KorgLargeGreyKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgLargeGreyKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct KorgMedGoldKnob : SVGKnob {
	KorgMedGoldKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgMedGoldKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};


struct KorgMedGreyKnob : SVGKnob {
	KorgMedGreyKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgMedGreyKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct KorgSmallGreyKnob : SVGKnob {
	KorgSmallGreyKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/KorgSmallGreyKnob.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct KorgCKSS : SVGSwitch, ToggleSwitch {
	KorgCKSS() {
		addFrame(SVG::load(assetPlugin(plugin,"res/KorgCKSS_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/KorgCKSS_1.svg")));
	}
};

struct LuciControlRandomizeButton : SVGSwitch, MomentarySwitch {
	LuciControlRandomizeButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/LuciControlRandomizeButton.svg")));
			}
		};


struct BigLuciButton : SVGSwitch, MomentarySwitch {
	BigLuciButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/BigLEDButton.svg")));
	}
};

template <typename BASE>
struct	luciControlRandomizeLight : BASE {
	luciControlRandomizeLight() {
		this->box.size = (Vec(80, 80));
	}
};

template <typename BASE>
struct luciLight : BASE {
	luciLight() {
		this->box.size = mm2px(Vec(100, 100));
	}
};