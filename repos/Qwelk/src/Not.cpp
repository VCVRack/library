#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8


struct ModuleNot : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SIG,
        NUM_INPUTS = INPUT_SIG + CHANNELS
    };
    enum OutputIds {
        OUTPUT_NOT,
        NUM_OUTPUTS = OUTPUT_NOT + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleNot() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleNot::step() {
    for (int i = 0; i < CHANNELS; ++i) {
        outputs[OUTPUT_NOT + i].value = inputs[INPUT_SIG + i].value != 0.0 ? 0.0 : 10.0;
    }
}


WidgetNot::WidgetNot() {
    ModuleNot *module = new ModuleNot();
    setModule(module);

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Not.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 25, ytop = 45, ystep = 39;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(createInput<PJ301MPort>(   Vec(x       , ytop + ystep * i), module, ModuleNot::INPUT_SIG  + i));
        addOutput(createOutput<PJ301MPort>( Vec(x + 26  , ytop + ystep * i), module, ModuleNot::OUTPUT_NOT + i));
    }
}

