#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef LPTEST_MODULE
////////////////////
// module widgets
////////////////////
using namespace rack;
extern Plugin *plugin;

struct LaunchpadTest;
struct LaunchpadTestWidget : ModuleWidget
{
	LaunchpadTestWidget(LaunchpadTest * module);
};

struct PatternBtn : SVGSwitch, ToggleSwitch {
	PatternBtn() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_1.svg")));
	}
};

struct LaunchpadTest : Module
{
	enum ParamIds
	{
		BTN1,
		BTN2,
		BTN3,
		BTN4,
		GATE_TIME,
		NUM_PARAMS
	};
	enum InputIds
	{

		NUM_INPUTS
	};
	enum OutputIds
	{
		KNOB_OUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		LP_CONNECTED,
		NUM_LIGHTS
	};
	LaunchpadTest() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		drv = new LaunchpadBindingDriver(this, Scene8, 1);
	}
	~LaunchpadTest()
	{
		delete drv;
	}
	void step() override;

	LaunchpadBindingDriver *drv;
};

#endif
