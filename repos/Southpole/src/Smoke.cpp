//
// Smoke = Mutable Instruments Clouds Parasites 
// copied from Arable Instruments Neil
//


#include <string.h>
#include "Southpole.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/digital.hpp"
#include "dsp/vumeter.hpp"
#include "clouds/dsp/granular_processor.h"


struct Smoke : Module {
  enum ParamIds {
    POSITION_PARAM,
    SIZE_PARAM,
    PITCH_PARAM,
    IN_GAIN_PARAM,
    DENSITY_PARAM,
    TEXTURE_PARAM,
    BLEND_PARAM,
    SPREAD_PARAM,
    FEEDBACK_PARAM,
    REVERB_PARAM,
    FREEZE_PARAM,
#ifdef PARASITES
    REVERSE_PARAM,
#endif     
    NUM_PARAMS
  };
  enum InputIds {
    FREEZE_INPUT,
    TRIG_INPUT,
    POSITION_INPUT,
    SIZE_INPUT,
    PITCH_INPUT,
    BLEND_INPUT,
    SPREAD_INPUT,
    FEEDBACK_INPUT,
    REVERB_INPUT,
    IN_L_INPUT,
    IN_R_INPUT,
    DENSITY_INPUT,
    TEXTURE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    OUT_L_OUTPUT,
    OUT_R_OUTPUT,
    NUM_OUTPUTS
  };
	enum LightIds {
		FREEZE_LIGHT,
#ifdef PARASITES
    REVERSE_LIGHT,
#endif   
MIX_GREEN_LIGHT, MIX_RED_LIGHT,
		PAN_GREEN_LIGHT, PAN_RED_LIGHT,
		FEEDBACK_GREEN_LIGHT, FEEDBACK_RED_LIGHT,
		REVERB_GREEN_LIGHT, REVERB_RED_LIGHT,
				NUM_LIGHTS
	};

  SampleRateConverter<2> inputSrc;
  SampleRateConverter<2> outputSrc;
  DoubleRingBuffer<Frame<2>, 256> inputBuffer;
  DoubleRingBuffer<Frame<2>, 256> outputBuffer;

  clouds::PlaybackMode playbackmode =  clouds::PLAYBACK_MODE_GRANULAR;
  
  
  int buffersize = 1;
  int currentbuffersize = 1;
  bool lofi = false;
  bool mono = false;
  uint8_t *block_mem;
  uint8_t *block_ccm;
  clouds::GranularProcessor *processor;

  bool triggered = false;
  float freezeLight = 0.0;
  bool freeze = false;
#ifdef PARASITES
  bool reverse = false;
  float reverseLight = 0.0;
  SchmittTrigger reverseTrigger;
#endif
  SchmittTrigger freezeTrigger;

  Smoke();
  ~Smoke();
  void step() override;
  
  
	json_t *toJson() override {
		json_t *rootJ = json_object();
    //playbackmode, lofi, mono
		json_object_set_new(rootJ, "playbackmode", json_integer(playbackmode));
    json_object_set_new(rootJ, "lofi", json_integer(lofi));
    json_object_set_new(rootJ, "mono", json_integer(mono));
    json_object_set_new(rootJ, "freeze", json_integer(freeze));
    json_object_set_new(rootJ, "buffersize", json_integer(buffersize));
#ifdef PARASITES
    json_object_set_new(rootJ, "reverse", json_integer(reverse));
#endif
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *playbackmodeJ = json_object_get(rootJ, "playbackmode");
		if (playbackmodeJ) {
			playbackmode = (clouds::PlaybackMode)json_integer_value(playbackmodeJ);
		}
    json_t *lofiJ = json_object_get(rootJ, "lofi");
		if (lofiJ) {
			lofi = json_integer_value(lofiJ);
		}
    json_t *monoJ = json_object_get(rootJ, "mono");
		if (monoJ) {
			mono = json_integer_value(monoJ);
		}
    json_t *freezeJ = json_object_get(rootJ, "freeze");
		if (freezeJ) {
			freeze = json_integer_value(freezeJ);
		}
    json_t *buffersizeJ = json_object_get(rootJ, "buffersize");
		if (buffersizeJ) {
			buffersize = json_integer_value(buffersizeJ);
		}      
#ifdef PARASITES
    json_t *reverseJ = json_object_get(rootJ, "reverse");
		if (reverseJ) {
			reverse = json_integer_value(reverseJ);
		}    
#endif    
	}
  
};


