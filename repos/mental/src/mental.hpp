#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

/// 06 versions

extern Model *modelMentalMults;
extern Model *modelMentalSubMixer;
extern Model *modelMentalMixer;
extern Model *modelMentalFold;
extern Model *modelMentalClip;
extern Model *modelMentalGates;
extern Model *modelMentalABSwitches;
extern Model *modelMentalQuantiser;
extern Model *modelMentalChord;
extern Model *modelMentalMuxes;
extern Model *modelMentalLogic;
extern Model *modelMentalButtons;
extern Model *modelMentalSums;
extern Model *modelMentalPitchShift;
extern Model *modelMentalClockDivider;
extern Model *modelMentalCartesian;
extern Model *modelMentalPatchMatrix;
extern Model *modelMentalBinaryDecoder;
extern Model *modelMentalSwitch8;
extern Model *modelMentalMux8;
extern Model *modelMentalCounters;
extern Model *modelMentalKnobs;
extern Model *modelMentalGateMaker;
extern Model *modelMentalMasterClock;
//extern Model *modelMentalPatchNotes;
extern Model *modelMentalQuadLFO;
extern Model *modelMentalRadioButtons;


/*struct MentalSubMixerWidget : ModuleWidget {
	MentalSubMixerWidget();
};

struct MentalMultsWidget : ModuleWidget {
	MentalMultsWidget();
};

struct MentalMixerWidget : ModuleWidget {
	MentalMixerWidget();
};

struct MentalFoldWidget : ModuleWidget {
	MentalFoldWidget();
};

struct MentalClipWidget : ModuleWidget {
	MentalClipWidget();
};

struct MentalGatesWidget : ModuleWidget {
	MentalGatesWidget();
};

struct MentalABSwitchesWidget : ModuleWidget {
	MentalABSwitchesWidget();
};

//struct MentalNoiseGateWidget : ModuleWidget {
//	MentalNoiseGateWidget();
//};

struct MentalQuantiserWidget : ModuleWidget {
	MentalQuantiserWidget();
};

struct MentalChordWidget : ModuleWidget {
	MentalChordWidget();
};

struct MentalMuxesWidget : ModuleWidget {
	MentalMuxesWidget();
};

struct MentalLogicWidget : ModuleWidget {
	MentalLogicWidget();
};

struct MentalButtonsWidget : ModuleWidget {
	MentalButtonsWidget();
};

struct MentalSumsWidget : ModuleWidget {
	MentalSumsWidget();
};

struct MentalPitchShiftWidget : ModuleWidget {
	MentalPitchShiftWidget();
};

struct MentalClockDividerWidget : ModuleWidget {
	MentalClockDividerWidget();
};

struct MentalCartesianWidget : ModuleWidget {
	MentalCartesianWidget();
};

struct MentalPatchMatrixWidget : ModuleWidget {
	MentalPatchMatrixWidget();
};

struct MentalBinaryDecoderWidget : ModuleWidget {
	MentalBinaryDecoderWidget();
};

struct MentalSwitch8Widget : ModuleWidget {
	MentalSwitch8Widget();
};

struct MentalMux8Widget : ModuleWidget {
	MentalMux8Widget();
};

struct MentalCountersWidget : ModuleWidget {
	MentalCountersWidget();
};

struct MentalKnobsWidget : ModuleWidget {
	MentalKnobsWidget();
};

struct MentalGateMakerWidget : ModuleWidget {
	MentalGateMakerWidget();
};

struct MentalMasterClockWidget : ModuleWidget {
	MentalMasterClockWidget();
};

struct MentalPatchNotesWidget : ModuleWidget {
  TextField * patch_notes;
	MentalPatchNotesWidget();
  json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};

struct MentalQuadLFOWidget : ModuleWidget {
	MentalQuadLFOWidget();
};

struct MentalRadioButtonsWidget : ModuleWidget {
	MentalRadioButtonsWidget();
}; */

/////////////////////////////////////////////
// ports

struct OutPort : SVGPort {
	OutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/OutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct InPort : SVGPort {
	InPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/InPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct CVInPort : SVGPort {
	CVInPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/CVInPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct CVOutPort : SVGPort {
	CVOutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/CVOutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct GateInPort : SVGPort {
	GateInPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/GateInPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct GateOutPort : SVGPort {
	GateOutPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/components/GateOutPort.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
