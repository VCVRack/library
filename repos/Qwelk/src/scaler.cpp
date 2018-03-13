#include "dsp/digital.hpp"
#include "qwelk.hpp"


struct ModuleScaler : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SUB_5,
        INPUT_MUL_2,
        INPUT_DIV_2,
        INPUT_ADD_5,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT_SUB_5,
        OUTPUT_MUL_2,
        OUTPUT_DIV_2,
        OUTPUT_ADD_5,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleScaler() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleScaler::step() {
    outputs[OUTPUT_SUB_5].value = inputs[INPUT_SUB_5].value - 5.0;
    outputs[OUTPUT_MUL_2].value = inputs[INPUT_MUL_2].normalize(outputs[OUTPUT_SUB_5].value) * 2.0;
    outputs[OUTPUT_DIV_2].value = inputs[INPUT_DIV_2].normalize(outputs[OUTPUT_MUL_2].value) * 0.5;
    outputs[OUTPUT_ADD_5].value = inputs[INPUT_ADD_5].normalize(outputs[OUTPUT_DIV_2].value) + 5.0;
}


WidgetScaler::WidgetScaler() {
    ModuleScaler *module = new ModuleScaler();
    setModule(module);

    box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Scaler.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));

    float x = box.size.x / 2.0 - 12, y = 0, ytop = 30, ystep = 30, mstep = 16;
    addInput(createInput<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), module, ModuleScaler::INPUT_SUB_5));
    addOutput(createOutput<PJ301MPort>( Vec(x, ytop + (y+=ystep)), module, ModuleScaler::OUTPUT_SUB_5));
    ytop += mstep;
    addInput(createInput<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), module, ModuleScaler::INPUT_MUL_2));
    addOutput(createOutput<PJ301MPort>( Vec(x, ytop + (y+=ystep)), module, ModuleScaler::OUTPUT_MUL_2));
    ytop += mstep;
    addInput(createInput<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), module, ModuleScaler::INPUT_DIV_2));
    addOutput(createOutput<PJ301MPort>( Vec(x, ytop + (y+=ystep)), module, ModuleScaler::OUTPUT_DIV_2));
    ytop += mstep;
    addInput(createInput<PJ301MPort>(   Vec(x, ytop + (y+=ystep)), module, ModuleScaler::INPUT_ADD_5));
    addOutput(createOutput<PJ301MPort>( Vec(x, ytop + (y+=ystep)), module, ModuleScaler::OUTPUT_ADD_5));
}
