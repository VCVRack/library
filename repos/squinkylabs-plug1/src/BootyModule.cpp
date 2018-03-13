
#include "Squinky.hpp"
#include "FrequencyShifter.h"
#include "WidgetComposite.h"
#if 0
#include "SinOscillator.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "HilbertFilterDesigner.h"
#endif

/**
 * Implementation class for BootyWidget
 */
struct BootyModule : Module
{
    enum ParamIds
    {
        PITCH_PARAM,      // the big pitch knob
        NUM_PARAMS
    };

    enum InputIds
    {
        AUDIO_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        SIN_OUTPUT,
        COS_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    BootyModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
    void onSampleRateChange() override;

    FrequencyShifter<WidgetComposite> shifter;
private:
  typedef float T;
#if 1
  //  WidgetComposite wc;
  //  FrequencyShifter<WidgetComposite> shifter;
#else
  
    SinOscillatorParams<T> oscParams;
    SinOscillatorState<T> oscState;
    BiquadParams<T, 3> hilbertFilterParamsSin;
    BiquadParams<T, 3> hilbertFilterParamsCos;
    BiquadState<T, 3> hilbertFilterStateSin;
    BiquadState<T, 3> hilbertFilterStateCos;

    LookupTableParams<T> exponential;

    float reciprocolSampleRate;
#endif
public:
   // float freqRange = 5;
    ChoiceButton *rangeChoice;
};

extern float values[];
extern const char* ranges[];

BootyModule::BootyModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS),
    shifter(this)
{
    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();

    shifter.init();
#if 0
    // Force lazy load of lookup table with this extra call
    
    SinOscillator<T, true>::setFrequency(oscParams, .01);
   
    // Make 128 entry table to do exp x={-5..5} y={2..2000}
    std::function<double(double)> expFunc = AudioMath::makeFunc_Exp(-5, 5, 2, 2000);
    LookupTable<T>::init(exponential, 128, -5, 5, expFunc);
#endif
}

void BootyModule::onSampleRateChange()
{
    T rate = engineGetSampleRate();
    #if 0
    reciprocolSampleRate = 1 / rate;
    HilbertFilterDesigner<T>::design(rate, hilbertFilterParamsSin, hilbertFilterParamsCos);
    #endif
    shifter.setSampleRate(rate);
}

json_t *BootyModule::toJson()
{
    json_t *rootJ = json_object();
  //  int rg = freqRange;
    const int rg = shifter.freqRange;
    json_object_set_new(rootJ, "range", json_integer(rg));
    return rootJ;
}

void BootyModule::fromJson(json_t *rootJ)
{

    json_t *driverJ = json_object_get(rootJ, "range");
    if (driverJ) {
        const int rg = json_number_value(driverJ);

        // TODO: should we be more robust about float <> int issues?
        //need to tell the control what text to display
        for (int i = 0; i < 5; ++i) {
            if (rg == values[i]) {
                rangeChoice->text = ranges[i];
            }
        }
        shifter.freqRange = rg;
        fflush(stdout);
    }
}

void BootyModule::step()
{
    shifter.step();
}
#if 0
void BootyModule::step()
{
    // add the knob and the CV
    T freqHz;
    T cvTotal = params[PITCH_PARAM].value + inputs[CV_INPUT].value;
    if (cvTotal > 5) {
        cvTotal = 5;
    }
    if (cvTotal < -5) {
        cvTotal = -5;
    }
    if (freqRange > .2) {
        cvTotal *= freqRange;
        cvTotal *= 1. / 5.;
        freqHz = cvTotal;
    } else {
        freqHz = LookupTable<T>::lookup(exponential, cvTotal);
    }

    SinOscillator<float, true>::setFrequency(oscParams, freqHz * reciprocolSampleRate);

    // Generate the quadrature sin oscillators.
    T x, y;
    SinOscillator<T, true>::runQuadrature(x, y, oscState, oscParams);

    // Filter the input through th quadrature filter
    const T input = inputs[AUDIO_INPUT].value;
    const T hilbertSin = BiquadFilter<T>::run(input, hilbertFilterStateSin, hilbertFilterParamsSin);
    const T hilbertCos = BiquadFilter<T>::run(input, hilbertFilterStateCos, hilbertFilterParamsCos);

    // Cross modulate the two sections.
    x *= hilbertSin;
    y *= hilbertCos;

    // And combine for final SSB output.
    outputs[SIN_OUTPUT].value = x + y;
    outputs[COS_OUTPUT].value = x - y;
}
#endif

