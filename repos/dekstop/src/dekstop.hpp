#include "rack.hpp"


using namespace rack;

extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct TriSEQ3Widget : ModuleWidget {
	TriSEQ3Widget();
	json_t *toJsonData();
	void fromJsonData(json_t *root);
};

struct GateSEQ8Widget : ModuleWidget {
	GateSEQ8Widget();
	json_t *toJsonData();
	void fromJsonData(json_t *root);
};

template <unsigned int ChannelCount>
struct RecorderWidget : ModuleWidget {
	RecorderWidget();
	json_t *toJsonData();
	void fromJsonData(json_t *root);
};

struct Recorder2Widget : RecorderWidget<2u>
{
	Recorder2Widget();
};

struct Recorder8Widget : RecorderWidget<8u>
{
	Recorder8Widget();
};
