#include "Simple.hpp"

#include "ClockWidget.hpp"

rack::Plugin* plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	p->slug = "IO-Simple";

#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
    p->website = "https://iohannrabeson.github.io/VCVRack-Simple/";

	p->addModel(rack::createModel<ButtonTriggerWidget>("Simple", "IO-ButtonTrigger", "Button Trigger", rack::UTILITY_TAG));
	p->addModel(rack::createModel<ClockDividerWidget>("Simple", "IO-ClockDivider", "Clock Divider", rack::CLOCK_TAG));
	p->addModel(rack::createModel<RecorderWidget>("Simple", "IO-Recorder", "Recorder", rack::UTILITY_TAG));
	p->addModel(rack::createModel<ClockWidget>("Simple", "IO-Clock", "Clock", rack::CLOCK_TAG));
}
