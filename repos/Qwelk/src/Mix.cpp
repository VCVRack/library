#include "dsp/digital.hpp"
#include "qwelk.hpp"
#include "qwelk_common.h"


struct ModuleMix : Module {
    enum ParamIds {
        PARAM_GAIN_M,
        PARAM_GAIN_S,
        PARAM_GAIN_MS,
        PARAM_GAIN_L,
        PARAM_GAIN_R,
        PARAM_GAIN_LR,
        NUM_PARAMS
    };
    enum InputIds {
        IN_L,
        IN_R,
        IN_M,
        IN_S,
        IN_GAIN_M,
        IN_GAIN_S,
        IN_GAIN_L,
        IN_GAIN_R,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_M,
        OUT_S,
        OUT_L,
        OUT_R,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ModuleMix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};
static inline float max(float a, float b) {return a < b ? b : a;}
void ModuleMix::step() {
    if (inputs[IN_L].active && inputs[IN_R].active) {
        float iam = max(inputs[IN_GAIN_M].value, 0) / 10.0;
        float ias = max(inputs[IN_GAIN_S].value, 0) / 10.0;
        float ams = params[PARAM_GAIN_MS].value;
        float am = inputs[IN_GAIN_M].active ? params[PARAM_GAIN_M].value * iam : params[PARAM_GAIN_M].value;
        float as = inputs[IN_GAIN_S].active ? params[PARAM_GAIN_S].value * ias : params[PARAM_GAIN_S].value;
        float l = inputs[IN_L].value;
        float r = inputs[IN_R].value;
        float m = l + r;
        float s = l - r;
        outputs[OUT_M].value = m * ams * am;
        outputs[OUT_S].value = s * ams * as;
    }
    if (inputs[IN_M].active && inputs[IN_S].active) {
        float ial = max(inputs[IN_GAIN_L].value, 0) / 10.0;
        float iar = max(inputs[IN_GAIN_R].value, 0) / 10.0;
        float alr = params[PARAM_GAIN_LR].value;
        float al = inputs[IN_GAIN_L].active ? params[PARAM_GAIN_L].value * ial : params[PARAM_GAIN_L].value;
        float ar = inputs[IN_GAIN_R].active ? params[PARAM_GAIN_R].value * iar : params[PARAM_GAIN_R].value;
        float m = inputs[IN_M].value;
        float s = inputs[IN_S].value;
        float l = (m + s) * 0.5;
        float r = (m - s) * 0.5;
        outputs[OUT_L].value = l * alr * al;
        outputs[OUT_R].value = r * alr * ar;
    }
}

WidgetMix::WidgetMix() {
    ModuleMix *module = new ModuleMix();
    setModule(module);

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Mix.svg")));
        addChild(panel);
    }

    addChild(createScrew<ScrewSilver>(Vec(5, 0)));
    addChild(createScrew<ScrewSilver>(Vec(5, 365)));

    float x = box.size.x / 2.0 - 27;
    
    addInput(createInput<PJ301MPort>(Vec(x     ,   25), module, ModuleMix::IN_L));
    addInput(createInput<PJ301MPort>(Vec(x + 30,   25), module, ModuleMix::IN_R));
    
    addParam(createParam<RoundSmallBlackKnob>(Vec(x + 28,  55), module, ModuleMix::PARAM_GAIN_MS, 0.0, 1.0, 1.0));

    addParam(createParam<TinyKnob>(Vec(x    , 90), module, ModuleMix::PARAM_GAIN_M, 0.0, 1.0, 1.0));
    addInput(createInput<PJ301MPort>(Vec(x + 30  , 88), module, ModuleMix::IN_GAIN_M));
    addOutput(createOutput<PJ301MPort>(Vec(x + 30, 113), module, ModuleMix::OUT_M));

    addParam(createParam<TinyKnob>(Vec(x    , 147), module, ModuleMix::PARAM_GAIN_S, 0.0, 1.0, 1.0));
    addInput(createInput<PJ301MPort>(Vec(x + 30  , 145), module, ModuleMix::IN_GAIN_S));
    addOutput(createOutput<PJ301MPort>(Vec(x + 30, 169), module, ModuleMix::OUT_S));

    addInput(createInput<PJ301MPort>(Vec(x     , 210), module, ModuleMix::IN_M));
    addInput(createInput<PJ301MPort>(Vec(x + 30, 210), module, ModuleMix::IN_S));

    addParam(createParam<RoundSmallBlackKnob>(Vec(x + 28,  240), module, ModuleMix::PARAM_GAIN_LR, 0.0, 1.0, 1.0));

    addParam(createParam<TinyKnob>(Vec(x    , 275), module, ModuleMix::PARAM_GAIN_L, 0.0, 1.0, 1.0));
    addInput(createInput<PJ301MPort>(Vec(x + 30  , 273), module, ModuleMix::IN_GAIN_L));
    addOutput(createOutput<PJ301MPort>(Vec(x + 30, 298), module, ModuleMix::OUT_L));

    addParam(createParam<TinyKnob>(Vec(x    , 332), module, ModuleMix::PARAM_GAIN_R, 0.0, 1.0, 1.0));
    addInput(createInput<PJ301MPort>(Vec(x + 30  , 330), module, ModuleMix::IN_GAIN_R));
    addOutput(createOutput<PJ301MPort>(Vec(x + 30, 355), module, ModuleMix::OUT_R));
}
