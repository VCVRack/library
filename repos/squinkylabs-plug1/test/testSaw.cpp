#include <assert.h>
#include <iostream>
#include "SawOscillator.h"

using namespace std;

// do objects exist?
template<typename T>
void testSaw1()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, false>::setFrequency(params, (T(.1)));
    SawOscillatorState<T> state;
    SawOscillator<T, false>::run(state, params);
}

/**
 * Does parameter calculation do anything?
 */
template<typename T>
void testSaw2()
{
    SawOscillatorParams<T> params;
    assert(params.phaseIncrement == 0);
    SawOscillator<T, false>::setFrequency(params, (T(.1)));
    assert(params.phaseIncrement > 0);
}

/**
 * Does something come out?
 */
template<typename T>
void testSaw3()
{
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.2)));
    SawOscillatorState<T> state;
    SawOscillator<T, true>::run(state, params);
    const T out = SawOscillator<T, true>::run(state, params);
    assert(out >= 0);
    assert(out < 1);
}


/**
 * Does it look like a saw?
 */
template<typename T>
void testSaw4()
{
    const T freq = T(.01);
    const T freq_2 = freq / 2;
    const T delta = freq / 1000;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.01)));
    SawOscillatorState<T> state;

    T last = 0;
    for (int i = 0; i < 1000; ++i) {
        const T output = SawOscillator<T, true>::run(state, params);

        assert(output >= 0);
        assert(output < 1);

        if (output < last) {
            assert(last > .99);
            assert(output < .01);
        } else {
            assert(output < (last + freq + delta));
        }

        last = output;
    }
}

/**
* is the quadrature really 90 out of phase?
*/
template<typename T>
void testSaw5()
{
    const T freq = T(.01);
    const T freq_2 = freq / 2;
    const T delta = freq / 1000;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, (T(.01)));
    SawOscillatorState<T> state;

    T output;
    T quadratureOutput;
    for (int i = 0; i < 1000; ++i) {
        SawOscillator<T, true>::runQuadrature(output, quadratureOutput, state, params);

        // normalize output (unwrap)
        if (quadratureOutput < output) {
            quadratureOutput += 1;
        }
        assert(quadratureOutput = (output + T(.25)));
    }
}


/**
* Does it look like a negative saw?
*/
template<typename T>
void testSaw6()
{
    const T freq = T(-.01);
    const T delta = freq / 1000;
    SawOscillatorParams<T> params;
    SawOscillator<T, true>::setFrequency(params, freq);
    SawOscillatorState<T> state;

    T last = 0;
    for (int i = 0; i < 1000; ++i) {
        const T output = SawOscillator<T, true>::run(state, params);

        assert(output >= 0);
        assert(output < 1);

        if (output > last) {
            // wrap case - did we more or less wrap?
            assert(last < .01);
            assert(output > .98);
        } else {
            // no-wrap - are we decreasing
            assert(output > (last + freq + delta));
        }

        last = output;
    }
}



template <typename T>
void testSawT()
{
    testSaw1<T>();
    testSaw2<T>();
    testSaw3<T>();
    testSaw4<T>();
    testSaw5<T>();
    testSaw6<T>();
}

void testSaw()
{
    testSawT<float>();
    testSawT<double>();
}