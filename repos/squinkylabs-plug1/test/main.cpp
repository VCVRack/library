/**
  * Unit test entry point
  */

#include <stdio.h>
#include <assert.h>
#include <string>

extern void testBiquad();
extern void testSaw();
extern void testLookupTable();
extern void testSinOscillator();
extern void testHilbert();
extern void testAudioMath();
extern void perfTest();
extern void testFrequencyShifter();

int main(int argc, char ** argv)
{
    bool runPerf = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--perf") {
            runPerf = true;
        }
    }
    // While this code may work in 32 bit applications, it's not tested for that.
    // Want to be sure we are testing the case we care about.
    assert(sizeof(size_t) == 8);

    testAudioMath();
    testBiquad();
    testSaw();
    testLookupTable();
    testSinOscillator();
    testHilbert();

    // after testing all the components, test composites.
    testFrequencyShifter();

    if (runPerf) {
        perfTest();
    }

    // When we run inside Visual Studio, don't exit debugger immediately
#if defined(_MSC_VER)
    printf("Test passed. Press any key to continue...\n"); fflush(stdout);
    getchar();
#else
    printf("Tests passed.\n");
#endif
}