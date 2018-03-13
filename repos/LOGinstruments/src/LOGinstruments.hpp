#include "rack.hpp"

using namespace rack;


extern Plugin *plugin;

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}


////////////////////
// Additional GUI stuff
////////////////////

#ifdef DEBUG
  #define dbgPrint(a) printf a
#else
  #define dbgPrint(a) (void)0
#endif


extern Model *modelconstant;
extern Model *modelconstant2;
extern Model *modelSpeck;
extern Model *modelBritix;
extern Model *modelCompa;
extern Model *modelLessMess;
extern Model *modelVelvet;
extern Model *modelCrystal;
