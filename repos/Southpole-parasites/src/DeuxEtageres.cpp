#include <iostream>
#include <stdlib.h>

#include "Southpole.hpp"
#include "dsp/digital.hpp"
#include "VAStateVariableFilter.h"


struct DeuxEtageres : Module {
    enum ParamIds {
        FREQ1_PARAM,
        FREQ2_PARAM,
        FREQ3_PARAM,
        FREQ4_PARAM,
        GAIN1_PARAM,
        GAIN2_PARAM,
        GAIN3_PARAM,
        GAIN4_PARAM,
        Q2_PARAM,
        Q3_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FREQ1_INPUT,
        FREQ2_INPUT,
        FREQ3_INPUT,
        FREQ4_INPUT,
		GAIN1_INPUT,
		GAIN2_INPUT,
		GAIN3_INPUT,
		GAIN4_INPUT,
		Q2_INPUT,
		Q3_INPUT,
		INL_INPUT,
		INR_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTL_OUTPUT,
        OUTR_OUTPUT,
        NUM_OUTPUTS
    };
	enum LightIds {
		CLIPL_LIGHT,
		CLIPR_LIGHT,
		NUM_LIGHTS
	};

    VAStateVariableFilter lpFilter[2];
    VAStateVariableFilter bp2Filter[2];
    VAStateVariableFilter bp3Filter[2];
    VAStateVariableFilter hpFilter[2];

    DeuxEtageres() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {

        params.resize(NUM_PARAMS);
        inputs.resize(NUM_INPUTS);
        outputs.resize(NUM_OUTPUTS);
        lights.resize(NUM_LIGHTS);

        for (int i=0; i<2; i++) {
            lpFilter[i].setFilterType(SVFLowpass);
            hpFilter[i].setFilterType(SVFHighpass);        
            bp2Filter[i].setFilterType(SVFBandpass);
            bp3Filter[i].setFilterType(SVFBandpass);
        }
        
    }
    void step() override;

//    void reset() override {
//    }
//    void randomize() override {
//    }

    json_t *toJson() override {
        json_t *rootJ = json_object();
        // states
        //json_t *statesJ = json_array();
        //for (int i = 0; i < NUM_CHANNELS; i++) {
        //    json_t *stateJ = json_boolean(state[i]);
        //    json_array_append_new(statesJ, stateJ);
        //}
        //json_object_set_new(rootJ, "states", statesJ);
        return rootJ;
    }
    void fromJson(json_t *rootJ) override {
        // states
        //json_t *statesJ = json_object_get(rootJ, "states");
        //if (statesJ) {
        //    for (int i = 0; i < NUM_CHANNELS; i++) {
        //        json_t *stateJ = json_array_get(statesJ, i);
        //        if (stateJ)
        //            state[i] = json_boolean_value(stateJ);
        //    }
        //}
    }

    //unsigned timer;
};

void DeuxEtageres::step() {

		float g_gain  = 0.;
        float gain1 = clampf(g_gain + params[GAIN1_PARAM].value + inputs[GAIN1_INPUT].normalize(0.) / 10.0, -1.0, 1.0);
        float gain2 = clampf(g_gain + params[GAIN2_PARAM].value + inputs[GAIN2_INPUT].normalize(0.) / 10.0, -1.0, 1.0);
        float gain3 = clampf(g_gain + params[GAIN3_PARAM].value + inputs[GAIN3_INPUT].normalize(0.) / 10.0, -1.0, 1.0);
        float gain4 = clampf(g_gain + params[GAIN4_PARAM].value + inputs[GAIN4_INPUT].normalize(0.) / 10.0, -1.0, 1.0);

		float g_cutoff = 0.;
        float freq1 = clampf(g_cutoff + params[FREQ1_PARAM].value + inputs[FREQ1_INPUT].normalize(0.), -4.0, 6.0);
        float freq2 = clampf(g_cutoff + params[FREQ2_PARAM].value + inputs[FREQ2_INPUT].normalize(0.), -4.0, 6.0);
        float freq3 = clampf(g_cutoff + params[FREQ3_PARAM].value + inputs[FREQ3_INPUT].normalize(0.), -4.0, 6.0);
        float freq4 = clampf(g_cutoff + params[FREQ4_PARAM].value + inputs[FREQ4_INPUT].normalize(0.), -4.0, 6.0);

        float reso2 = clampf(g_cutoff + params[Q2_PARAM].value + inputs[Q3_INPUT].normalize(0.) / 10.0, .0, 1.0);
        float reso3 = clampf(g_cutoff + params[Q3_PARAM].value + inputs[Q3_INPUT].normalize(0.) / 10.0, .0, 1.0);

        for (int i=0; i<2; i++) {
            lpFilter[i].setQ(.5); //Resonance(.5);
            hpFilter[i].setQ(.5); //Resonance(.5);

            lpFilter[i].setSampleRate(engineGetSampleRate());
            hpFilter[i].setSampleRate(engineGetSampleRate());
            bp2Filter[i].setSampleRate(engineGetSampleRate());
            bp3Filter[i].setSampleRate(engineGetSampleRate());

            float dry = inputs[INL_INPUT + i].value;

            const float f0 = 261.626;
            
            const float rmax = 0.9995; // Qmax = 1000
            //const float rmax = 0.975; // Qmax = 20

            float lp_cutoff  = f0 * powf(2.f, freq1);
            float bp2_cutoff = f0 * powf(2.f, freq2);
            float bp3_cutoff = f0 * powf(2.f, freq3);
            float hp_cutoff  = f0 * powf(2.f, freq4);

            lpFilter[i].setCutoffFreq(lp_cutoff);
            
            bp2Filter[i].setCutoffFreq(bp2_cutoff);
            bp2Filter[i].setResonance( rmax*reso2 );

            bp3Filter[i].setCutoffFreq(bp3_cutoff);
            bp3Filter[i].setResonance( rmax*reso3 );

            hpFilter[i].setCutoffFreq(hp_cutoff);
            //hpFilter.setResonance( params[Q3_PARAM].value );

            float lpout  = lpFilter[i].processAudioSample(dry, 1);
            float bp2out = bp2Filter[i].processAudioSample(dry, 1);
            float bp3out = bp3Filter[i].processAudioSample(dry, 1);
            float hpout  = hpFilter[i].processAudioSample(dry, 1);

            float lpgain  = pow(20.,-gain1); 
            float bp2gain = pow(20.,-gain2); 
            float bp3gain = pow(20.,-gain3); 
            float hpgain  = pow(20.,-gain4);
            
            float sumout = lpout*lpgain + hpout*hpgain +  bp2out*bp2gain + bp3out*bp3gain;

            outputs[OUTL_OUTPUT + i].value = sumout;
    
            // Lights
            lights[CLIPL_LIGHT + i].setBrightnessSmooth( fabs(sumout) > 10. ? 1. : 0. );    
        }
}

