// 
//  Base class for Quadratic iterator instruments
//

#pragma once

namespace rack {

struct logisticMap {

	float ax = 0.1;
	float axnew = 0.0;
	float Cparam = 3.57;
	float x_out = 0.0;
	float Cmax = 3.998000;
	//float Cmin = 3.569959;
	float Cmin = 3.870000;
	float Crange = Cmax - Cmin;
	float CrangeFine = 0.001999;
	float Cvalue = Cmin;
	float CvalueFine = 0.0;
	float CmodValue = 0.0;
	float CmodDepthParam = 0.0;
	float output = 0.0;
	
	
	void setCmod_depth_param(float _value){
	CmodDepthParam = _value / 10.;
	CmodDepthParam *= CmodDepthParam;;
	}
	
	
	void setCmod_value(float _value){
	CmodValue = _value * CmodDepthParam;
	}


	void setCvalue(float _value){
	Cvalue = _value * Crange;
	}

	
	void setCvalueFine(float _value){
	CvalueFine = _value * CrangeFine;
	}


	void process (){
	CmodValue = CmodValue * (Cmax - Cvalue);
	Cparam = clampf ( Cmin + Cvalue + CvalueFine + CmodValue, Cmin , Cmax );
	axnew = Cparam * ax * ( 1.f - ax );
	output = std::isfinite( axnew ) ? 5. * axnew : 0.f;
	ax = axnew;	
	}


	float getAudio () {
	return output;
	}


};

}; // namespace rack
