#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 2


struct ModuleXFade : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_A,
        INPUT_B     = INPUT_A + CHANNELS,
        INPUT_X     = INPUT_B + CHANNELS,
        NUM_INPUTS  = INPUT_X + CHANNELS
    };
    enum OutputIds {
        OUTPUT_BLEND,
        NUM_OUTPUTS = OUTPUT_BLEND + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleXFade() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleXFade::step() {
    for (int i = 0; i < CHANNELS; ++i) {
        float blend = inputs[INPUT_X + i].value / 10.0;
        outputs[OUTPUT_BLEND + i].value = (1.0 - blend) * inputs[INPUT_A + i].value + inputs[INPUT_B + i].value * blend;
    }
}


WidgetXFade::WidgetXFade() {
    ModuleXFade *module = new ModuleXFade();
    setModule(module);

    box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/XFade.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 37.5;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(createInput<PJ301MPort>(   Vec(x, ytop + ystep * i), module, ModuleXFade::INPUT_A + i));
        addInput(createInput<PJ301MPort>(   Vec(x, ytop + ystep*1 + ystep * i), module, ModuleXFade::INPUT_B + i));
        addInput(createInput<PJ301MPort>(   Vec(x, ytop + ystep*2 + ystep * i), module, ModuleXFade::INPUT_X + i));
        addOutput(createOutput<PJ301MPort>( Vec(x, ytop + ystep*3 + ystep  * i), module, ModuleXFade::OUTPUT_BLEND + i));
        ytop += 4 * ystep - 17.5;
    }
}

