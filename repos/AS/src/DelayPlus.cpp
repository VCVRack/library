//**************************************************************************************
//Delay Plus module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//**************************************************************************************
#include "AS.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/filter.hpp"

#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

#define HISTORY_SIZE (1<<21)

struct DelayPlusFx : Module {
	enum ParamIds {
		TIME_PARAM,
		FEEDBACK_PARAM,
		COLOR_PARAM,
		MIX_PARAM,
		BYPASS_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TIME_INPUT,
		FEEDBACK_INPUT,
		COLOR_INPUT,
		COLOR_RETURN,

		MIX_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		COLOR_SEND,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	  enum LightIds {
		BYPASS_LED,
		NUM_LIGHTS
	}; 

	DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
	DoubleRingBuffer<float, 16> outBuffer;
	SampleRateConverter<1> src;
	float lastWet = 0.0f;
	RCFilter lowpassFilter;
	RCFilter highpassFilter;

	int lcd_tempo = 0;
	SchmittTrigger bypass_button_trig;

	bool fx_bypass = false;
	DelayPlusFx() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;

	json_t *toJson()override {
		json_t *rootJm = json_object();

		json_t *statesJ = json_array();
		
			json_t *bypassJ = json_boolean(fx_bypass);
			json_array_append_new(statesJ, bypassJ);
		
		json_object_set_new(rootJm, "as_FxBypass", statesJ);

		return rootJm;
	}

