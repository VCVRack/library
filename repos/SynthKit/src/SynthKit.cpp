#include "SynthKit.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
  plugin = p;
  // This is the unique identifier for your plugin
  p->slug = TOSTRING(SLUG);
  p->version = TOSTRING(VERSION);
  p->website = "https://github.com/JerrySievert/SynthKit";
  p->manual = "https://github.com/JerrySievert/SynthKit/blob/master/README.md";

  // For each module, specify the ModuleWidget subclass, manufacturer slug (for
  // saving in patches), manufacturer human-readable name, module slug, and
  // module name
  p->addModel(modelAddition);
  p->addModel(modelSubtraction);
  p->addModel(modelAnd);
  p->addModel(modelOr);
  p->addModel(modelM1x8);
  p->addModel(modelM1x8CV);
  p->addModel(modelClockDivider);
  p->addModel(modelRotatingClockDivider);
  p->addModel(modelPrimeClockDivider);
  p->addModel(modelFibonacciClockDivider);
  p->addModel(modelSeq4);
  p->addModel(modelSeq8);
  p->addModel(modelDevKit);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your
  // module is created to reduce startup times of Rack.
}
