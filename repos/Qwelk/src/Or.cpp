#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8


struct ModuleOr : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_CHANNEL,
        NUM_INPUTS = INPUT_CHANNEL + CHANNELS
    };
    enum OutputIds {
        OUTPUT_OR,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleOr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleOr::step() {
    int gate_on = 0;
    for (int i = 0; !gate_on && i < CHANNELS; ++i)
        gate_on = inputs[INPUT_CHANNEL + i].value;
    outputs[OUTPUT_OR].value = gate_on ? 10 : 0;
}


WidgetOr::WidgetOr() {
    ModuleOr *module = new ModuleOr();
    setModule(module);

    box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Or.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 32.85;
    for (int i = 0; i < CHANNELS; ++i)
        addInput(createInput<PJ301MPort>(Vec(x, ytop + ystep * i), module, ModuleOr::INPUT_CHANNEL + i));
    ytop += 9;
    addOutput(createOutput<PJ301MPort>( Vec(x, ytop + ystep * CHANNELS), module, ModuleOr::OUTPUT_OR));
}

