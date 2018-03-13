#include "CharredDesert.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
  plugin = p;
  // This is the unique identifier for your plugin
  p->slug = TOSTRING(SLUG);
  p->version = TOSTRING(VERSION);
  p->website = "https://github.com/JerrySievert/CharredDesert";
  p->manual =
      "https://github.com/JerrySievert/CharredDesert/blob/master/docs/README.md";

  p->addModel(modelDTMF);
  p->addModel(modelNoise);
  p->addModel(modelCVSeq);
  p->addModel(modelNot);
  p->addModel(modelPan);
  p->addModel(modelShift);
}
