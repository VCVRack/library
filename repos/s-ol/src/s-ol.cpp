#include "s-ol.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
  plugin = p;

  p->slug = TOSTRING(SLUG);
  p->version = TOSTRING(VERSION);
  p->website = "https://github.com/s-ol/vcvmods";
  p->manual = "https://github.com/s-ol/vcvmods";

  p->addModel(modelCircleVCO);
  p->addModel(modelWrapComp);
  p->addModel(modelModulo);
}