DeuxEtageresWidget::DeuxEtageresWidget() {

    DeuxEtageres *module = new DeuxEtageres();
    setModule(module);
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DeuxEtageres.svg")));
		panel->box.size = box.size;
		addChild(panel);	
	}

    const float x1 = 8;
	const float x2 = 65;
	
    const float y1 = 5.;
    const float yh = 25.;

    const float vfmin = -4.;
    const float vfmax =  6.;

    const float gmax = -1.;
    const float gmin =  1.;

    // TO DO possible default freqs: 880, 5000

    addInput(createInput<sp_Port>(Vec(x1, y1+ 1* yh), module, DeuxEtageres::FREQ4_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 2* yh), module, DeuxEtageres::GAIN4_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 1*yh), module, DeuxEtageres::FREQ4_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 2*yh), module, DeuxEtageres::GAIN4_PARAM,  gmin,  gmax, 0.));

    addInput(createInput<sp_Port>(Vec(x1, y1+ 3* yh), module, DeuxEtageres::FREQ2_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 4* yh), module, DeuxEtageres::GAIN2_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 5* yh), module, DeuxEtageres::Q2_INPUT));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 3*yh), module, DeuxEtageres::FREQ2_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 4*yh), module, DeuxEtageres::GAIN2_PARAM,  gmin,  gmax, 0.));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1+ 5*yh), module, DeuxEtageres::Q2_PARAM,      0.0,   1.0, 0.));
    
    addInput(createInput<sp_Port>(Vec(x1, y1+ 6* yh), module, DeuxEtageres::FREQ3_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 7* yh), module, DeuxEtageres::GAIN3_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+ 8* yh), module, DeuxEtageres::Q3_INPUT));
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 6*yh), module, DeuxEtageres::FREQ3_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 7*yh), module, DeuxEtageres::GAIN3_PARAM,  gmin,  gmax, 0.));
    addParam(createParam<sp_Trimpot>(Vec(x2, y1+ 8*yh), module, DeuxEtageres::Q3_PARAM,      0.0,   1.0, 0.));
    
    addInput(createInput<sp_Port>(Vec(x1, y1+ 9* yh), module, DeuxEtageres::FREQ1_INPUT));
    addInput(createInput<sp_Port>(Vec(x1, y1+10* yh), module, DeuxEtageres::GAIN1_INPUT));    
	addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+ 9*yh), module, DeuxEtageres::FREQ1_PARAM, vfmin, vfmax, 0.));
    addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1+10*yh), module, DeuxEtageres::GAIN1_PARAM,  gmin,  gmax, 0.));
	
    addInput(createInput<sp_Port>(  Vec(x1, y1+12*yh), module, DeuxEtageres::INL_INPUT));    
    addOutput(createOutput<sp_Port>(Vec(x2, y1+12*yh), module, DeuxEtageres::OUTL_OUTPUT));
	addChild(createLight<SmallLight<RedLight>>(Vec(x2-10., y1+12.2*yh), module, DeuxEtageres::CLIPL_LIGHT));

    addInput(createInput<sp_Port>(  Vec(x1, y1+13*yh), module, DeuxEtageres::INR_INPUT));    
    addOutput(createOutput<sp_Port>(Vec(x2, y1+13*yh), module, DeuxEtageres::OUTR_OUTPUT));
	addChild(createLight<SmallLight<RedLight>>(Vec(x2-10., y1+13.2*yh), module, DeuxEtageres::CLIPR_LIGHT));
}
