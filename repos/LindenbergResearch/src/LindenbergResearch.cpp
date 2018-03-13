#include "LindenbergResearch.hpp"

using namespace rack;


Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    p->slug = TOSTRING(SLUG);
    p->version = TOSTRING(VERSION);

    p->addModel(modelSimpleFilter);
    p->addModel(modelMS20Filter);
    p->addModel(modelAlmaFilter);
    p->addModel(modelReShaper);
    p->addModel(modelVCO);
    p->addModel(modelBlankPanel);
    p->addModel(modelBlankPanelM1);
}
