#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 3


struct ModuleXor : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_A,
        INPUT_B     = INPUT_A + CHANNELS,
        NUM_INPUTS  = INPUT_B + CHANNELS
    };
    enum OutputIds {
        OUTPUT_XOR,
        NUM_OUTPUTS = OUTPUT_XOR + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleXor() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleXor::step() {
    for (int i = 0; i < CHANNELS; ++i)
        outputs[OUTPUT_XOR + i].value = inputs[INPUT_A + i].value == inputs[INPUT_B + i].value ? 0.0 : 10.0;
}


WidgetXor::WidgetXor() {
    ModuleXor *module = new ModuleXor();
    setModule(module);

    box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Xor.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, ytop = 45, ystep = 37.5;
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(createInput<PJ301MPort>(   Vec(x, ytop + ystep * i), module, ModuleXor::INPUT_A + i));
        addInput(createInput<PJ301MPort>(   Vec(x, ytop + ystep*1 + ystep * i), module, ModuleXor::INPUT_B + i));
        addOutput(createOutput<PJ301MPort>( Vec(x, ytop + ystep*2 + ystep  * i), module, ModuleXor::OUTPUT_XOR + i));
        ytop += 3 * ystep - 42.5;
    }
}