/***********************************************************************************
 *
 * RangeChoice selector widget
 *
 ***********************************************************************************/

const char* ranges[5] = {
    "5 Hz",
    "50 Hz",
    "500 Hz",
    "5 kHz",
    "exp"
};

float values[5] = {
    5,
    50,
    500,
    5000,
    0
};

struct RangeItem : MenuItem
{
    RangeItem(int index, float * output, ChoiceButton * inParent) :
        rangeIndex(index), rangeOut(output), rangeChoice(inParent)
    {
        text = ranges[index];
    }

    const int rangeIndex;
    float * const rangeOut;
    ChoiceButton* const rangeChoice;

    void onAction(EventAction &e) override
    {
        rangeChoice->text = ranges[rangeIndex];
        *rangeOut = values[rangeIndex];
    }
};

struct RangeChoice : ChoiceButton
{

    RangeChoice(float * out, const Vec& pos, float width) : output(out)
    {
        assert(*output == 5);
        this->text = std::string(ranges[0]);
        this->box.pos = pos;
        this->box.size.x = width;
    }
    float * const output;
    void onAction(EventAction &e) override
    {
        Menu *menu = gScene->createMenu();

        menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
        menu->box.size.x = box.size.x;
        {
            menu->addChild(new RangeItem(0, output, this));
            menu->addChild(new RangeItem(1, output, this));
            menu->addChild(new RangeItem(2, output, this));
            menu->addChild(new RangeItem(3, output, this));
            menu->addChild(new RangeItem(4, output, this));
        }
    }
};

////////////////////
// module widget
////////////////////


struct BootyWidget : ModuleWidget
{
    BootyWidget(BootyModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
BootyWidget::BootyWidget(BootyModule *module) : ModuleWidget(module)
{
    // BootyModule *module = new BootyModule();
 //	setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/booty_panel.svg")));
        addChild(panel);
    }

    const int leftInputX = 11;
    const int rightInputX = 55;

    const int row0 = 45;
    const int row1 = 102;
    static int row2 = 186;

    // Inputs on Row 0
    addInput(Port::create<PJ301MPort>(Vec(leftInputX, row0), Port::INPUT, module, BootyModule::AUDIO_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(rightInputX, row0), Port::INPUT, module, BootyModule::CV_INPUT));

    // shift Range on row 2
    const float margin = 16;
    float xPos = margin;
    float width = 6 * RACK_GRID_WIDTH - 2 * margin;

    // TODO: why do we need to reach into the module from here? I guess any
    // time UI callbacks need to go bak..
    module->rangeChoice = new RangeChoice(&module->shifter.freqRange, Vec(xPos, row2), width);
    addChild(module->rangeChoice);

    // knob on row 1
    addParam(ParamWidget::create<Rogan3PSBlue>(Vec(18, row1), module, BootyModule::PITCH_PARAM, -5.0, 5.0, 0.0));

    const float row3 = 317.5;

    // Outputs on row 3
    const float leftOutputX = 9.5;
    const float rightOutputX = 55.5;

    addOutput(Port::create<PJ301MPort>(Vec(leftOutputX, row3), Port::OUTPUT, module, BootyModule::SIN_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(rightOutputX, row3), Port::OUTPUT, module, BootyModule::COS_OUTPUT));

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelBootyModule = Model::create<BootyModule, BootyWidget>("Squinky Labs",
    "squinkylabs-freqshifter",
    "Booty Frequency Shifter", EFFECT_TAG, RING_MODULATOR_TAG);

