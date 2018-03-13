#pragma once

#include <functional>
#include "TimeStatsCollector.h"

template <typename T>
class TestBuffers;


/**
 * Used to estimate the amount of time a function takes to execute.
 * Will run the function over and over in a tight loop. Return value of function
 * is save to testBuffers. Otherwise the compiler might optimize the whole thing away.
 * Usually ends by printing out the data.
 */
template <typename T>
class MeasureTime
{
public:

    /**
     * Executes function "func" and measures how long it takes.
     * Will call func in a tight look lasting minTime seconds.
     * When done, prints out statistics.
     */
    static void run(const char * name, std::function<T()> func, float minTime)
    {
        int64_t iterations;
        bool done = false;

        //keep increasing the number of iterations until we last at least minTime seconds
        for (iterations = 100; !done; iterations *= 2) {
            double elapsed = measureTimeSub(func, iterations);
            if (elapsed >= minTime) {
                double itersPerSec = iterations / elapsed;
                double full = 44100;
                double percent = full * 100 / itersPerSec;
                printf("\nmeasure %s over time %f\n", name, minTime);
                printf("did %lld iterations in %f seconds\n", iterations, elapsed);
                printf("that's %f per sec\n", itersPerSec);
                printf("percent CPU usage: %f\n", percent);
                printf("best case instances: %f\n", 100 / percent);
                printf("quota used per 1 percent : %f\n", percent * 100);
                fflush(stdout);
                done = true;
            }
        }
    }

   /**
    * Run test iterators time, return total seconds.
    */
    static double measureTimeSub(std::function<T()> func, int64_t iterations)
    {
        const double t0 = SqTime::seconds();
        for (int64_t i = 0; i < iterations; ++i) {
            const T x = func();
            TestBuffers<T>::put(x);
        }

        const double t1 = SqTime::seconds();
        const double elapsed = t1 - t0;
        return elapsed;
    }
};


/**
 * Simple producer / consumer for test data.
 * Serves up a precalculated list of random numbers.
 */
template <typename T>
class TestBuffers
{
public:
    static const size_t size = 60000;
    static void put(T x)
    {
        destData[destIndex++] = x;
        if (destIndex >= size) {
            destIndex = 0;
        }
    }
    static T get()
    {
        T ret = sourceData[sourceIndex++];
        if (sourceIndex >= size) {
            sourceIndex = 0;
        }
        return ret;
    }
    //
    TestBuffers()
    {
        for (int i = 0; i < size; ++i) {
            sourceData[i] = (float) rand() / (float) RAND_MAX;
        }
    }
private:
    static size_t sourceIndex;
    static size_t destIndex;
    static T sourceData[size];
    static T destData[size];
};


template <typename T>
T TestBuffers<T>::sourceData[size];

template <typename T>
T TestBuffers<T>::destData[size];

template <typename T>
size_t TestBuffers<T>::sourceIndex = 0;

template <typename T>
size_t TestBuffers<T>::destIndex = 512;


/**
 * Simple timer implementation for running inside Visual Studio
 */





