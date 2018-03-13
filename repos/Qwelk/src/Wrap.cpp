#include "dsp/digital.hpp"
#include "qwelk.hpp"

#define CHANNELS 8


struct ModuleWrap : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        IN_WRAP,
        IN_SIG,
        NUM_INPUTS = IN_SIG + CHANNELS
    };
    enum OutputIds {
        OUT_WRAPPED,
        NUM_OUTPUTS = OUT_WRAPPED + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    int _wrap = -10;
    
    ModuleWrap() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleWrap::step() {
    int wrap = (/*clampf(*/inputs[IN_WRAP].value/*, -5.0, 5.0)*/ / 5.0) * (CHANNELS - 1);

    for (int i = 0; i < CHANNELS; ++i) {
        int w = i;
        if (wrap > 0)
            w = (i + wrap) % CHANNELS;
        else if (wrap < 0)
            w = (i + CHANNELS - wrap) % CHANNELS;
        outputs[OUT_WRAPPED + i].value = inputs[IN_SIG + w].value;
    }
}

WidgetWrap::WidgetWrap() {
    ModuleWrap *module = new ModuleWrap();
    setModule(module);

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Wrap.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 25, ytop = 60, ystep = 39;

    addInput(createInput<PJ301MPort>(Vec(17.5, 30), module, ModuleWrap::IN_WRAP));
    
    for (int i = 0; i < CHANNELS; ++i) {
        addInput(createInput<PJ301MPort>(   Vec(x       , ytop + ystep * i), module, ModuleWrap::IN_SIG  + i));
        addOutput(createOutput<PJ301MPort>( Vec(x + 26  , ytop + ystep * i), module, ModuleWrap::OUT_WRAPPED + i));
    }
}

