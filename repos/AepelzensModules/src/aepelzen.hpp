#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct Knob29 : RoundKnob {
	Knob29() {
		setSVG(SVG::load(assetPlugin(plugin, "res/knob_29px.svg")));
		box.size = Vec(29, 29);
	}
};

extern Model *modelQuadSeq;
extern Model *modelGateSeq;
extern Model *modelDice;
extern Model *modelBurst;
extern Model *modelFolder;
extern Model *modelWalker;
extern Model *modelErwin;
extern Model *modelWerner;
