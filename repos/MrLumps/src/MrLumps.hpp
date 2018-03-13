#include "rack.hpp"
#include "app.hpp"

using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct VCS1x8Widget : ModuleWidget {
    VCS1x8Widget();
};

struct VCS2x4Widget : ModuleWidget {
    VCS2x4Widget();
};

struct SEQEuclidWidget : ModuleWidget {
	SEQEuclidWidget();
	Menu *createContextMenu() override;
};
