#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

namespace bogaudio {

struct Button18 : SVGSwitch, MomentarySwitch {
	Button18() {
		addFrame(SVG::load(assetPlugin(plugin, "res/button_18px_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/button_18px_1.svg")));
		box.size = Vec(18, 18);
	}
};

struct Knob16 : RoundKnob {
	Knob16() {
		setSVG(SVG::load(assetPlugin(plugin, "res/knob_16px.svg")));
		box.size = Vec(16, 16);
	}
};

struct Knob26 : RoundKnob {
	Knob26() {
		setSVG(SVG::load(assetPlugin(plugin, "res/knob_26px.svg")));
		box.size = Vec(26, 26);
	}
};

struct Knob29 : RoundKnob {
	Knob29() {
		setSVG(SVG::load(assetPlugin(plugin, "res/knob_29px.svg")));
		box.size = Vec(29, 29);
	}
};

struct Knob38 : RoundKnob {
	Knob38() {
		setSVG(SVG::load(assetPlugin(plugin, "res/knob_38px.svg")));
		box.size = Vec(38, 38);
	}
};

struct Port24 : SVGPort {
	Port24() {
		background->svg = SVG::load(assetPlugin(plugin, "res/port.svg"));
		background->wrap();
		box.size = Vec(24, 24);
	}
};

struct SliderSwitch2State14 : SVGSwitch, ToggleSwitch {
	SliderSwitch2State14() {
		addFrame(SVG::load(assetPlugin(plugin, "res/slider_switch_2_14px_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/slider_switch_2_14px_1.svg")));
	}
};

struct StatefulButton : ParamWidget, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> _frames;
	SVGWidget* _svgWidget; // deleted elsewhere.

	StatefulButton(const char* offSVGPath, const char* onSVGPath);
	void step() override;
	void onDragStart(EventDragStart& e) override;
	void onDragEnd(EventDragEnd& e) override;
};

struct StatefulButton9 : StatefulButton {
	StatefulButton9() : StatefulButton("res/button_9px_0.svg", "res/button_9px_1.svg") {}
};

} // namespace bogaudio