Smoke::Smoke() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  const int memLen = 118784;
  const int ccmLen = 65536 - 128;
  block_mem = new uint8_t[memLen]();
  block_ccm = new uint8_t[ccmLen]();
  processor = new clouds::GranularProcessor();
  memset(processor, 0, sizeof(*processor));

  freezeTrigger.setThresholds(0.0, 1.0);
#ifdef PARASITES
  reverseTrigger.setThresholds(0.0, 1.0);   
#endif  
  processor->Init(block_mem, memLen, block_ccm, ccmLen);
}

Smoke::~Smoke() {
  delete processor;
  delete[] block_mem;
  delete[] block_ccm;
}

void Smoke::step() {

  Frame<2> inputFrame;
  // Get input
  if (!inputBuffer.full()) {
    //Frame<2> inputFrame;
    inputFrame.samples[0] = inputs[IN_L_INPUT].value * params[IN_GAIN_PARAM].value / 5.0;
    inputFrame.samples[1] = inputs[IN_R_INPUT].active ? inputs[IN_R_INPUT].value * params[IN_GAIN_PARAM].value / 5.0 : inputFrame.samples[0];
    inputBuffer.push(inputFrame);
  }

  // Trigger
  if (inputs[TRIG_INPUT].value >= 1.0) {
    triggered = true;
  }

  // Render frames
  if (outputBuffer.empty()) {
    clouds::ShortFrame input[32] = {};
    // Convert input buffer
    {
      inputSrc.setRatio(32000.0 / engineGetSampleRate());
      Frame<2> inputFrames[32];
      int inLen = inputBuffer.size();
      int outLen = 32;
      inputSrc.process(inputBuffer.startData(), &inLen, inputFrames, &outLen);
      inputBuffer.startIncr(inLen);

      // We might not fill all of the input buffer if there is a deficiency, but this cannot be avoided due to imprecisions between the input and output SRC.
      for (int i = 0; i < outLen; i++) {
        input[i].l = clampf(inputFrames[i].samples[0] * 32767.0, -32768, 32767);
        input[i].r = clampf(inputFrames[i].samples[1] * 32767.0, -32768, 32767);
      }
    }
    if(currentbuffersize != buffersize){
      //re-init processor with new size
      delete processor;
      delete[] block_mem;
      int memLen = 118784*buffersize;
      const int ccmLen = 65536 - 128;
      block_mem = new uint8_t[memLen]();
      processor = new clouds::GranularProcessor();
      memset(processor, 0, sizeof(*processor));
      processor->Init(block_mem, memLen, block_ccm, ccmLen);
      currentbuffersize = buffersize;
    }

    // Set up processor
    processor->set_num_channels(mono ? 1 : 2);
    processor->set_low_fidelity(lofi);
    // TODO Support the other modes
    processor->set_playback_mode(playbackmode);
    processor->Prepare();

    
    if (freezeTrigger.process(params[FREEZE_PARAM].value)) {
       freeze = !freeze;
    } 
    

    
    clouds::Parameters* p = processor->mutable_parameters();
    p->trigger = triggered;
    p->gate = triggered;
    p->freeze = (inputs[FREEZE_INPUT].value >= 1.0 || freeze);
    p->position = clampf(params[POSITION_PARAM].value + inputs[POSITION_INPUT].value / 5.0, 0.0, 1.0);
    p->size = clampf(params[SIZE_PARAM].value + inputs[SIZE_INPUT].value / 5.0, 0.0, 1.0);
    p->pitch = clampf((params[PITCH_PARAM].value + inputs[PITCH_INPUT].value) * 12.0, -48.0, 48.0);
    p->density = clampf(params[DENSITY_PARAM].value + inputs[DENSITY_INPUT].value / 5.0, 0.0, 1.0);
    p->texture = clampf(params[TEXTURE_PARAM].value + inputs[TEXTURE_INPUT].value / 5.0, 0.0, 1.0);
    float blend = clampf(params[BLEND_PARAM].value + inputs[BLEND_INPUT].value / 5.0, 0.0, 1.0);
    p->dry_wet = blend;
    p->stereo_spread =  clampf(params[SPREAD_PARAM].value + inputs[SPREAD_INPUT].value / 5.0, 0.0, 1.0);;
    p->feedback =  clampf(params[FEEDBACK_PARAM].value + inputs[FEEDBACK_INPUT].value / 5.0, 0.0, 1.0);;
    p->reverb =  clampf(params[REVERB_PARAM].value + inputs[REVERB_INPUT].value / 5.0, 0.0, 1.0);;

#ifdef PARASITES
    if (reverseTrigger.process(params[REVERSE_PARAM].value)) {
       reverse = !reverse;
    } 
    p->granular.reverse = reverse;
    lights[REVERSE_LIGHT].setBrightness(p->granular.reverse ? 1.0 : 0.0);
#endif

    clouds::ShortFrame output[32];
    processor->Process(input, output, 32);
    
    lights[FREEZE_LIGHT].setBrightness(p->freeze ? 1.0 : 0.0);

    // Convert output buffer
    {
      Frame<2> outputFrames[32];
      for (int i = 0; i < 32; i++) {
        outputFrames[i].samples[0] = output[i].l / 32768.0;
        outputFrames[i].samples[1] = output[i].r / 32768.0;
      }

      outputSrc.setRatio( engineGetSampleRate() / 32000.0);
      int inLen = 32;
      int outLen = outputBuffer.capacity();
      outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
      outputBuffer.endIncr(outLen);
    }

    triggered = false;
  }

  // Set output
  Frame<2> outputFrame;
  if (!outputBuffer.empty()) {
    outputFrame = outputBuffer.shift();
    outputs[OUT_L_OUTPUT].value = 5.0 * outputFrame.samples[0];
    outputs[OUT_R_OUTPUT].value = 5.0 * outputFrame.samples[1];
  }

	// Lights
  
	clouds::Parameters *p = processor->mutable_parameters();
	VUMeter vuMeter;
	vuMeter.dBInterval = 6.0;
	Frame<2> lightFrame = p->freeze ? outputFrame : inputFrame;
	vuMeter.setValue(fmaxf(fabsf(lightFrame.samples[0]), fabsf(lightFrame.samples[1])));
	lights[FREEZE_LIGHT].setBrightness(p->freeze ? 0.75 : 0.0);
	lights[MIX_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(3));
	lights[PAN_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(2));
	lights[FEEDBACK_GREEN_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(1));
	lights[REVERB_GREEN_LIGHT].setBrightness(0.0);
	lights[MIX_RED_LIGHT].setBrightness(0.0);
	lights[PAN_RED_LIGHT].setBrightness(0.0);
	lights[FEEDBACK_RED_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(1));
	lights[REVERB_RED_LIGHT].setBrightnessSmooth(vuMeter.getBrightness(0));
  
}




