#pragma once
#include "dsp/digital.hpp"
namespace rack {

// a basic parabolic projectile motion
// https://en.wikipedia.org/wiki/Projectile_motion

struct Ballistic {
	// newtonian
	float impulse = 10.f;
	float gravity = 9.8f; // g
	float angle = 45.f;
	float zenith = 1.f / (pow(impulse, 2) / (2.f * gravity));
	float bounce = 0.000001f;
	// integration
	const float radFactor  = M_PI / 180.f;
	float delta = 1.f / engineGetSampleRate();
	float phase = 0.f;
	// signal
	float yValue = 0.f;
	float audioOut = 0.f;
	float controlValue = 0.f;
	float controlOut = 0.f;
	PulseGenerator zeroTrigOut;
	float zeroTrigSignal = 0.f;
	// float triggerLength = engineGetSampleRate()/1000.f;
	float triggerLength = 10e-3;
	bool isRunning = false;
	bool bounceOnOff = false;
	bool isReBounding = false;
	int reBoundCount = 0;
	// get switches from panel
	void setBounceOnOff ( bool _bounceOnOff ){
		bounceOnOff = _bounceOnOff;
	}	
	//get parameters from panel
	void setImpulse( float _impulse ){
		if(isRunning){
			// avoid very small impulses. 
			impulse = 0.01f + 100.f * pow ( clampf( _impulse, 0.f, 1.f ) ,2) ;
		}
	}
	void setGravity( float _gravity )
	{
		if(isRunning){
			// avoid zero gravity. Max. 10g
			gravity = 0.01f + 98.f * clampf( _gravity, 0.f, 1.f );
			}
	}	
	void setAngle  ( float _angle ){
		if(isRunning){
			// avoid shoot at 0º. max angle slightly less than 90º
			// angle in degrees ( parameter must be 0. ~ 1. )	
			if(isReBounding){
				angle = radFactor *  ( 0.001f + ( 89.998 * clampf( _angle, 0.f, 1.f) * pow(bounce,(float)reBoundCount) )); //radical !!
				} else {
				angle = radFactor *  ( 0.001f + ( 89.998 * clampf( _angle, 0.f, 1.f) ));
				}
			}
	}
	void setBounce ( float _bounce){
		// use a log scale
		bounce = ( 6.f + log10( clampf( _bounce, 0.000001f, 0.99f))) / 6.f;
	}	
	// compute trajectory
	void shoot (bool trigger){	
		// get trigger
		if ( trigger ){ 
			isRunning = true; 
			isReBounding = false;
			reBoundCount = 0.f;
			// bridge zer trigger to received trigger
			zeroTrigOut.trigger(triggerLength);
			}
		// compute trajectory
		if( isRunning ){						
			// max. height
			zenith = (pow(impulse, 2) / (2.f * gravity));
			// integrate trajectory normalized to 1
			yValue = ((impulse * sin( angle ) * phase) - (gravity * pow( phase, 2) * 0.5f )) / zenith;
			// copy signal to CV
			controlValue = yValue;
			// integration step
			phase += delta;		
			// touch the ground 
			if( yValue < 0.f ){ 		
				// launch "zero trigger" when trajectory's "y" crosses zero
				zeroTrigOut.trigger(triggerLength);	
				// check bounce mode
				if ( bounceOnOff ){
				yValue = controlValue = 0.f;
				phase = 0.f;
				isReBounding = true;
				reBoundCount++;
				//stop after an arbitrary number of rebounds
				if( reBoundCount >= 16384 ){
					isRunning = false;	
					isReBounding = false;
					reBoundCount = 0;
					yValue = controlValue = 0.f;
					}
				} else {
				isRunning = false;	
				isReBounding = false;
				reBoundCount = 0;
				yValue = controlValue = 0.f;
				}
			}			
			// if is rebounding, switch signal sign for impair rebounds
			if (isReBounding && reBoundCount % 2 != 0) { yValue *= -1.f;}
		} else {
			// not running ? silence...
			yValue 
			= controlValue 
			= controlOut 
			= audioOut 
			= phase 
			= reBoundCount 
			= zeroTrigSignal = 0.f;
		}
		// feed signals
		audioOut =  5.f * yValue;
		controlOut = 10.f * controlValue; 
		// prepare trigger signals
		zeroTrigSignal = 10.f * (float) zeroTrigOut.process(1.0 / engineGetSampleRate());
	}
	// retrieve audio signal
	float getAudio () {
		return std::isfinite( audioOut ) ? audioOut : 0.f;
	}
	// retrieve control signal
	float getControl () {
		return std::isfinite( controlOut ) ? controlOut : 0.f;
	}
	// retrieve zero trigger
	float getZeroTrigger (){
		return ( zeroTrigSignal );
	}
};

}; // namespace rack