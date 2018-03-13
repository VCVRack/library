#include "aepelzen.hpp"
#include "dsp/digital.hpp"
#include <samplerate.h>

#define BUF_LEN 32
#define UPSAMPLE_RATIO 4

struct Folder : Module
{
  enum ParamIds
    {
      GAIN_PARAM,
      GAIN_ATT_PARAM,
      SYM_PARAM,
      SYM_ATT_PARAM,
      STAGE_PARAM,
      NUM_PARAMS
    };
  enum InputIds
    {
      GATE_INPUT,
      GAIN_INPUT,
      SYM_INPUT,
      NUM_INPUTS
    };
  enum OutputIds
    {
      GATE_OUTPUT,
      NUM_OUTPUTS
    };

  enum LightIds
    {
      NUM_LIGHTS
    };

  void step() override;

  Folder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    state = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
    state_down = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &error);
  }

  ~Folder() {
    src_delete(state);
    src_delete(state_down);
  }

  void reset() override
  {
    onSampleRateChange();
  }

  void onSampleRateChange() override {}

  void randomize() override  {}

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "alternativeMode", json_boolean(alternativeMode));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *modeJ = json_object_get(rootJ, "alternativeMode");
      if(modeJ) {
	alternativeMode = json_boolean_value(modeJ);
      }
  }

  float in, out, gain, sym;
  float threshold = 1.0;

  bool alternativeMode = false;

  //variables for samplerate converter
  SRC_DATA data;
  SRC_DATA data_down;
  SRC_STATE* state;
  SRC_STATE* state_down;
  int error = 0;
  int frame = 0;
  float in_buffer[BUF_LEN] = {};
  float out_buffer[UPSAMPLE_RATIO*BUF_LEN] = {};
  float folded_buffer[BUF_LEN] = {};
};

float fold(float in, float threshold) {
  float out;
  if (in>threshold || in<-threshold) {
    out = clamp(fabs(fabs(fmod(in - threshold, threshold*4)) - threshold*2) - threshold, -5.0f,5.0f);
  }
  else {
    out = in;
  }
  return out;
}

float fold3(float in, float t) {
  float out = 0;
  if(in>t) {
    out = t - (in - t);
  }
  else if (in < -t) {
    out = -t + (-t - in);
  }
  else {
    out = in;
  }
  return out;
}


void Folder::step()
{
  gain = clamp(params[GAIN_PARAM].value + (inputs[GAIN_INPUT].value * params[GAIN_ATT_PARAM].value), 0.0f,14.0f);
  sym = clamp(params[SYM_PARAM].value + inputs[SYM_INPUT].value/5.0 * params[SYM_ATT_PARAM].value, -1.0f, 1.0f);
  in = (inputs[GATE_INPUT].value/5.0 + sym) * gain;

  if(++frame >= BUF_LEN) {
    //upsampling
    data.data_in = in_buffer;
    data.data_out = out_buffer;
    data.input_frames = BUF_LEN;
    data.output_frames = UPSAMPLE_RATIO*BUF_LEN;
    data.src_ratio = UPSAMPLE_RATIO;
    data.end_of_input = 0;

    src_process(state, &data);

    //fold
    for(int i=0;i<data.output_frames_gen;i++) {
      if(!alternativeMode) {
	int stages = (int)(params[STAGE_PARAM].value)*2;
	for (int y=0;y<stages;y++) {
	  out_buffer[i] = fold3(out_buffer[i], threshold);
	}
      }
      else {
	out_buffer[i] = fold(out_buffer[i], threshold);
      }
      out_buffer[i] = tanh(out_buffer[i]);
    }

    //downsampling
    data_down.data_in = out_buffer;
    data_down.data_out = folded_buffer;
    data_down.input_frames = data.output_frames_gen;
    data_down.output_frames = BUF_LEN;
    data_down.src_ratio = 1.0/UPSAMPLE_RATIO;
    data_down.end_of_input = 0;
    src_process(state_down, &data_down);
    frame = 0;
  }

  in_buffer[frame] = in;
  //out = folded_buffer[frame] * 5.0;
  outputs[GATE_OUTPUT].value = folded_buffer[frame] * 5.0;
  return;
}


struct FolderWidget : ModuleWidget
{
	SVGPanel *panel;
	FolderWidget(Folder *module);
	Menu *createContextMenu() override;
};

FolderWidget::FolderWidget(Folder *module) : ModuleWidget(module)
{
  //box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
  box.size = Vec(4 * 15, RACK_GRID_HEIGHT);

  panel = new SVGPanel();
  panel->box.size = box.size;
  panel->setBackground(SVG::load(assetPlugin(plugin, "res/Folder.svg")));
  addChild(panel);

  // addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  // addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
  // addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  // addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

  //note: SmallKnob size = 28px, Trimpot = 17 px
  addParam(ParamWidget::create<BefacoSwitch>(Vec(16, 50), module, Folder::STAGE_PARAM, 1, 3, 2));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(16, 100), module, Folder::GAIN_PARAM, 0.0, 14.0, 1.0));
  addParam(ParamWidget::create<Trimpot>(Vec(21.5, 145), module, Folder::GAIN_ATT_PARAM, -1.0, 1.0, 0));
  addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(16, 185), module, Folder::SYM_PARAM, -1.0, 1.0, 0.0));
  addParam(ParamWidget::create<Trimpot>(Vec(21.5, 230), module, Folder::SYM_ATT_PARAM, -1.0, 1.0, 0.0));


  addInput(Port::create<PJ301MPort>(Vec(3, 320), Port::INPUT, module, Folder::GATE_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(3, 276), Port::INPUT, module, Folder::GAIN_INPUT));;
  addInput(Port::create<PJ301MPort>(Vec(30, 276), Port::INPUT, module, Folder::SYM_INPUT));;
  addOutput(Port::create<PJ301MPort>(Vec(30,320), Port::OUTPUT, module, Folder::GATE_OUTPUT));
}

struct FolderMenuItem : MenuItem {
	Folder *module;
	void onAction(EventAction &e) override {
	  module->alternativeMode ^= true;
	}
	void step() override {
	  rightText = (module->alternativeMode) ? "✔" : "";
		MenuItem::step();
	}
};

Menu *FolderWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	Folder *folder = dynamic_cast<Folder*>(module);
	assert(folder);

	menu->addChild(construct<MenuEntry>());
	menu->addChild(construct<FolderMenuItem>(&FolderMenuItem::text, "Alternative Folding Algorithm", &FolderMenuItem::module, folder));

	return menu;
}

Model *modelFolder = Model::create<Folder, FolderWidget>("Aepelzens Modules", "folder", "Manifold", WAVESHAPER_TAG);
