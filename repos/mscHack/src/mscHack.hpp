#include "rack.hpp"
#include "CLog.h"

using namespace rack;

extern Plugin *plugin;

#define CV_MAX (15.0f)
#define AUDIO_MAX (6.0f)
#define VOCT_MAX (6.0f)

#define TOJSON true
#define FROMJSON false

void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len );
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len );

////////////////////
// module widgets
////////////////////

struct MasterClockx4_Widget : ModuleWidget {
	MasterClockx4_Widget();
};

struct Seq_3x16x16_Widget : ModuleWidget {
	Seq_3x16x16_Widget();
};

struct SEQ_6x32x16_Widget : ModuleWidget {
	SEQ_6x32x16_Widget();
};

struct SynthDrums_Widget : ModuleWidget {
	SynthDrums_Widget();
};

struct Seq_Triad_Widget : ModuleWidget {
	Seq_Triad_Widget();
};

struct Seq_Triad2_Widget : ModuleWidget {
	Seq_Triad2_Widget();
};

struct ARP700_Widget : ModuleWidget {
	ARP700_Widget();
};

struct Mix_1x4_Stereo_Widget : ModuleWidget {
	Mix_1x4_Stereo_Widget();
};

struct Mix_2x4_Stereo_Widget : ModuleWidget {
	Mix_2x4_Stereo_Widget();
};

struct Mix_4x4_Stereo_Widget : ModuleWidget {
	Mix_4x4_Stereo_Widget();
};

struct Mix_4x4_Stereo_Widget_old : ModuleWidget {
	Mix_4x4_Stereo_Widget_old();
};

struct PingPong_Widget : ModuleWidget {
	PingPong_Widget();
};

struct Osc_3Ch_Widget : ModuleWidget {
	Osc_3Ch_Widget();
};

struct Compressor_Widget : ModuleWidget {
	Compressor_Widget();
};

struct XFade_Widget : ModuleWidget {
	XFade_Widget();
};




