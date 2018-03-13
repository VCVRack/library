#include "ML_modules.hpp"

#include "dsp/digital.hpp"

struct TrigSwitch : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		CV_INPUT = TRIG_INPUT + 8,
		NUM_INPUTS = CV_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};

	TrigSwitch() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	int position=0;



        const float in_min[4] = {0.0, 0.0, 0.0, -5.0};
        const float in_max[4] = {8.0, 6.0, 10.0, 5.0};


	SchmittTrigger stepTriggers[8];


	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
	};

};


void TrigSwitch::step() {

	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process(inputs[TRIG_INPUT+i].normalize(0.0))) position = i;
		lights[i].value = (i==position)?1.0:0.0;
	};

	outputs[OUT_OUTPUT].value = inputs[CV_INPUT+position].normalize(0.0);
};



struct TrigSwitchWidget : ModuleWidget {
	TrigSwitchWidget(TrigSwitch *module);
};

TrigSwitchWidget::TrigSwitchWidget(TrigSwitch *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigSwitch.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


	const float offset_y = 62, delta_y = 32, row1=15, row2 = 55, row3 = 80;

	for (int i=0; i<8; i++) {

		addInput(Port::create<PJ301MPort>(             Vec(row1, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch::TRIG_INPUT + i));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>( Vec(row2, offset_y + i*delta_y + 8), module, TrigSwitch::STEP_LIGHT+i));
		addInput(Port::create<PJ301MPort>(             Vec(row3, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch::CV_INPUT + i));

	}
	addOutput(Port::create<PJ301MPort>(Vec(row3, 320), Port::OUTPUT, module, TrigSwitch::OUT_OUTPUT));

}

Model *modelTrigSwitch = Model::create<TrigSwitch, TrigSwitchWidget>("ML modules", "TrigSwitch", "TrigSwitch 8->1", SWITCH_TAG, UTILITY_TAG );