//**************************************************************************************
//
//BPM Clock module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//Based on code taken from Master Clock Module VCV Module Strum 2017 https://github.com/Strum/Strums_Mental_VCV_Modules
//**************************************************************************************

#include "AS.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

struct LFOGenerator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;	
	void setFreq(float freq_to_set)
  {
    freq = freq_to_set;
  }		
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}	
	float sqr() {
		float sqr = phase < pw ? 1.0f : -1.0f;
		return sqr;
	}
};

struct BPMClock : Module {
	enum ParamIds {    
    TEMPO_PARAM,    
    TIMESIGTOP_PARAM,
    TIMESIGBOTTOM_PARAM,
    RESET_SWITCH,
    RUN_SWITCH,    
		NUM_PARAMS
	};  
	enum InputIds { 
    RESET_INPUT,    
		NUM_INPUTS
	};
	enum OutputIds {
		BEAT_OUT,
    EIGHTHS_OUT,
    SIXTEENTHS_OUT,
    BAR_OUT,
    RESET_OUTPUT,       
		NUM_OUTPUTS
	};
  enum LightIds {
		RESET_LED,
    RUN_LED,
		NUM_LIGHTS
	}; 
  
  LFOGenerator clock;
  
  SchmittTrigger eighths_trig;
	SchmittTrigger quarters_trig;
  SchmittTrigger bars_trig;
  SchmittTrigger run_button_trig;
	SchmittTrigger reset_btn_trig;
  SchmittTrigger reset_ext_trig;

  const float lightLambda = 0.075f;
  float resetLight = 0.0f;

  bool running = true;
  
  int eighths_count = 0;
	int quarters_count = 0;
  int bars_count = 0;
  
  int tempo, time_sig_top, time_sig_bottom = 0;
  float frequency = 2.0f;
  int quarters_count_limit = 4;
  int eighths_count_limit = 2;
  int bars_count_limit = 16;
  
	BPMClock() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    /*
    params.resize(NUM_PARAMS);
    inputs.resize(NUM_INPUTS);
    outputs.resize(NUM_OUTPUTS);
    lights.resize(NUM_LIGHTS);
    eighths_trig.setThresholds(0.0, 1.0);
    quarters_trig.setThresholds(0.0, 1.0);
    bars_trig.setThresholds(0.0, 1.0);
*/
  }
	void step() override;

  json_t *toJson() override
  {
		json_t *rootJ = json_object();
    json_t *button_statesJ = json_array();
		json_t *button_stateJ = json_integer((int)running);
		json_array_append_new(button_statesJ, button_stateJ);		
		json_object_set_new(rootJ, "run", button_statesJ);    
    return rootJ;
  }
  
  void fromJson(json_t *rootJ) override
  {
    json_t *button_statesJ = json_object_get(rootJ, "run");
		if (button_statesJ)
    {			
				json_t *button_stateJ = json_array_get(button_statesJ,0);
				if (button_stateJ)
					running = !!json_integer_value(button_stateJ);			
		}  
  }  
};

void BPMClock::step()
{
  if (run_button_trig.process(params[RUN_SWITCH].value)){
		  running = !running;
	}
  lights[RUN_LED].value = running ? 1.0f : 0.0f;
    
  tempo = std::round(params[TEMPO_PARAM].value);
  time_sig_top = std::round(params[TIMESIGTOP_PARAM].value);
  time_sig_bottom = std::round(params[TIMESIGBOTTOM_PARAM].value);
  time_sig_bottom = std::pow(2,time_sig_bottom+1);
 
  frequency = tempo/60.0f;
  //RESET TRIGGERS
  //EXTERNAL RESET TRIGGER
	if (reset_ext_trig.process(inputs[RESET_INPUT].value)) {
    eighths_count = 0;
    quarters_count = 0;
    bars_count = 0;
    resetLight = 1.0;
    outputs[RESET_OUTPUT].value = 10.0f;
  //INTERNAL RESET TRIGGER
  }else  if (reset_btn_trig.process(params[RESET_SWITCH].value)) {
    eighths_count = 0;
    quarters_count = 0;
    bars_count = 0;
    resetLight = 1.0;
    outputs[RESET_OUTPUT].value = 10.0f;
  }else{
    outputs[RESET_OUTPUT].value = 0.0f;
  }

  resetLight -= resetLight / lightLambda / engineGetSampleRate();
  lights[RESET_LED].value = resetLight;

  if (!running) {
    eighths_count = 0;
    quarters_count = 0;
    bars_count = 0; 
    outputs[BAR_OUT].value = 0.0;
    outputs[BEAT_OUT].value = 0.0;
    outputs[EIGHTHS_OUT].value = 0.0;
    outputs[SIXTEENTHS_OUT].value = 0.0;
    outputs[SIXTEENTHS_OUT].value = 0.0; 
  } else{
    if (time_sig_top == time_sig_bottom){
      clock.setFreq(frequency*4);
      quarters_count_limit = 4;
      eighths_count_limit = 2;
      bars_count_limit = 16;    
    } else{
      if (time_sig_bottom == 4){
        quarters_count_limit = 4;
        eighths_count_limit = 2;
        bars_count_limit = time_sig_top * 4;  
        clock.setFreq(frequency*4); 
      }
      if (time_sig_bottom == 8){
        quarters_count_limit = 4;
        eighths_count_limit = 2;
        bars_count_limit = time_sig_top * 2;
        clock.setFreq(frequency*4);
        if ((time_sig_top % 3) == 0){
          quarters_count_limit = 6;
          eighths_count_limit = 2;
          bars_count_limit = (time_sig_top/3) * 6;
          clock.setFreq(frequency*6);
        }      
      }
    }
  
    clock.step(1.0 / engineGetSampleRate());
    outputs[SIXTEENTHS_OUT].value = clamp(10.0f * clock.sqr(), 0.0f, 10.0f);
 
    if (eighths_trig.process(clock.sqr()) && eighths_count <= eighths_count_limit)
      eighths_count++;
    if (eighths_count >= eighths_count_limit)
    {
      eighths_count = 0;    
    }
    if (eighths_count == 0) outputs[EIGHTHS_OUT].value = 10.0f;
    else outputs[EIGHTHS_OUT].value = 0.0f;
    
    if (quarters_trig.process(clock.sqr()) && quarters_count <= quarters_count_limit)
      quarters_count++;
    if (quarters_count >= quarters_count_limit)
    {
      quarters_count = 0;    
    }
    if (quarters_count == 0) outputs[BEAT_OUT].value = 10.0f;
    else outputs[BEAT_OUT].value = 0.0f;
    
    if (bars_trig.process(clock.sqr()) && bars_count <= bars_count_limit)
      bars_count++;
    if (bars_count >= bars_count_limit)
    {
      bars_count = 0;    
    }
    if (bars_count == 0) outputs[BAR_OUT].value = 10.0f;
    else outputs[BAR_OUT].value = 0.0f; 
    }
}

