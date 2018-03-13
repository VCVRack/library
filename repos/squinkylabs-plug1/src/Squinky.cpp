// plugin main
#include "Squinky.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

/**
 * Here we register the whole plugin, which may have more than one module in it.
 */
void init(rack::Plugin *p)
{
    plugin = p;
    p->slug = "squinkylabs-plug1";
    p->version = TOSTRING(VERSION);
    //p->website = "https://github.com/VCVRack/Tutorial";
    //p->manual = "https://github.com/VCVRack/Tutorial/blob/master/README.md";

    assert(modelBootyModule);
    p->addModel(modelBootyModule);
}