	void fromJson(json_t *rootJm)override {
		json_t *statesJ = json_object_get(rootJm, "as_FxBypass");
		
			json_t *bypassJ = json_array_get(statesJ, 0);

			fx_bypass = !!json_boolean_value(bypassJ);
		
	}

};

void DelayPlusFx::step() {

  if (bypass_button_trig.process(params[BYPASS_SWITCH].value))
    {
		  fx_bypass = !fx_bypass;
	  }
    lights[BYPASS_LED].value = fx_bypass ? 1.0f : 0.0f;

	// Get input to delay block
	float in = inputs[IN_INPUT].value;
	float feedback = clamp(params[FEEDBACK_PARAM].value + inputs[FEEDBACK_INPUT].value / 10.0f, 0.0f, 1.0f);
	float dry = in + lastWet * feedback;

	// Compute delay time in seconds
	//float delay = 1e-3 * powf(10.0 / 1e-3, clampf(params[TIME_PARAM].value + inputs[TIME_INPUT].value / 10.0, 0.0, 1.0));
	float delay = clamp(params[TIME_PARAM].value + inputs[TIME_INPUT].value, 0.001f, 10.0f);
	//LCD display tempo  - show value as ms
	 lcd_tempo = std::round(delay*1000);
	// Number of delay samples
	float index = delay * engineGetSampleRate();

	// TODO This is a horrible digital delay algorithm. Rewrite later.

	// Push dry sample into history buffer
	if (!historyBuffer.full()) {
		historyBuffer.push(dry);
	}

	// How many samples do we need consume to catch up?
	float consume = index - historyBuffer.size();
	//printf("%f\t%d\t%f\n", index, historyBuffer.size(), consume);
	// printf("wanted: %f\tactual: %d\tdiff: %d\tratio: %f\n", index, historyBuffer.size(), consume, index / historyBuffer.size());
	if (outBuffer.empty()) {

		double ratio = 1.0;
		if (consume <= -16)
			ratio = 0.5;
		else if (consume >= 16)
			ratio = 2.0;

		// printf("%f\t%lf\n", consume, ratio);
		int inFrames = min(historyBuffer.size(), 16);
		int outFrames = outBuffer.capacity();
		// printf(">\t%d\t%d\n", inFrames, outFrames);

		src.setRates(ratio * engineGetSampleRate(), engineGetSampleRate());
		src.process((const Frame<1>*)historyBuffer.startData(), &inFrames, (Frame<1>*)outBuffer.endData(), &outFrames);
		historyBuffer.startIncr(inFrames);
		outBuffer.endIncr(outFrames);
		// printf("<\t%d\t%d\n", inFrames, outFrames);
		// printf("====================================\n");
	}
	float out;
	float mix;
	float wet = 0.0f;
	if (!outBuffer.empty()) {
		wet = outBuffer.shift();
	}

	if (outputs[COLOR_SEND].active == false) {
		//internal color
		// Apply color to delay wet output
		float color = clamp(params[COLOR_PARAM].value + inputs[COLOR_INPUT].value / 10.0f, 0.0f, 1.0f);
		float lowpassFreq = 10000.0f * powf(10.0f, clamp(2.0*color, 0.0f, 1.0f));
		lowpassFilter.setCutoff(lowpassFreq / engineGetSampleRate());
		lowpassFilter.process(wet);
		wet = lowpassFilter.lowpass();
		float highpassFreq = 10.0f * powf(100.0f, clamp(2.0f*color - 1.0f, 0.0f, 1.0f));
		highpassFilter.setCutoff(highpassFreq / engineGetSampleRate());
		highpassFilter.process(wet);
		wet = highpassFilter.highpass();
		//lastWet = wet;
	}else {
	//external color, to filter the wet delay signal outside of the module, or to feed another module
		outputs[COLOR_SEND].value = wet;
		wet = inputs[COLOR_RETURN].value;	
	}
	lastWet = wet;
	mix = clamp(params[MIX_PARAM].value + inputs[MIX_INPUT].value / 10.0f, 0.0f, 1.0f);
	out = crossfade(in, wet, mix);
	//check bypass switch status
	if (fx_bypass){
		outputs[OUT_OUTPUT].value = in;
	}else{
		outputs[OUT_OUTPUT].value = out;
	}
}

///////////////////////////////////
struct MsDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
   // NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	 NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.5);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);    
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::right  << std::setw(5) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////
struct DelayPlusFxWidget : ModuleWidget 
{ 
    DelayPlusFxWidget(DelayPlusFx *module);
};

DelayPlusFxWidget::DelayPlusFxWidget(DelayPlusFx *module) : ModuleWidget(module) {

	box.size = Vec(RACK_GRID_WIDTH*8, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DelayPlus.svg")));
		addChild(panel);
	}

	//MS DISPLAY 
	MsDisplayWidget *display = new MsDisplayWidget();
	display->box.pos = Vec(14,50);
	display->box.size = Vec(70, 20);
	display->value = &module->lcd_tempo;
	addChild(display); 	
	int y_offset=40;
 	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//KNOBS
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(74, 38+y_offset), module, DelayPlusFx::TIME_PARAM, 0.001f, 10.0f, 0.350f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(74, 90+y_offset), module, DelayPlusFx::FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(74, 140+y_offset), module, DelayPlusFx::COLOR_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<as_FxKnobWhite>(Vec(74, 213+y_offset), module, DelayPlusFx::MIX_PARAM, 0.0f, 1.0f, 0.5f));
	//BYPASS SWITCH
  	addParam(ParamWidget::create<LEDBezel>(Vec(49, 272+y_offset), module, DelayPlusFx::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f));
  	addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(51.2, 274+y_offset), module, DelayPlusFx::BYPASS_LED));
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 45+y_offset), Port::INPUT, module, DelayPlusFx::TIME_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 95+y_offset), Port::INPUT, module, DelayPlusFx::FEEDBACK_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 145+y_offset), Port::INPUT, module, DelayPlusFx::COLOR_INPUT));
	//DELAY SIGNAL SEND
	addOutput(Port::create<as_PJ301MPort>(Vec(20, 184+y_offset), Port::OUTPUT, module, DelayPlusFx::COLOR_SEND));
	//DELAY SIGNAL RETURN
	addInput(Port::create<as_PJ301MPort>(Vec(75, 184+y_offset), Port::INPUT, module, DelayPlusFx::COLOR_RETURN));
	//INPUTS
	addInput(Port::create<as_PJ301MPort>(Vec(10, 220+y_offset), Port::INPUT, module, DelayPlusFx::MIX_INPUT));
	addInput(Port::create<as_PJ301MPort>(Vec(10, 310), Port::INPUT, module, DelayPlusFx::IN_INPUT));
	//OUTPUT
	addOutput(Port::create<as_PJ301MPort>(Vec(85, 310), Port::OUTPUT, module, DelayPlusFx::OUT_OUTPUT));
}

Model *modelDelayPlusFx = Model::create<DelayPlusFx, DelayPlusFxWidget>("AS", "DelayPlusFx", "DelayPlus Fx", DELAY_TAG, EFFECT_TAG);