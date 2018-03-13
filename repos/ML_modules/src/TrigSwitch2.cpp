#include "ML_modules.hpp"

#include "dsp/digital.hpp"

struct TrigSwitch2 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CV_INPUT,
		TRIG_INPUT,
		NUM_INPUTS = TRIG_INPUT + 8
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT+8
	};
	enum LightIds {
		STEP_LIGHT,
		NUM_LIGHTS = STEP_LIGHT+8
	};

	TrigSwitch2() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) { reset(); };

	void step() override;

	enum OutMode {
		ZERO,
		LAST
	};

	OutMode outMode = ZERO;

	json_t *toJson() override {

		json_t *rootJ = json_object();

		// outMode:
	
		json_object_set_new(rootJ, "outMode", json_integer(outMode));

		return rootJ;
	};
	
	void fromJson(json_t *rootJ) override {

		// outMode:

		json_t *outModeJ = json_object_get(rootJ, "outMode");
		if(outModeJ) outMode = (OutMode) json_integer_value(outModeJ);

	};


	int position=0;


	float outs[8] = {};


	SchmittTrigger stepTriggers[8];


	void reset() override {

		position = 0;
		for(int i=0; i<8; i++) lights[i].value = 0.0;
		for(int i=0; i<8; i++) outs[i]=0.0;
	};

};


void TrigSwitch2::step() {


	if(outMode==ZERO) { for(int i=0; i<8; i++) outs[i]=0.0; }

	for(int i=0; i<8; i++) {
		if( stepTriggers[i].process(inputs[TRIG_INPUT+i].normalize(0.0))) position = i;

	};

	outs[position] = inputs[CV_INPUT].normalize(0.0);

	for(int i=0; i<8; i++) lights[i].value = (i==position)?1.0:0.0;

	for(int i=0; i<8; i++) outputs[OUT_OUTPUT+i].value = outs[i];
	
};



struct TrigSwitch2OutModeItem : MenuItem {

	TrigSwitch2 *trigSwitch2;
	TrigSwitch2::OutMode outMode;

	void onAction(EventAction &e) override {
		trigSwitch2->outMode = outMode;
	};


	void step() override {
		rightText = (trigSwitch2->outMode == outMode)? "✔" : "";
	};

};


struct TrigSwitch2Widget : ModuleWidget {
	TrigSwitch2Widget(TrigSwitch2 *module);
	json_t *toJsonData() ;
	void fromJsonData(json_t *root) ;
	Menu *createContextMenu() override;
};

TrigSwitch2Widget::TrigSwitch2Widget(TrigSwitch2 *module) : ModuleWidget(module) {

	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin,"res/TrigSwitch2.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));


	const float offset_y = 62, delta_y = 32, row1=15, row2 = 55, row3 = 80;

	for (int i=0; i<8; i++) {

		addInput(Port::create<PJ301MPort>(             Vec(row1, offset_y + i*delta_y), Port::INPUT, module, TrigSwitch2::TRIG_INPUT + i));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>( Vec(row2, offset_y + i*delta_y + 8), module, TrigSwitch2::STEP_LIGHT+i));
		addOutput(Port::create<PJ301MPort>(           Vec(row3, offset_y + i*delta_y), Port::OUTPUT, module, TrigSwitch2::OUT_OUTPUT + i));

	}
	addInput(Port::create<PJ301MPort>(Vec(row3, 320), Port::INPUT, module, TrigSwitch2::CV_INPUT));

}

Menu *TrigSwitch2Widget::createContextMenu() {

	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->pushChild(spacerLabel);

	TrigSwitch2 *trigSwitch2 = dynamic_cast<TrigSwitch2*>(module);
	assert(trigSwitch2);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Output Mode";
	menu->pushChild(modeLabel);

	TrigSwitch2OutModeItem *zeroItem = new TrigSwitch2OutModeItem();
	zeroItem->text = "Zero";	
	zeroItem->trigSwitch2 = trigSwitch2;
	zeroItem->outMode = TrigSwitch2::ZERO;
	menu->pushChild(zeroItem);

	TrigSwitch2OutModeItem *lastItem = new TrigSwitch2OutModeItem();
	lastItem->text = "Last";	
	lastItem->trigSwitch2 = trigSwitch2;
	lastItem->outMode = TrigSwitch2::LAST;
	menu->pushChild(lastItem);


	return menu;
};

Model *modelTrigSwitch2 = Model::create<TrigSwitch2, TrigSwitch2Widget>("ML modules", "TrigSwitch2", "TrigSwitch 1->8", SWITCH_TAG, UTILITY_TAG );