SmokeWidget::SmokeWidget() {
  Smoke *module = new Smoke();
  setModule(module);
  box.size = Vec(6* RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

#ifdef PARASITES
  {
	  panel1 = new SVGPanel();
	  panel1->setBackground(SVG::load(assetPlugin(plugin, "res/Smoke-parasite.svg")));
	  panel1->box.size = box.size;
	  addChild(panel1);
  }
  {
    panel2 = new SVGPanel();
    panel2->setBackground(SVG::load(assetPlugin(plugin, "res/Espectro-parasite.svg")));
    panel2->box.size = box.size;
    addChild(panel2);
  }
  {
    panel3 = new SVGPanel();
    panel3->setBackground(SVG::load(assetPlugin(plugin, "res/Ritardo-parasite.svg")));
    panel3->box.size = box.size;
    addChild(panel3);
  }
  {
    panel4 = new SVGPanel();
    panel4->setBackground(SVG::load(assetPlugin(plugin, "res/Camilla-parasite.svg")));
    panel4->box.size = box.size;
    addChild(panel4);
  }
  {
    panel5 = new SVGPanel();
    panel5->setBackground(SVG::load(assetPlugin(plugin, "res/Oliverb.svg")));
    panel5->box.size = box.size;
    addChild(panel5);
  }
  {
    panel6 = new SVGPanel();
    panel6->setBackground(SVG::load(assetPlugin(plugin, "res/Resonestor.svg")));
    panel6->box.size = box.size;
    addChild(panel6);
  }
  
#else
  {
    panel1 = new SVGPanel();
    panel1->setBackground(SVG::load(assetPlugin(plugin, "res/Smoke.svg")));
    panel1->box.size = box.size;
    addChild(panel1);
  }
  {
    panel2 = new SVGPanel();
    panel2->setBackground(SVG::load(assetPlugin(plugin, "res/Espectro.svg")));
    panel2->box.size = box.size;
    addChild(panel2);
  }
  {
    panel3 = new SVGPanel();
    panel3->setBackground(SVG::load(assetPlugin(plugin, "res/Ritardo.svg")));
    panel3->box.size = box.size;
    addChild(panel3);
  }
  {
    panel4 = new SVGPanel();
    panel4->setBackground(SVG::load(assetPlugin(plugin, "res/Camilla.svg")));
    panel4->box.size = box.size;
    addChild(panel4);
  }
#endif

  const float x1 = 5;
  const float x2 = 35;
  const float x3 = 65; 
  const float y1 = 25.0f;
  const float yh = 29.0f;

 	struct FreezeLight : GreenLight {
		FreezeLight() {
			box.size = Vec(28-16, 28-16);
			bgColor = COLOR_BLACK_TRANSPARENT;
    } 
  };

  addInput(createInput<sp_Port>(Vec(x1, .25*yh+y1), module, Smoke::TRIG_INPUT));

  addInput(createInput<sp_Port>(Vec(x1, 1.25*yh+y1), module, Smoke::FREEZE_INPUT));
  addParam(createParam<LEDButton>(Vec(  x2,   1.35*yh+y1  ), module, Smoke::FREEZE_PARAM, 0.0, 1.0, 0.0));
  addChild(createLight<FreezeLight>(Vec(x2+3, 1.35*yh+y1+3), module,Smoke::FREEZE_LIGHT));
#ifdef PARASITES
  addParam(createParam<LEDButton>(Vec(  x3,   1.35*yh+y1  ), module, Smoke::REVERSE_PARAM, 0.0, 1.0, 0.0));
  addChild(createLight<FreezeLight>(Vec(x3+3, 1.35*yh+y1+3), module, Smoke::REVERSE_LIGHT));
#endif
  addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 2.5*yh+y1), module, Smoke::POSITION_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 2.5*yh+y1), module, Smoke::SIZE_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 2.5*yh+y1), module, Smoke::PITCH_PARAM, -2.0, 2.0, 0.0));

  addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 5.0*yh+y1), module, Smoke::DENSITY_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 5.0*yh+y1), module, Smoke::TEXTURE_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 5.0*yh+y1), module, Smoke::BLEND_PARAM, 0.0, 1.0, 0.5));
  
  
  addParam(createParam<sp_SmallBlackKnob>(Vec(x1, 7.5*yh+y1), module, Smoke::SPREAD_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 7.5*yh+y1), module, Smoke::FEEDBACK_PARAM, 0.0, 1.0, 0.5));
  addParam(createParam<sp_SmallBlackKnob>(Vec(x3, 7.5*yh+y1), module, Smoke::REVERB_PARAM, 0.0, 1.0, 0.5));
  
  addInput(createInput<sp_Port>(Vec(x1, 3.25*yh+y1), module, Smoke::POSITION_INPUT));
  addInput(createInput<sp_Port>(Vec(x2, 3.25*yh+y1), module, Smoke::SIZE_INPUT));
  addInput(createInput<sp_Port>(Vec(x3, 3.25*yh+y1), module, Smoke::PITCH_INPUT));

  addInput(createInput<sp_Port>(Vec(x1, 5.75*yh+y1), module, Smoke::DENSITY_INPUT));
  addInput(createInput<sp_Port>(Vec(x2, 5.75*yh+y1), module, Smoke::TEXTURE_INPUT));
  addInput(createInput<sp_Port>(Vec(x3, 5.75*yh+y1), module, Smoke::BLEND_INPUT));
 
  addInput(createInput<sp_Port>(Vec(x1, 8.25*yh+y1), module, Smoke::SPREAD_INPUT));
  addInput(createInput<sp_Port>(Vec(x2, 8.25*yh+y1), module, Smoke::FEEDBACK_INPUT));
  addInput(createInput<sp_Port>(Vec(x3, 8.25*yh+y1), module, Smoke::REVERB_INPUT));

  addParam(createParam<sp_SmallBlackKnob>(Vec(x2, 10*yh+y1), module, Smoke::IN_GAIN_PARAM, 0.0, 1.0, 0.5));

  addInput(createInput<sp_Port>(Vec(x1, 9.5*yh+y1), module, Smoke::IN_L_INPUT));
  addInput(createInput<sp_Port>(Vec(x1, 10.5*yh+y1), module, Smoke::IN_R_INPUT));
  addOutput(createOutput<sp_Port>(Vec(x3, 9.5*yh+y1), module, Smoke::OUT_L_OUTPUT));
  addOutput(createOutput<sp_Port>(Vec(x3, 10.5*yh+y1), module, Smoke::OUT_R_OUTPUT));
  
  addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 40), module, Smoke::MIX_GREEN_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 30), module, Smoke::PAN_GREEN_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 20), module, Smoke::FEEDBACK_GREEN_LIGHT));
	addChild(createLight<MediumLight<GreenRedLight>>(Vec(x3+10, 10), module, Smoke::REVERB_GREEN_LIGHT));
 
}

