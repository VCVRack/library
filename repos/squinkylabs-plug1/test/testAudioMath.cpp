
#include <assert.h>
#include <iostream>
#include "AudioMath.h"

using namespace std;

static void test0()
{
    assert(AudioMath::closeTo(0, 0, .000001));
    assert(AudioMath::closeTo(1000, 1001, 1.1));
    assert(!AudioMath::closeTo(1000, 1010, 1.1));
    assert(!AudioMath::closeTo(1010, 1000, 1.1));
}

static void test1()
{
    assert(AudioMath::closeTo(3.145, AudioMath::Pi, .1));
    assert(AudioMath::closeTo(3.145 / 2, AudioMath::Pi_2, .1));
    assert(AudioMath::closeTo(log(10), AudioMath::Ln10, .001));
}

static void test2()
{
    const double d = .0001;
    std::function<double(double)> f = AudioMath::makeFunc_Sin();
    assert(AudioMath::closeTo(f(0), 0, d));
    assert(AudioMath::closeTo(f(.5), 0, d));
    assert(AudioMath::closeTo(f(.25), 1, d));
    assert(AudioMath::closeTo(f(.75), -1, d));
    assert(AudioMath::closeTo(f(.125), 1.0 / sqrt(2), d));

}

static void test3()
{
    const double d = .0001;
    std::function<double(double)> f = AudioMath::makeFunc_Exp(0, 4, 2, 32);

    assert(AudioMath::closeTo(f(0), 2, d));
    assert(AudioMath::closeTo(f(1), 4, d));
    assert(AudioMath::closeTo(f(2), 8, d));
    assert(AudioMath::closeTo(f(3), 16, d));
    assert(AudioMath::closeTo(f(4), 32, d));
    assert(f(5) > 33);
    assert(f(-1) < 1.5);
}

void testAudioMath()
{
    test0();
    test1();
    test2();
    test3();
}