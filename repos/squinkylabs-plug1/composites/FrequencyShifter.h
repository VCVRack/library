#pragma once

#include "LookupTable.h"
#include "SinOscillator.h"
#include "BiquadFilter.h"
#include "BiquadParams.h"
#include "BiquadState.h"
#include "HilbertFilterDesigner.h"

/**
 * Complete Frequency Shifter
 *
 * if TBase is WidgetComposite, used as the implementation part of the Booty Shifter module.
 * If TBase is TestComposite build stand alone for unit tests, 
 */
template <class TBase>
class FrequencyShifter : public TBase
{
public:
    FrequencyShifter(struct Module * module) : TBase(module)
    {
    }
    FrequencyShifter() : TBase()
    {
    }
    void setSampleRate(float rate)
    {
        reciprocolSampleRate = 1 / rate;
        HilbertFilterDesigner<T>::design(rate, hilbertFilterParamsSin, hilbertFilterParamsCos);
    }

    // must be called after setSampleRate
    // TODO: can't we combine these?
    void init()
    {
        // Force lazy load of lookup table with this extra call

        SinOscillator<T, true>::setFrequency(oscParams, T(.01));

        // Make 128 entry table to do exp x={-5..5} y={2..2000}
        std::function<double(double)> expFunc = AudioMath::makeFunc_Exp(-5, 5, 2, 2000);
        LookupTable<T>::init(exponential, 128, -5, 5, expFunc);
    }

    // DO these belong here?
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
    /**
     * Main processing entry point. Called every sample
     */
    void step();

    typedef float T;
    T freqRange = 5; // the freq range switch
private:
    SinOscillatorParams<T> oscParams;
    SinOscillatorState<T> oscState;
    BiquadParams<T, 3> hilbertFilterParamsSin;
    BiquadParams<T, 3> hilbertFilterParamsCos;
    BiquadState<T, 3> hilbertFilterStateSin;
    BiquadState<T, 3> hilbertFilterStateCos;

    LookupTableParams<T> exponential;

    float reciprocolSampleRate;
};

template <class TBase>
inline void FrequencyShifter<TBase>::step()
{
    assert(exponential.isValid());
   
    // add the knob and the CV
    T freqHz;
    T cvTotal = TBase::params[PITCH_PARAM].value + TBase::inputs[CV_INPUT].value;
    if (cvTotal > 5) {
        cvTotal = 5;
    }
    if (cvTotal < -5) {
        cvTotal = -5;
    }
    if (freqRange > .2) {
        cvTotal *= freqRange;
        cvTotal *= T(1. / 5.);
        freqHz = cvTotal;
    } else {
        freqHz = LookupTable<T>::lookup(exponential, cvTotal);
    }

    SinOscillator<float, true>::setFrequency(oscParams, freqHz * reciprocolSampleRate);

    // Generate the quadrature sin oscillators.
    T x, y;
    SinOscillator<T, true>::runQuadrature(x, y, oscState, oscParams);

    // Filter the input through th quadrature filter
    const T input = TBase::inputs[AUDIO_INPUT].value;
    const T hilbertSin = BiquadFilter<T>::run(input, hilbertFilterStateSin, hilbertFilterParamsSin);
    const T hilbertCos = BiquadFilter<T>::run(input, hilbertFilterStateCos, hilbertFilterParamsCos);

    // Cross modulate the two sections.
    x *= hilbertSin;
    y *= hilbertCos;

    // And combine for final SSB output.
    TBase::outputs[SIN_OUTPUT].value = x + y;
    TBase::outputs[COS_OUTPUT].value = x - y;
}
