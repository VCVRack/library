
#include "bogaudio.hpp"

#include "Additator.hpp"
#include "EightFO.hpp"

#include "Shaper.hpp"
#include "ShaperPlus.hpp"
#include "DADSRH.hpp"
#include "DADSRHPlus.hpp"

#include "Analyzer.hpp"

#include "Detune.hpp"
#include "DGate.hpp"
#include "Manual.hpp"
#include "Noise.hpp"
#include "Offset.hpp"
#include "Reftone.hpp"
#include "SampleHold.hpp"
#include "Stack.hpp"
#include "Switch.hpp"
#include "VCA.hpp"

#include "Test.hpp"
#include "template_panels.hpp"

//NEW_INCLUDES_HERE

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/bogaudio/BogaudioModules";
	p->manual = "https://github.com/bogaudio/BogaudioModules/blob/master/README.md";

#ifdef EXPERIMENTAL
	p->addModel(modelAdditator);
	p->addModel(modelEightFO);
#endif

	p->addModel(modelShaper);
	p->addModel(modelShaperPlus);
	p->addModel(modelDADSRH);
	p->addModel(modelDADSRHPlus);

	p->addModel(modelAnalyzer);

	p->addModel(modelDetune);
	p->addModel(modelDGate);
	p->addModel(modelManual);
	p->addModel(modelNoise);
	p->addModel(modelOffset);
	p->addModel(modelReftone);
	p->addModel(modelSampleHold);
	p->addModel(modelStack);
	p->addModel(modelSwitch);
	p->addModel(modelVCA);

#ifdef TEST
	p->addModel(modelTest);

	p->addModel(modelThreeHP);
	p->addModel(modelSixHP);
	p->addModel(modelEightHP);
	p->addModel(modelTenHP);
	p->addModel(modelTwelveHP);
	p->addModel(modelThirteenHP);
	p->addModel(modelFifteenHP);
	p->addModel(modelEighteenHP);
	p->addModel(modelTwentyHP);
	p->addModel(modelTwentyTwoHP);
	p->addModel(modelTwentyFiveHP);
	p->addModel(modelThirtyHP);
#endif

	//NEW_MODELS_HERE
}
