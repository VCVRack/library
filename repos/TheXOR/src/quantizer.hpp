#include "common.hpp"

////////////////////
// module widgets
////////////////////
using namespace rack;
extern Plugin *plugin;

#define NUM_QUANTIZERS  (6)

struct Quantizer;
struct QuantizerWidget : ModuleWidget
{
	QuantizerWidget(Quantizer * module);
};

struct Quantizer : Module
{
	enum ParamIds
	{
		NUM_PARAMS
	};
	enum InputIds
	{
		IN_1,
		NUM_INPUTS = IN_1 + NUM_QUANTIZERS
	};
	enum OutputIds
	{
		OUT_1,
		NUM_OUTPUTS = OUT_1 + NUM_QUANTIZERS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};
	Quantizer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{		
	}
	void step() override;

private:
	float quantize_out(Input &in);
};
