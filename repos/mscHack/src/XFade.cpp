#include "mscHack.hpp"
#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
#include "CLog.h"

#define CHANNELS 3

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct XFade : Module 
{
	enum ParamIds 
    {
        PARAM_MIX,
        PARAM_LEVEL,
        nPARAMS
    };

	enum InputIds 
    {
        IN_MIXCV,
        IN_AL,
        IN_AR           = IN_AL + CHANNELS,
        IN_BL           = IN_AR + CHANNELS,
        IN_BR           = IN_BL + CHANNELS,
        nINPUTS         = IN_BR + CHANNELS
	};

	enum OutputIds 
    {
        OUT_L,
        OUT_R       = OUT_L + CHANNELS,
        nOUTPUTS    = OUT_R + CHANNELS
	};

    CLog            lg;

    // Contructor
	XFade() : Module(nPARAMS, nINPUTS, nOUTPUTS){}

    // Overrides 
	void    step() override;
    //json_t* toJson() override;
    //void    fromJson(json_t *rootJ) override;
    void    randomize() override;
    void    reset() override;
};

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
XFade_Widget::XFade_Widget() 
{
    int i, x, y;

	XFade *module = new XFade();
	setModule(module);
	box.size = Vec( 15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/XFade.svg")));
		addChild(panel);
	}

    //module->lg.Open("XFade.txt");

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365))); 
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

    x = 10;
    y = 47;

    for( i = 0; i < CHANNELS; i++ )
    {
        // audio A
        addInput(createInput<MyPortInSmall>( Vec( x, y ), module, XFade::IN_AL + i ) );
        addInput(createInput<MyPortInSmall>( Vec( x + 28, y ), module, XFade::IN_AR + i ) );

        // audio B
        addInput(createInput<MyPortInSmall>( Vec( x, y + 32 ), module, XFade::IN_BL + i ) );
        addInput(createInput<MyPortInSmall>( Vec( x + 28, y + 32 ), module, XFade::IN_BR + i ) );

        // audio  outputs
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 83, y ), module, XFade::OUT_L + i ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 83, y + 32 ), module, XFade::OUT_R + i ) );

        y += 67;
    }

    // mix CV
    addInput(createInput<MyPortInSmall>( Vec( 4, 263 ), module, XFade::IN_MIXCV) );

    // mix knobs
    addParam(createParam<Yellow2_Huge>( Vec( 30, 243 ), module, XFade::PARAM_MIX, -1.0, 1.0, 0.0 ) );
    addParam(createParam<Yellow2_Huge>( Vec( 30, 313 ), module, XFade::PARAM_LEVEL, 0.0, 2.0, 1.0 ) );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void XFade::reset()
{
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void XFade::randomize()
{
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void XFade::step() 
{
    int i;
    float mix, mixa, mixb;

    if( inputs[ IN_MIXCV ].active )
        mix = inputs[ IN_MIXCV ].value / AUDIO_MAX;
    else
        mix = params[ PARAM_MIX ].value;

    if( mix <= 0.0 )
    {
        mixa = 1.0;
        mixb = 1.0 + mix;
    }
    else
    {
        mixb = 1.0;
        mixa = 1.0 - mix;
    }

    for( i = 0; i < CHANNELS; i++ )
    {
        outputs[ OUT_L + i ].value = ( ( inputs[ IN_AL + i ].value * mixa ) + ( inputs[ IN_BL + i ].value * mixb ) ) * params[ PARAM_LEVEL ].value;
        outputs[ OUT_R + i ].value = ( ( inputs[ IN_AR + i ].value * mixa ) + ( inputs[ IN_BR + i ].value * mixb ) ) * params[ PARAM_LEVEL ].value;
    }
}