void SmokeWidget::step() {
	Smoke *smoke = dynamic_cast<Smoke*>(module);
	assert(smoke);

	panel1->visible = true;
	panel2->visible = false;
	panel3->visible = false;
	panel4->visible = false;
  #ifdef PARASITES
    panel5->visible = false;
    panel6->visible = false;
  #endif
  if ( smoke->playbackmode == clouds::PLAYBACK_MODE_SPECTRAL) {
    panel1->visible = false;
    panel2->visible = true;
  }
  if ( smoke->playbackmode == clouds::PLAYBACK_MODE_LOOPING_DELAY) {
    panel1->visible = false;
    panel3->visible = true;
  }
  if ( smoke->playbackmode == clouds::PLAYBACK_MODE_STRETCH) {
    panel1->visible = false;
    panel4->visible = true;
  }
  #ifdef PARASITES
    if ( smoke->playbackmode == clouds::PLAYBACK_MODE_OLIVERB) {
      panel1->visible = false;
      panel5->visible = true;    
    }
    if ( smoke->playbackmode == clouds::PLAYBACK_MODE_RESONESTOR) {
      panel1->visible = false;
      panel6->visible = true;
    }
  #endif

	ModuleWidget::step();
}

struct CloudsModeItem : MenuItem {
  Smoke *clouds;
  clouds::PlaybackMode mode;

