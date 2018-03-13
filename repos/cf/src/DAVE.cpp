


#include "cf.hpp"


struct DAVE : Module {
	enum ParamIds {
		
		NUM_PARAMS
	};
	enum InputIds {

		NUM_INPUTS
	};
	enum OutputIds {

		NUM_OUTPUTS
	};


	DAVE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

};


void DAVE::step() {

}
struct DAVEWidget : ModuleWidget {
	DAVEWidget(DAVE *module);
};

DAVEWidget::DAVEWidget(DAVE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/DAVE.svg")));
}

Model *modelDAVE = Model::create<DAVE, DAVEWidget>("cf", "DAVE", "Dave", BLANK_TAG);
