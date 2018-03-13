#include "Southpole.hpp"
#include "dsp/digital.hpp"

#define NBUF 6 

struct Rakes : Module {
	enum ParamIds {
		DECAY_PARAM,
		MIX_PARAM,
		TUNE1_PARAM,
		TUNE2_PARAM,
		TUNE3_PARAM,
		TUNE4_PARAM,
		TUNE5_PARAM,
		TUNE6_PARAM,
		GAIN1_PARAM,
		GAIN2_PARAM,
		GAIN3_PARAM,
		GAIN4_PARAM,
		GAIN5_PARAM,
		GAIN6_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INL_INPUT,
		INR_INPUT,
		DECAY_INPUT,
		MIX_INPUT,
		TUNE1_INPUT,
		TUNE2_INPUT,
		TUNE3_INPUT,
		TUNE4_INPUT,
		TUNE5_INPUT,
		TUNE6_INPUT,
		//GAIN1_INPUT,GAIN2_INPUT,
		//GAIN3_INPUT,GAIN4_INPUT,
		//GAIN5_INPUT,GAIN6_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//SchmittTrigger clock;

	float *bufl[NBUF];
	float *bufr[NBUF];
	int maxsize;

	int head[NBUF];
	int size[NBUF];

	Rakes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	

		maxsize = engineGetSampleRate();

		for (int j=0; j < NBUF; j++) {
  			bufl[j] = new float [maxsize];
  			bufr[j] = new float [maxsize];
			for (int i=0; i < maxsize; i++) {
				bufl[j][i] = 0;		
				bufr[j][i] = 0;		
			}
			head[j] = 0;
		}
	}

	float xm1 = 0;
	float ym1 = 0;

	float dcblock( float x ) {
		float y = x - xm1 + 0.995 * ym1;
  		xm1 = x;
  		ym1 = y;
		return y;
	}

	void step() override;
};



void Rakes::step() {

	//float mix  = clampf(params[MIX_PARAM].value + inputs[MIX_INPUT].normalize(0.) / 10.0, 0.0, 1.0);
	float mix  = params[MIX_PARAM].value;
	float rate = clampf(params[DECAY_PARAM].value + inputs[DECAY_INPUT].normalize(0.) / 10.0, 0.0, 1.0);

	const float f0 = 261.626;
	float inl  = inputs[INL_INPUT].normalize(0.);
	float inr  = inputs[INR_INPUT].normalize(inl);

	float sumoutl  = 0;
	float sumoutr  = 0;
	float sumgain = 1.;

	for (int j=0; j < NBUF; j++) {
		//float gain = clampf(params[GAIN1_PARAM + j].value + inputs[GAIN1_INPUT + j].normalize(0.) / 10.0, 0.0, 1.0);
		float gain = params[GAIN1_PARAM + j].value;
		if (gain < 1e-3) continue;
		sumgain += gain;

		float tune = clampf(params[TUNE1_PARAM + j].value + inputs[TUNE1_INPUT + j].normalize(0.), -5.0, 5.5);
		//tune = round(12.*tune)/12.;
		float freq = f0 * powf(2., tune);

		// decay rate is T60 time
		float fb = pow(10, -3./freq/fabs(5.*rate));

		size[j] = maxsize / freq;

		float outl = bufl[j][head[j]];
		float outr = bufr[j][head[j]];

		bufl[j][head[j]] = inl - fb * outl;
		bufr[j][head[j]] = inr - fb * outr;
		head[j]++;
		if (head[j] > size[j]) { head[j] = 0; }

		sumoutl  += gain*outl;
		sumoutr  += gain*outr;
	}

	sumoutl = dcblock(sumoutl) / sumgain; //in + gain*out;
	sumoutr = dcblock(sumoutr) / sumgain; //in + gain*out;

	outputs[OUTL_OUTPUT].value = crossf(inl,sumoutl,mix);
	outputs[OUTR_OUTPUT].value = crossf(inr,sumoutr,mix);
}


RakesWidget::RakesWidget() {
	Rakes *module = new Rakes();
	setModule(module);
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Rakes.svg")));
		addChild(panel);
	}

	const float x1 = 5.;
	const float x2 = 35.;
	const float x3 = 65.;
	const float y1 = 40.;
	const float yh = 32.;
	 
	addInput(createInput<sp_Port>			(Vec(x1+15, y1+0*yh), module, Rakes::DECAY_INPUT));
    addParam(createParam<sp_SmallBlackKnob>	(Vec(x1+45, y1+0*yh), module, Rakes::DECAY_PARAM, 0.0, 1.0, 0.5));

	for (int j=0; j < NBUF; j++) {
		addInput(createInput<sp_Port>			(Vec(x1, y1+(j+1.2)*yh), module, Rakes::TUNE1_INPUT + j));
    	addParam(createParam<sp_SmallBlackKnob>	(Vec(x2, y1+(j+1.2)*yh), module, Rakes::TUNE1_PARAM + j,  -5.0, 5.5, 0.0));
		//addInput(createInput<sp_Port>			(Vec(x1+50, y2+j.2*yh), module, Rakes::GAIN1_INPUT + j));
    	addParam(createParam<sp_SmallBlackKnob>	(Vec(x3, y1+(j+1.2)*yh), module, Rakes::GAIN1_PARAM + j,  0.0, 1.0, 0.0));
	}


	addInput(createInput<sp_Port>	(Vec(x1+15, y1+8*yh), module, Rakes::INL_INPUT));
	addInput(createInput<sp_Port>	(Vec(x1+15, y1+9*yh), module, Rakes::INR_INPUT));

	//addInput(createInput<sp_Port>			(Vec(x1, y1+7*yh), module, Rakes::MIX_INPUT));
    addParam(createParam<sp_SmallBlackKnob>	(Vec(x2, y1+7*yh), module, Rakes::MIX_PARAM, 0.0, 1.0, 0.5));

	addOutput(createOutput<sp_Port> (Vec(x1+45, y1+8*yh), module, Rakes::OUTL_OUTPUT));
	addOutput(createOutput<sp_Port> (Vec(x1+45, y1+9*yh), module, Rakes::OUTR_OUTPUT));
}




