#include "aepelzen.hpp"
#include "dsp/digital.hpp"
#include <math.h>

#define NUM_CHANNELS 4

struct Erwin : Module {
  enum ParamIds {
    CHANNEL_TRANSPOSE_PARAM,
    NOTE_PARAM = CHANNEL_TRANSPOSE_PARAM + NUM_CHANNELS,
    NUM_PARAMS = NOTE_PARAM + 12
  };
  enum InputIds {
    TRANSPOSE_INPUT,
    SEMI_INPUT,
    IN_INPUT,
    NUM_INPUTS = IN_INPUT + 4
  };
  enum OutputIds {
    OUT_OUTPUT,
    NUM_OUTPUTS = OUT_OUTPUT + 4
  };
  enum LightIds {
    NOTE_LIGHT,
    NUM_LIGHTS = NOTE_LIGHT + 12
  };

  Erwin() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { reset(); };
  void step() override;
  json_t* toJson() override;
  void fromJson(json_t *rootJ) override;

  //float ratios[13] = {1.0, 16.0/15, 9.0/8, 6.0/5, 5.0/4, 4.0/3, 7.0/5, 3.0/2, 8.0/5, 5.0/3, 16.0/9, 15.0/8, 2.0};

  bool noteState[12] = {};
  int octave;
  int transposeOctave = 0;
  int transposeSemi = 0;
  float freq;

  SchmittTrigger noteTriggers[12];
};

json_t* Erwin::toJson() {
  json_t *rootJ = json_object();

  // Note values
  json_t *gatesJ = json_array();
  for (int i = 0; i < 12; i++) {
    json_t *gateJ = json_boolean(noteState[i]);
    json_array_append_new(gatesJ, gateJ);
  }
  json_object_set_new(rootJ, "notes", gatesJ);
  return rootJ;
}

void Erwin::fromJson(json_t *rootJ) {
  // Note values
  json_t *gatesJ = json_object_get(rootJ, "notes");
  for (int i = 0; i < 12; i++) {
    json_t *gateJ = json_array_get(gatesJ, i);
    noteState[i] = !!json_boolean_value(gateJ);
  }
}

void Erwin::step() {

  for(int y=0;y<NUM_CHANNELS;y++) {
    octave = trunc(inputs[IN_INPUT+y].value);
    freq = inputs[IN_INPUT+y].value - octave;
    //limit to 4 octaves
    transposeOctave = clampi((int)round(inputs[TRANSPOSE_INPUT].value / 3.0) + (int)round(params[CHANNEL_TRANSPOSE_PARAM + y].value),-4, 4);
    //limit to 1 octave
    transposeSemi = (int)round(inputs[SEMI_INPUT].value * 1.2);

    int semi = (int)(round(freq * 12));
    //find last matching note in scale
    int lastValidIndex = 0;
    for(int i=semi;i>=0;i--) {
      if(noteState[i]) {
	lastValidIndex = i;
	break;
      }
    }

    if(transposeSemi) {
      lastValidIndex = (lastValidIndex + transposeSemi);
    }

    outputs[OUT_OUTPUT + y].value = octave + lastValidIndex * 1/12.0 + transposeOctave;
  }

  // Note buttons
  for (int i = 0; i < 12; i++) {
    if (noteTriggers[i].process(params[NOTE_PARAM + i].value)) {
      noteState[i] = !noteState[i];
    }
    lights[NOTE_LIGHT + i].value = (noteState[i] >= 1.0) ? 0.7 : 0;
  }
}

template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
	  this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct ErwinWidget : ModuleWidget {
	ErwinWidget(Erwin *module);
};

ErwinWidget::ErwinWidget(Erwin *module) : ModuleWidget(module) {
  box.size = Vec(15*8, 380);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Erwin.svg")));
    addChild(panel);
  }

  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

  addInput(Port::create<PJ301MPort>(Vec(22.5, 42), Port::INPUT, module, Erwin::TRANSPOSE_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(76, 42), Port::INPUT, module, Erwin::SEMI_INPUT));

  for(int i=0;i<4;i++) {
    addOutput(Port::create<PJ301MPort>(Vec(76, 235 + i*30), Port::OUTPUT, module, Erwin::OUT_OUTPUT + i));
    addInput(Port::create<PJ301MPort>(Vec(76, 100 +i*30), Port::INPUT, module, Erwin::IN_INPUT + i));
  }

  addParam(ParamWidget::create<Trimpot>(Vec(16, 90), module, Erwin::CHANNEL_TRANSPOSE_PARAM, -4, 4, 0));
  addParam(ParamWidget::create<Trimpot>(Vec(38, 90), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 1, -4, 4, 0));
  addParam(ParamWidget::create<Trimpot>(Vec(16, 117.5), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 2, -4, 4, 0));
  addParam(ParamWidget::create<Trimpot>(Vec(38, 117.5), module, Erwin::CHANNEL_TRANSPOSE_PARAM + 3, -4, 4, 0));

  //Note buttons
  int white=0;
  int black = 0;
  for(int i=0; i<12; i++) {
    if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10 ) {
      addParam(ParamWidget::create<LEDBezel>(Vec(10,311.5 - black*30), module, Erwin::NOTE_PARAM + i, 0.0, 1.0, 0.0));
      addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(12, 313.5 - black*30), module, Erwin::NOTE_LIGHT+i));
      black++;
    }
    else {
      if(i == 4)
	black++;
      addParam(ParamWidget::create<LEDBezel>(Vec(35,326.5 - white*30), module, Erwin::NOTE_PARAM + i, 0.0, 1.0, 0.0));
      addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(Vec(37, 328.5 - white*30), module, Erwin::NOTE_LIGHT+i));
      white++;
    }
  }
}

Model *modelErwin = Model::create<Erwin, ErwinWidget>("Aepelzens Modules", "Erwin", "Erwin", UTILITY_TAG);
