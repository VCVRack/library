#include "rack.hpp"


using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MultipleWidget : ModuleWidget {
	MultipleWidget();
};

struct Blank8hpWidget : ModuleWidget {
	Blank8hpWidget();
};

struct p0wrWidget : ModuleWidget {
	p0wrWidget();
};

struct TR808CowbellWidget : ModuleWidget {
	TR808CowbellWidget();
};
