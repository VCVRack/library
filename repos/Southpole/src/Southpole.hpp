
#include "rack.hpp"
using namespace rack;
extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct SmokeWidget : ModuleWidget {
	SVGPanel *panel1;
	SVGPanel *panel2;
	SVGPanel *panel3;
	SVGPanel *panel4;
	SVGPanel *panel5;
	SVGPanel *panel6;	
	SmokeWidget();
	void step() override;
	Menu *createContextMenu() override;
};

#ifdef PARASITES

struct SplashParasiteWidget : ModuleWidget {
	SVGPanel *panel0;
	SVGPanel *panel1;
	SVGPanel *panel2;
	SplashParasiteWidget();
	void step() override;
	Menu *createContextMenu() override;
};


#else

struct AnnuliWidget : ModuleWidget {
	SVGPanel *panel;
	SVGPanel *panel2;
	AnnuliWidget();
	void step() override;
	Menu *createContextMenu() override;
};

struct CornrowsWidget : ModuleWidget {
	CornrowsWidget();
	Menu *createContextMenu() override;
};

struct SplashWidget : ModuleWidget {
	SVGPanel *tidesPanel;
	SVGPanel *sheepPanel;
	SplashWidget();
	void step() override;
	Menu *createContextMenu() override;
};

struct BalaclavaWidget : ModuleWidget {	BalaclavaWidget(); };
struct BandanaWidget : ModuleWidget { BandanaWidget(); };
struct FallsWidget : ModuleWidget { FallsWidget(); };
struct AuxWidget : ModuleWidget { AuxWidget(); };

struct AbrWidget : ModuleWidget { AbrWidget(); };
struct ButWidget : ModuleWidget { ButWidget(); };
struct EtagereWidget : ModuleWidget { EtagereWidget(); };
struct DeuxEtageresWidget : ModuleWidget { DeuxEtageresWidget(); };
struct RiemannWidget : ModuleWidget { RiemannWidget(); };
struct SnsWidget : ModuleWidget { SnsWidget(); };
struct BounceWidget : ModuleWidget { BounceWidget(); };
struct WriggleWidget : ModuleWidget { WriggleWidget(); };
struct PisteWidget : ModuleWidget {	PisteWidget(); };
struct FuseWidget : ModuleWidget {
	FuseWidget();
	Menu *createContextMenu() override;
};

struct SsshWidget : ModuleWidget {	SsshWidget(); };
struct SnakeWidget : ModuleWidget {	SnakeWidget(); };
struct GnomeWidget : ModuleWidget {	GnomeWidget(); };
struct Blank1HPWidget : ModuleWidget {	Blank1HPWidget(); };
struct Blank2HPWidget : ModuleWidget {	Blank2HPWidget(); };
struct Blank4HPWidget : ModuleWidget {	Blank4HPWidget(); };
struct Blank8HPWidget : ModuleWidget {
	Blank8HPWidget(); 
//	TextField *textField;
//	json_t *toJson() override;
//	void fromJson(json_t *rootJ) override;
};
struct Blank16HPWidget : ModuleWidget {	Blank16HPWidget(); };
struct Blank42HPWidget : ModuleWidget {	Blank42HPWidget(); };

#endif

// GUI COMPONENTS

struct sp_Port : SVGPort {
	sp_Port() {
		background->svg = SVG::load(assetPlugin(plugin, "res/sp-Port20.svg"));
		background->wrap();
		box.size = background->box.size; 
	}
};

struct sp_Switch : SVGSwitch, ToggleSwitch {
	sp_Switch() {
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/sp-switchv_1.svg")));
	}
};

struct sp_Encoder : SVGKnob {
	sp_Encoder() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-encoder.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct sp_BlackKnob : SVGKnob {
	sp_BlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		sw->wrap();
		box.size = Vec(32,32);
	}
};

struct sp_SmallBlackKnob : SVGKnob {
	sp_SmallBlackKnob() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-knobBlack.svg"));
		sw->wrap();
		box.size = Vec(20,20);
	}
};

struct sp_Trimpot : SVGKnob {
	sp_Trimpot() {
        minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		sw->svg = SVG::load(assetPlugin(plugin, "res/sp-trimpot.svg"));
		sw->wrap();
		box.size = Vec(18,18);
	}
};