  void onAction(EventAction &e) override {
    clouds->playbackmode = mode;
  }
  void step() override {
    rightText = (clouds->playbackmode == mode) ? "✔" : "";
  }
};


struct CloudsMonoItem : MenuItem {
  Smoke *clouds;
  bool setting;

  void onAction(EventAction &e) override {
    clouds->mono = setting;
  }
  void step() override {
    rightText = (clouds->mono == setting) ? "✔" : "";
  }
};


struct CloudsLofiItem : MenuItem {
  Smoke *clouds;
  bool setting;

  void onAction(EventAction &e) override {
    clouds->lofi = setting;
  }
  void step() override {
    rightText = (clouds->lofi == setting) ? "✔" : "";
  }
};


struct CloudsBufferItem : MenuItem {
  Smoke *clouds;
  int setting;

  void onAction(EventAction &e) override {
    clouds->buffersize = setting;
  }
  void step() override {
    rightText = (clouds->buffersize == setting) ? "✔" : "";
  }
};

Menu *SmokeWidget::createContextMenu() {
  Menu *menu = ModuleWidget::createContextMenu();

  Smoke *clouds = dynamic_cast<Smoke*>(module);
  assert(clouds);


  menu->addChild(construct<MenuLabel>());
  menu->addChild(construct<MenuLabel>(&MenuEntry::text, "MODE"));
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "GRANULAR", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_GRANULAR));
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "SPECTRAL", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_SPECTRAL));
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "LOOPING_DELAY", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_LOOPING_DELAY));
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "STRETCH", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_STRETCH));
#ifdef PARASITES  
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "OLIVERB", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_OLIVERB));
  menu->addChild(construct<CloudsModeItem>(&MenuEntry::text, "RESONESTOR", &CloudsModeItem::clouds, clouds, &CloudsModeItem::mode, clouds::PLAYBACK_MODE_RESONESTOR));
