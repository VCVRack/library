#include "PluginInit.h"
#include "SEQWidget.h"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p)
{
    plugin = p;
    plugin->slug = "KarateSnoopy";
    plugin->website = "https://github.com/KarateSnoopy/vcv-karatesnoopy";
#ifdef VERSION
    plugin->version = TOSTRING(VERSION);
#endif

    p->addModel(createModel<SEQWidget>("KarateSnoopy", "KSnpy 2D Grid Seq", "KSnpy 2D Grid Seq", SEQUENCER_TAG));
}
