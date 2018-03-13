#include <functional>
#include <time.h>
#include <cmath>
#include <limits>

#include "HilbertFilterDesigner.h"
#include "AudioMath.h"
#include "BiquadParams.h"
#include "BiquadFilter.h"
#include "BiquadState.h"
#include "FrequencyShifter.h"
#include "TestComposite.h"

using Shifter = FrequencyShifter<TestComposite>;

#include "MeasureTime.h"

static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;

    MeasureTime<float>::run("test1 null", [&d, scale]() {
        return TestBuffers<float>::get();
     }, 1);

    MeasureTime<float>::run("test1 sin", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<double>::run("test1 sin double", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run("test1 sinx2", []() {
        float x = std::sin(TestBuffers<float>::get());
        x = std::sin(x);
        return x;
        }, 1);

    MeasureTime<float>::run("mult", []() {
        float x = TestBuffers<float>::get(); 
        float y = TestBuffers<float>::get();
        return x * y;
        }, 1);

    MeasureTime<float>::run("div", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x / y;
        }, 1);


}

template <typename T>
static void testHilbert()
{
    BiquadParams<T, 3> paramsSin;
    BiquadParams<T, 3> paramsCos;
    BiquadState<T, 3> state;
    HilbertFilterDesigner<T>::design(44100, paramsSin, paramsCos);

    MeasureTime<T>::run("hilbert", [&state, &paramsSin]() {

        T d = BiquadFilter<T>::run(TestBuffers<T>::get(), state, paramsSin);
        return d;
        }, 1);

}


void testShifter()
{
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();

    fs.inputs[Shifter::AUDIO_INPUT].value = 0;

    MeasureTime<float>::run("shifter", [&fs]() {
        fs.inputs[Shifter::AUDIO_INPUT].value = TestBuffers<float>::get();
        fs.step();
        return fs.outputs[Shifter::SIN_OUTPUT].value;
        }, 1);

}

void perfTest()
{
    test1();
    testHilbert<float>();
    testHilbert<double>();
    testShifter();
}