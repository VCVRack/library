#include "NonLinearInstruments.hpp"
#include "classes/Ballistic.cpp"
#include "dsp/digital.hpp"


struct BallisticENV : Module {
	enum ParamIds {
		IMPULSE_PARAM,
		IMPULSE_MOD_PARAM,
		ANGLE_PARAM,
		ANGLE_MOD_PARAM,
		GRAVITY_PARAM,
		GRAVITY_MOD_PARAM,
		SHOOT_PARAM,
		BOUNCE_PARAM,
		BOUNCE_MOD_PARAM,
		BOUNCE_ON_OFF,
		NUM_PARAMS
	};
	enum InputIds {
		IMPULSE_INPUT,
		ANGLE_INPUT,
		GRAVITY_INPUT,
		BOUNCE_INPUT,
		TRIGGER_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		X_OUTPUT,
		C_OUTPUT,
		ZERO_TRIG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		SHOOT_LIGHT,
		NUM_LIGHTS
	};

	Ballistic parabola;

	BallisticENV() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};

void BallisticENV::step() {		
	parabola.setBounceOnOff( params[BOUNCE_ON_OFF].value );		
	parabola.setImpulse( params[IMPULSE_PARAM].value + params[IMPULSE_MOD_PARAM].value * inputs[IMPULSE_INPUT].value  );
	parabola.setGravity( params[GRAVITY_PARAM].value + params[GRAVITY_MOD_PARAM].value * inputs[GRAVITY_INPUT].value  );
	parabola.setAngle  ( params[ANGLE_PARAM].value   + params[ANGLE_MOD_PARAM].value   * inputs[ANGLE_INPUT].value    );
	parabola.setBounce ( params[BOUNCE_PARAM].value  + params[BOUNCE_MOD_PARAM].value   * inputs[BOUNCE_INPUT].value   );
	parabola.shoot( params[SHOOT_PARAM].value   + inputs[TRIGGER_INPUT].value );
	outputs[X_OUTPUT].value = parabola.getAudio();
	outputs[C_OUTPUT].value = parabola.getControl();
	outputs[ZERO_TRIG_OUTPUT].value = parabola.getZeroTrigger();
}


BallisticENVWidget::BallisticENVWidget() {
	BallisticENV *module = new BallisticENV();
	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/BallisticENV.svg")));
		addChild(panel);
	}
	
	
	addParam(createParam<KorgMedGreyKnob>(Vec(75, 40), module, BallisticENV::IMPULSE_PARAM, 0.0, 1.0, 0.5 ));		
	addParam(createParam<KorgSmallGreyKnob>(Vec(40, 48), module, BallisticENV::IMPULSE_MOD_PARAM, 0.0, 1.0, 0.0 ));		
	addInput(createInput<PJ301MPort>(Vec(10, 50), module, BallisticENV::IMPULSE_INPUT)); // -5 ~ +5
	addParam(createParam<KorgMedGreyKnob>(Vec(75, 95), module, BallisticENV::ANGLE_PARAM, 0.0, 1.0, 1.0));
	addParam(createParam<KorgSmallGreyKnob>(Vec(40, 103), module, BallisticENV::ANGLE_MOD_PARAM, 0.0, 1.0, 0.0 ));		
	addInput(createInput<PJ301MPort>(Vec(10, 105), module, BallisticENV::ANGLE_INPUT)); // -5 ~ +5
	addParam(createParam<KorgMedGreyKnob>(Vec(75, 150), module, BallisticENV::GRAVITY_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<KorgSmallGreyKnob>(Vec(40, 158), module, BallisticENV::GRAVITY_MOD_PARAM, 0.0, 1.0, 0.0 ));	
	addInput(createInput<PJ301MPort>(Vec(10, 160), module, BallisticENV::GRAVITY_INPUT)); // -5 ~ +5
	addParam(createParam<KorgCKSS>(Vec(58, 192), module, BallisticENV::BOUNCE_ON_OFF, 0.0, 1.0, 1.0));
	addParam(createParam<KorgMedGreyKnob>(Vec(75, 216), module, BallisticENV::BOUNCE_PARAM, 0.0, 1.0, 0.5));
	addParam(createParam<KorgSmallGreyKnob>(Vec(40, 222), module, BallisticENV::BOUNCE_MOD_PARAM, 0.0, 1.0, 0.0 ));	
	addInput(createInput<PJ301MPort>(Vec(10, 224), module, BallisticENV::BOUNCE_INPUT)); // -5 ~ +5
	addParam(createParam<CKD6>(Vec(42, 262), module, BallisticENV::SHOOT_PARAM, 0.0, 5.0, 0.0));
	addInput(createInput<PJ301MPort>(Vec(10, 264), module, BallisticENV::TRIGGER_INPUT));	
	addOutput(createOutput<PJ301MPort>(Vec(11, 320), module, BallisticENV::X_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(45, 320), module, BallisticENV::C_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(80, 320), module, BallisticENV::ZERO_TRIG_OUTPUT));

	
}
