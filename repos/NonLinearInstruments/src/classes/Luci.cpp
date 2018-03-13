#pragma once
#include "dsp/digital.hpp"
namespace rack {

//  Base class for Luci
// Luci cell has its own frequency, externally adjustable.
// When signal reaches +1, cell bangs her trigger out simmultaneously on her four out ports.
// She gets 4 incoming triggers from neighbouring cells. 
// The incoming triggers modify the ramp by influence amount.
	
struct luciCell {

	// luci variables
	float frequency = .5f;	// in HZ
	float f0 = .1277468f; // 1volt/octave C-6 = 0, transposed -6 octaves...	
	float influence = 0.f;
	float phase = (rand() % 100) / 100.f; // start with random phase
	
	// integration
	float delta = frequency / engineGetSampleRate();
	
	// this is for the standard vcv trigger
	//PulseGenerator trigOut;
	//float trigOutValue = 0.f;
	//float triggerLength = 10e-3;
	
	// this is for the intrinsic one sample Luci trigger (to feed other cells)
	bool luciTrigger = false;
	bool randomizedStatus = false;
	
	// from CV to Hz
	void setFrequency( float pitchVoltage ){
		frequency = f0 * powf(2.f, pitchVoltage);
	}
	
	void setInfluence( float _influence){
		// expected parameter cv 0~10v
		// influence =  clampf( _influence, -3.50124f, 3.50124f);
		influence = 0.f + 1.050732f * ( _influence / 10.f);
	}
	
	void setTriggers( bool _trig1, bool _trig2, bool _trig3, bool _trig4){
		if( _trig1 || _trig2 || _trig3 || _trig4 ){
			phase *= influence;
		}
	}
	
	void randomize( bool _rnd_trig){
		if( _rnd_trig ){
			phase = (rand() % 100) / 100.f;
			randomizedStatus = true;
			} else { randomizedStatus = false; }
	}
	
	// shoot triggers out and reset phase
	void resetLuci (){
		//trigOut.trigger(triggerLength);
		luciTrigger = true;
		phase = 0.f;
	}
	
	float process(){
		// find increment per sample
		delta = frequency / engineGetSampleRate();
		
		// prepare trigOut signal
		// trigOutValue = 10.f * (float) trigOut.process(delta);
		
		// restart cycle when reaching 1.f
		if( phase >= 1.f){
			this->resetLuci();
		} else {
			luciTrigger = false;
		}
		// integrate
		phase += delta;
		// process function returns an audio signal
		// avoid random noise to slip thru output while randomize button is pressed
		if( randomizedStatus ){ 
			return( 0.f );
			} else {
			return( ( 10.f * phase ) - 5.f );
			}
	}
	
	// extract trigger values
	// float getTrigger (){
	// return( trigOutValue );
	// }
	
	bool getLuciTrigger (){
		return (luciTrigger);
	}
};

}; // namespace rack