#endif     
  menu->addChild(construct<MenuLabel>(&MenuEntry::text, "STEREO/MONO"));
  menu->addChild(construct<CloudsMonoItem>(&MenuEntry::text, "STEREO", &CloudsMonoItem::clouds, clouds, &CloudsMonoItem::setting, false));
  menu->addChild(construct<CloudsMonoItem>(&MenuEntry::text, "MONO", &CloudsMonoItem::clouds, clouds, &CloudsMonoItem::setting, true));  
  
  menu->addChild(construct<MenuLabel>(&MenuEntry::text, "HIFI/LOFI"));
  menu->addChild(construct<CloudsLofiItem>(&MenuEntry::text, "HIFI", &CloudsLofiItem::clouds, clouds, &CloudsLofiItem::setting, false));
  menu->addChild(construct<CloudsLofiItem>(&MenuEntry::text, "LOFI", &CloudsLofiItem::clouds, clouds, &CloudsLofiItem::setting, true));  
  
#ifdef BUFFERRESIZING
// disable by default as it seems to make alternative modes unstable
  menu->addChild(construct<MenuLabel>(&MenuEntry::text, "BUFFER SIZE (EXPERIMENTAL)"));
  menu->addChild(construct<CloudsBufferItem>(&MenuEntry::text, "ORIGINAL", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 1));
  menu->addChild(construct<CloudsBufferItem>(&MenuEntry::text, "2X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 2));
  menu->addChild(construct<CloudsBufferItem>(&MenuEntry::text, "4X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 4));
  menu->addChild(construct<CloudsBufferItem>(&MenuEntry::text, "8X", &CloudsBufferItem::clouds, clouds, &CloudsBufferItem::setting, 8));
#endif  
    
  return menu;
}