////////////////////////////////////
struct BpmDisplayWidget : TransparentWidget {
  int *value;
  std::shared_ptr<Font> font;

  BpmDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
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
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
////////////////////////////////////
struct SigDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  SigDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
    
    
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    //NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
     NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);    
    // text 
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::setw(2) << *value;

    Vec textPos = Vec(3, 17); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};
//////////////////////////////////

struct BPMClockWidget : ModuleWidget 
{ 
    BPMClockWidget(BPMClock *module);
};


BPMClockWidget::BPMClockWidget(BPMClock *module) : ModuleWidget(module) {
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
  
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/BPMClock.svg")));
		addChild(panel);
	}
  //SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  //BPM DISPLAY 
  BpmDisplayWidget *display = new BpmDisplayWidget();
  display->box.pos = Vec(23,50);
  display->box.size = Vec(45, 20);
  display->value = &module->tempo;
  addChild(display); 
  //TEMPO KNOB
  addParam(ParamWidget::create<as_KnobBlack>(Vec(26, 74), module, BPMClock::TEMPO_PARAM, 40.0f, 250.0f, 120.0f));
  //SIG TOP DISPLAY 
  SigDisplayWidget *display2 = new SigDisplayWidget();
  display2->box.pos = Vec(54,123);
  display2->box.size = Vec(30, 20);
  display2->value = &module->time_sig_top;
  addChild(display2);
  //SIG TOP KNOB
  addParam(ParamWidget::create<as_KnobBlack>(Vec(8, 110), module, BPMClock::TIMESIGTOP_PARAM,2.0f, 15.0f, 4.0f));
  //SIG BOTTOM DISPLAY    
  SigDisplayWidget *display3 = new SigDisplayWidget();
  display3->box.pos = Vec(54,155);
  display3->box.size = Vec(30, 20);
  display3->value = &module->time_sig_bottom;
  addChild(display3); 
  //SIG BOTTOM KNOB
  addParam(ParamWidget::create<as_KnobBlack>(Vec(8, 150), module, BPMClock::TIMESIGBOTTOM_PARAM,0.0f, 3.0f, 1.0f));
  //RESET & RUN LEDS
  addParam(ParamWidget::create<LEDBezel>(Vec(55, 202), module, BPMClock::RUN_SWITCH , 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(57.2, 204.3), module, BPMClock::RUN_LED));
  addParam(ParamWidget::create<LEDBezel>(Vec(10.5, 202), module, BPMClock::RESET_SWITCH , 0.0f, 1.0f, 0.0f));
  addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(12.7, 204.3), module, BPMClock::RESET_LED));
  //RESET INPUT
  addInput(Port::create<as_PJ301MPort>(Vec(10, 240), Port::INPUT, module, BPMClock::RESET_INPUT));
  //RESET OUTPUT
  addOutput(Port::create<as_PJ301MPort>(Vec(55, 240), Port::OUTPUT, module, BPMClock::RESET_OUTPUT));
  //TEMPO OUTPUTS
  addOutput(Port::create<as_PJ301MPort>(Vec(10, 280), Port::OUTPUT, module, BPMClock::BAR_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(55, 280), Port::OUTPUT, module, BPMClock::BEAT_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(10, 320), Port::OUTPUT, module, BPMClock::EIGHTHS_OUT));
  addOutput(Port::create<as_PJ301MPort>(Vec(55, 320), Port::OUTPUT, module, BPMClock::SIXTEENTHS_OUT));

}

Model *modelBPMClock = Model::create<BPMClock, BPMClockWidget>("AS", "BPMClock", "BPM Clock", CLOCK_TAG);