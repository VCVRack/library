#pragma once

#include <memory>
#include <emmintrin.h>
#include <functional>

template <typename T> class LookupTableParams;
/* Lookup table with evenly spaced lookup "bins"
 * Uses linear interpolation
 */

 // TODO: templatize on size?
template <typename T>
class LookupTable
{

public:
    static void init(LookupTableParams<T>& params, int bins, T xMin, T xMax, std::function<double(double)> f);
    static T lookup(const LookupTableParams<T>& params, T input);

private:
    static int cvtt(T *);

#ifdef _DEBUG
    static void checkInput(const LookupTableParams<T>& params, const T *in, int sampleFrames)
    {
        for (int i = 0; i < sampleFrames; ++i) {
            f_t input = in[i];
            assert(input >= params.xMin && input <= params.xMax);
        }
    }
#else
#define checkInput __noop
#endif

};

template<typename T>
inline T LookupTable<T>::lookup(const LookupTableParams<T>& params, T input)
{
    assert(params.isValid());
    assert(input >= params.xMin && input <= params.xMax);

    // need to scale by bins
    T scaledInput = input * params.a + params.b;
    assert(params.a != 0);

    int input_int = cvtt(&scaledInput);
    T input_float = scaledInput - input_int;

    // Unfortunately, when we use float instead of doubles,
    // numeric precision issues get us "out of range". So
    // here we clamp to range. It would be more efficient if we didn't do this.
    // Perhaps the calculation of a and b could be done so this can't happen?
    if (input_float < 0) {
        input_float = 0;
    }
    else if (input_float > 1) {
        input_float = 1;
    }
    
    assert(input_float >= 0 && input_float <= 1);
    assert(input_int >= 0 && input_int <= params.numBins_i);

    T * entry = params.entries + (2 * input_int);
    T x = entry[0];
    x += input_float * entry[1];

    return x;
}

template<typename T>
inline void LookupTable<T>::init(LookupTableParams<T>& params,
    int bins, T x0In, T x1In, std::function<double(double)> f)
{
    params.alloc(bins);

    // f(x0 = ax + 0 == index
    // f(x0) = 0
    // f(x1) = bins
    params.a = (T) bins / (x1In - x0In);
    params.b = -params.a * x0In;

    if (x0In == 0) assert(params.b == 0);
    assert((params.a * x0In + params.b) == 0);
    assert((params.a * x1In + params.b) == bins);

    for (int i = 0; i <= bins; ++i) {
        double x0 = (i - params.b) / params.a;
        double x1 = ((i + 1) - params.b) / params.a;

        double y0 = f(x0);
        double y1 = f(x1);
        double slope = y1 - y0;
        T * entry = params.entries + (2 * i);
        entry[0] = (T) y0;
        entry[1] = (T) slope;
#if 0
        {
            char buf[512];
            sprintf(buf, "fill index[%d], x=%f value=%f slope=%f\n", i, x0, params.values[i], slope);
            DebugUtil::trace(buf);
        }
#endif
    }
    params.xMin = x0In;
    params.xMax = x1In;
}


template<>
inline int LookupTable<double>::cvtt(double* input)
{
    auto x = _mm_load_sd(input);
    return _mm_cvttsd_si32(x);
}

template<>
inline int LookupTable<float>::cvtt(float* input)
{
    auto x = _mm_load_ss(input);
    return _mm_cvttss_si32(x);
}

template <typename T>
class LookupTableParams
{
public:
    int numBins_i = 0;		// numBins will be number of entries (pairs of values)
    T a = 0, b = 0;			// lookup index = a * x + b, so domain of x can be whatever we want

    T * entries = 0;		// each entry is value, slope
    T xMin = 0;				// minimum x value we will accept as input
    T xMax = 0;				// max x value we will accept as input

    LookupTableParams()
    {
    }
    LookupTableParams(const LookupTableParams&) = delete;
    LookupTableParams& operator=(const LookupTableParams&) = delete;

    ~LookupTableParams()
    {
        free(entries);
    }

    bool isValid() const
    {
        return ((entries != 0) &&
            (xMax > xMin) &&
            (numBins_i > 0)
            );
    }

    void alloc(int bins)
    {
        if (entries) free(entries);
        // allocate one extra, so we can index all the way to the end...
        entries = (T *) malloc((bins + 1) * 2 * sizeof(T));
        numBins_i = bins;
        a = 0;
        b = 0;
    }
};



