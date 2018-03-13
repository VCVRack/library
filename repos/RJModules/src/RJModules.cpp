#include "RJModules.hpp"
#include "VAStateVariableFilter.h"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/Miserlou/RJModules";

    // Generators
    p->addModel(modelSupersaw);
    p->addModel(modelTwinLFO);
    p->addModel(modelNoise);
    p->addModel(modelRangeLFO);

    // FX
	p->addModel(modelBitCrush);
    p->addModel(modelWidener);
    p->addModel(modelFilterDelay);
    p->addModel(modelSidechain);
    p->addModel(modelStutter);

    // Filters
    p->addModel(modelFilter);
    p->addModel(modelFilters);
    p->addModel(modelNotch);

    // Numerical
    p->addModel(modelIntegers);
    p->addModel(modelFloats);
    p->addModel(modelRandoms);

    // Mix
    p->addModel(modelLRMixer);
    p->addModel(modelMono);
    p->addModel(modelVolumes);
    p->addModel(modelPanner);
    p->addModel(modelPanners);

    // Live
    p->addModel(modelBPM);
    p->addModel(modelButton);
    p->addModel(modelButtons);

    // Util
    p->addModel(modelSplitter);
    p->addModel(modelSplitters);
    p->addModel(modelDisplays);
    p->addModel(modelRange);


}


