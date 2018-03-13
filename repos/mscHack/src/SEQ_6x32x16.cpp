#include "mscHack.hpp"
#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
#include "CLog.h"

#define nCHANNELS 6
#define nSTEPS 32
#define nPROG 16

typedef struct
{
    bool    bPending;
    int     prog;
}PHRASE_CHANGE_STRUCT;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct SEQ_6x32x16 : Module 
{
	enum ParamIds 
    {
        PARAM_CPY_NEXT,
        PARAM_RAND          = PARAM_CPY_NEXT + nCHANNELS,
        PARAM_PAUSE         = PARAM_RAND + nCHANNELS,
        PARAM_BILEVEL       = PARAM_PAUSE + nCHANNELS,
        PARAM_LO_KNOB       = PARAM_BILEVEL + nCHANNELS,
        PARAM_MD_KNOB       = PARAM_LO_KNOB + nCHANNELS,
        PARAM_HI_KNOB       = PARAM_MD_KNOB + nCHANNELS,
        PARAM_SWING_KNOB    = PARAM_HI_KNOB + nCHANNELS,
        PARAM_SWING_KNOB2   = PARAM_SWING_KNOB + nCHANNELS,

        nPARAMS             = PARAM_SWING_KNOB2 + nCHANNELS
    };

	enum InputIds 
    {
        IN_GLOBAL_CLK_RESET,
        IN_GLOBAL_PAT_CHANGE,
        IN_CLK,
        IN_PAT_TRIG     = IN_CLK + nCHANNELS,
        nINPUTS         = IN_PAT_TRIG + nCHANNELS
	};

	enum OutputIds 
    {
        OUT_TRIG,
        OUT_LEVEL    = OUT_TRIG + nCHANNELS,
        OUT_BEAT1    = OUT_LEVEL + nCHANNELS,
        nOUTPUTS     = OUT_BEAT1 + nCHANNELS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    bool            m_bPauseState[ nCHANNELS ] = {};

    bool            m_bBiLevelState[ nCHANNELS ] = {};

    int             m_RandCount[ nCHANNELS ] = {};
    int             m_CpyNextCount[ nCHANNELS ] = {};

    SinglePatternClocked32  *m_pPatternDisplay[ nCHANNELS ] = {};
    int                     m_Pattern[ nCHANNELS ][ nPROG ][ nSTEPS ];
    int                     m_MaxPat[ nCHANNELS ][ nPROG ] = {};

    PatternSelectStrip      *m_pProgramDisplay[ nCHANNELS ] = {};
    int                     m_CurrentProg[ nCHANNELS ] = {};
    int                     m_MaxProg[ nCHANNELS ] = {};
    PHRASE_CHANGE_STRUCT    m_ProgPending[ nCHANNELS ] = {};

    SchmittTrigger          m_SchTrigClock[ nCHANNELS ];
    SchmittTrigger          m_SchTrigProg[ nCHANNELS ];
    SchmittTrigger          m_SchTrigGlobalClkReset;
    SchmittTrigger          m_SchTrigGlobalProg;

    bool                    m_bGlobalClocked[ nCHANNELS ] = {};

    bool                    m_bTrig[ nCHANNELS ] = {};
    PulseGenerator          m_gatePulse[ nCHANNELS ];
    PulseGenerator          m_gateBeatPulse[ nCHANNELS ];

    // swing
    int                     m_SwingLen[ nCHANNELS ] = {0};
    int                     m_SwingCount[ nCHANNELS ] = {0};
    int                     m_ClockTick[ nCHANNELS ] = {0};

    // buttons    
    MyLEDButton             *m_pButtonPause[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonCopy[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonBiLevel[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonRand[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonAutoPat[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonHoldCV[ nCHANNELS ] = {};

    bool                    m_bAutoPatChange[ nCHANNELS ] = {};
    bool                    m_bHoldCVState[ nCHANNELS ] = {};

    // Contructor
	SEQ_6x32x16() : Module(nPARAMS, nINPUTS, nOUTPUTS, 0){}

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    randomize() override;
    void    reset() override;

    void    CpyNext( int ch );
    void    Rand( int ch );
    void    ChangeProg( int ch, int prog, bool force );
    void    SetPendingProg( int ch, int prog );

    //void    reset() override;
};

//-----------------------------------------------------
// MyLEDButton_AutoPat
//-----------------------------------------------------
void MyLEDButton_AutoPat( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bAutoPatChange[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_Pause
//-----------------------------------------------------
void MyLEDButton_Pause( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bPauseState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_HoldCV
//-----------------------------------------------------
void MyLEDButton_HoldCV( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bHoldCVState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_BiLevel
//-----------------------------------------------------
void MyLEDButton_BiLevel( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bBiLevelState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_CpyNxt
//-----------------------------------------------------
void MyLEDButton_CpyNxt( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->CpyNext( id );
}

//-----------------------------------------------------
// MyLEDButton_Rand
//-----------------------------------------------------
void MyLEDButton_Rand( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->Rand( id );
}

//-----------------------------------------------------
// Procedure:   PatternChangeCallback
//
//-----------------------------------------------------
void SEQ_6x32x16_PatternChangeCallback ( void *pClass, int ch, int pat, int level, int maxpat )
{
    SEQ_6x32x16 *mymodule = (SEQ_6x32x16 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    mymodule->m_MaxPat[ ch ][ mymodule->m_CurrentProg[ ch ] ] = maxpat;
    mymodule->m_Pattern[ ch ][ mymodule->m_CurrentProg[ ch ] ] [ pat ] = level;
}

//-----------------------------------------------------
// Procedure:   ProgramChangeCallback
//
//-----------------------------------------------------
void SEQ_6x32x16_ProgramChangeCallback ( void *pClass, int ch, int pat, int max )
{
    SEQ_6x32x16 *mymodule = (SEQ_6x32x16 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    if( mymodule->m_CurrentProg[ ch ] != pat )
    {
        if( !mymodule->m_bPauseState[ ch ] && mymodule->inputs[ SEQ_6x32x16::IN_CLK + ch ].active )
            mymodule->SetPendingProg( ch, pat );
        else
            mymodule->ChangeProg( ch, pat, false );
    }
    else if( mymodule->m_MaxProg[ ch ] != max )
    {
        mymodule->m_MaxProg[ ch ] = max;
    }
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
SEQ_6x32x16_Widget::SEQ_6x32x16_Widget() 
{
    int x, y, x2, y2;
	SEQ_6x32x16 *module = new SEQ_6x32x16();
	setModule(module);
	box.size = Vec( 15*41, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SEQ_6x32x16.svg")));
		addChild(panel);
	}

    //module->lg.Open("SEQ_6x32x16.txt");

    x = 7;
    y = 22;

    // global inputs
    addInput(createInput<MyPortInSmall>( Vec( 204, 357 ), module, SEQ_6x32x16::IN_GLOBAL_CLK_RESET ) );
    addInput(createInput<MyPortInSmall>( Vec( 90, 357 ), module, SEQ_6x32x16::IN_GLOBAL_PAT_CHANGE ) );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        // inputs
        addInput(createInput<MyPortInSmall>( Vec( x + 6, y + 7 ), module, SEQ_6x32x16::IN_CLK + ch ) );
        addInput(createInput<MyPortInSmall>( Vec( x + 64, y + 31 ), module, SEQ_6x32x16::IN_PAT_TRIG + ch ) );

        // pattern display
        module->m_pPatternDisplay[ ch ] = new SinglePatternClocked32( x + 39, y + 2, 13, 13, 5, 2, 7, DWRGB( 255, 128, 64 ), DWRGB( 18, 18, 0 ), DWRGB( 180, 75, 180 ), DWRGB( 80, 45, 80 ), nSTEPS, ch, module, SEQ_6x32x16_PatternChangeCallback );
	    addChild( module->m_pPatternDisplay[ ch ] );

        // program display
        module->m_pProgramDisplay[ ch ] = new PatternSelectStrip( x + 106, y + 31, 9, 7, DWRGB( 180, 180, 0 ), DWRGB( 90, 90, 64 ), DWRGB( 0, 180, 200 ), DWRGB( 0, 90, 90 ), nPROG, ch, module, SEQ_6x32x16_ProgramChangeCallback );
	    addChild( module->m_pProgramDisplay[ ch ] );

        // add knobs
        y2 = y + 34;
        addParam(createParam<Green1_Tiny>( Vec( x + 374, y2 ), module, SEQ_6x32x16::PARAM_SWING_KNOB + ch, 0.0, 0.6, 0.0 ) );

        x2 = x + 447;
        addParam(createParam<Green1_Tiny>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LO_KNOB + ch, 0.0, 1.0, 0.0 ) ); x2 += 24;
        addParam(createParam<Green1_Tiny>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_MD_KNOB + ch, 0.0, 1.0, 0.0 ) ); x2 += 24;
        addParam(createParam<Green1_Tiny>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_HI_KNOB + ch, 0.0, 1.0, 0.0 ) );

        // add buttons
        module->m_pButtonAutoPat[ ch ] = new MyLEDButton( x + 55, y + 35, 9, 9, 6.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_AutoPat );
	    addChild( module->m_pButtonAutoPat[ ch ] );

        module->m_pButtonPause[ ch ] = new MyLEDButton( x + 26, y + 10, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_Pause );
	    addChild( module->m_pButtonPause[ ch ] );

        y2 = y + 33;
        module->m_pButtonCopy[ ch ] = new MyLEDButton( x + 290, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_CpyNxt );
	    addChild( module->m_pButtonCopy[ ch ] );

        module->m_pButtonRand[ ch ] = new MyLEDButton( x + 315, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_Rand );
	    addChild( module->m_pButtonRand[ ch ] );

        module->m_pButtonHoldCV[ ch ] = new MyLEDButton( x + 405, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_HoldCV );
	    addChild( module->m_pButtonHoldCV[ ch ] );

        module->m_pButtonBiLevel[ ch ] = new MyLEDButton( x + 425, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_BiLevel );
	    addChild( module->m_pButtonBiLevel[ ch ] );

        // add outputs
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 580, y + 7 ), module, SEQ_6x32x16::OUT_TRIG + ch ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 544, y + 33 ), module, SEQ_6x32x16::OUT_LEVEL + ch ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 37, y + 31 ), module, SEQ_6x32x16::OUT_BEAT1 + ch ) );

        y += 56;
    }

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365))); 
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

    module->m_bInitialized = true;

    reset();
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *SEQ_6x32x16::toJson() 
{
    bool *pbool;
    int  *pint;
	json_t *rootJ = json_object();

	// m_bPauseState
    pbool = &m_bPauseState[ 0 ];

	json_t *gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_boolean( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bPauseState", gatesJ );

	// m_bBiLevelState
    pbool = &m_bBiLevelState[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_boolean( pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bBiLevelState", gatesJ );

	// m_Pattern
    pint = &m_Pattern[ 0 ][ 0 ][ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS * nSTEPS * nPROG; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_Pattern", gatesJ );

	// m_MaxPat
    pint = &m_MaxPat[ 0 ][ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS * nPROG; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_MaxPat", gatesJ );

	// m_CurrentProg
    pint = &m_CurrentProg[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_CurrentProg", gatesJ );

	// m_MaxProg
    pint = &m_MaxProg[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_MaxProg", gatesJ );

	// m_bAutoPatChange
    pbool = &m_bAutoPatChange[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_boolean( pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bAutoPatChange", gatesJ );

	// m_bHoldCVState
    pbool = &m_bHoldCVState[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_boolean( pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bHoldCVState", gatesJ );

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void SEQ_6x32x16::fromJson(json_t *rootJ) 
{
   bool *pbool;
    int *pint;

	// m_bPauseState
    pbool = &m_bPauseState[ 0 ];

	json_t *StepsJ = json_object_get(rootJ, "m_bPauseState");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_boolean_value( gateJ );
		}
	}

	// m_bBiLevelState
    pbool = &m_bBiLevelState[ 0 ];

	StepsJ = json_object_get(rootJ, "m_bBiLevelState");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_boolean_value( gateJ );
		}
	}

	// m_Pattern
    pint = &m_Pattern[ 0 ][ 0 ][ 0 ];

	StepsJ = json_object_get(rootJ, "m_Pattern");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS * nSTEPS * nPROG; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

	// m_MaxPat
    pint = &m_MaxPat[ 0 ][ 0 ];

	StepsJ = json_object_get(rootJ, "m_MaxPat");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS * nPROG; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

	// m_CurrentProg
    pint = &m_CurrentProg[ 0 ];

	StepsJ = json_object_get(rootJ, "m_CurrentProg");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

	// m_MaxProg
    pint = &m_MaxProg[ 0 ];

	StepsJ = json_object_get(rootJ, "m_MaxProg");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

	// m_bAutoPatChange
    pbool = &m_bAutoPatChange[ 0 ];

	StepsJ = json_object_get(rootJ, "m_bAutoPatChange");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_boolean_value( gateJ );
		}
	}

	// m_bHoldCVState
    pbool = &m_bHoldCVState[ 0 ];

	StepsJ = json_object_get(rootJ, "m_bHoldCVState");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_boolean_value( gateJ );
		}
	}

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonAutoPat[ ch ]->Set( m_bAutoPatChange[ ch ] );
        m_pButtonPause[ ch ]->Set( m_bPauseState[ ch ] );
        m_pButtonBiLevel[ ch ]->Set( m_bBiLevelState[ ch ] );
        m_pButtonHoldCV[ ch ]->Set( m_bHoldCVState[ ch ] );

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
        m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ m_CurrentProg[ ch ] ] );

        m_pProgramDisplay[ ch ]->SetPat( m_CurrentProg[ ch ], false );
        m_pProgramDisplay[ ch ]->SetMax( m_MaxProg[ ch ] );
    }
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void SEQ_6x32x16::reset()
{
    if( !m_bInitialized )
        return;

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonPause[ ch ]->Set( false );
        m_pButtonBiLevel[ ch ]->Set( false );
    }

    memset( m_bPauseState, 0, sizeof(m_bPauseState) );
    memset( m_bBiLevelState, 0, sizeof(m_bBiLevelState) );
    memset( m_Pattern, 0, sizeof(m_Pattern) );
    memset( m_CurrentProg, 0, sizeof(m_CurrentProg) );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        for( int prog = 0; prog < nCHANNELS; prog++ )
            m_MaxPat[ ch ][ prog ] = nSTEPS - 1;

        m_MaxProg[ ch ] = nPROG - 1;

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ 0 ] );
        m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ 0 ] );

        m_pProgramDisplay[ ch ]->SetPat( 0, false );
        m_pProgramDisplay[ ch ]->SetMax( m_MaxProg[ ch ] );

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );

        ChangeProg( ch, 0, true );
    }
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void SEQ_6x32x16::randomize()
{
    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        for( int p = 0; p < nPROG; p++ )
        {
            for( int i = 0; i < nSTEPS; i++ )
            {
                m_Pattern[ ch ][ p ][ i ] = (int)(randomf() * 3.4 );
            }
        }

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
    }
}

//-----------------------------------------------------
// Procedure:   Rand
//
//-----------------------------------------------------
void SEQ_6x32x16::Rand( int ch )
{
    for( int i = 0; i < nSTEPS; i++ )
    {
        m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ i ] = (int)(randomf() * 3.4 );
    }

    m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
}

//-----------------------------------------------------
// Procedure:   CpyNext
//
//-----------------------------------------------------
void SEQ_6x32x16::CpyNext( int ch )
{
    int next;

    next = m_CurrentProg[ ch ] + 1;

    if( next >= nPROG )
        return;

    memcpy( m_Pattern[ ch ][ next ], m_Pattern[ ch ][ m_CurrentProg[ ch ] ], sizeof(int) * nSTEPS );

    if(m_bPauseState[ ch ] || !inputs[ SEQ_6x32x16::IN_CLK + ch ].active )
        ChangeProg( ch, next, false );
}

//-----------------------------------------------------
// Procedure:   ChangProg
//
//-----------------------------------------------------
void SEQ_6x32x16::ChangeProg( int ch, int prog, bool bforce )
{
    if( ch < 0 || ch >= nCHANNELS )
        return;

    if( !bforce && prog == m_CurrentProg[ ch ] )
        return;

    if( prog < 0 )
        prog = nPROG - 1;
    else if( prog >= nPROG )
        prog = 0;

    m_CurrentProg[ ch ] = prog;

    m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ prog ] );
    m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ prog ] );
    m_pProgramDisplay[ ch ]->SetPat( prog, false );
}

//-----------------------------------------------------
// Procedure:   SetPendingProg
//
//-----------------------------------------------------
void SEQ_6x32x16::SetPendingProg( int ch, int progIn )
{
    int prog;

    if( progIn < 0 || progIn >= nPROG )
        prog = ( m_CurrentProg[ ch ] + 1 ) & 0xF;
    else
        prog = progIn;

    if( prog > m_MaxProg[ ch ] )
        prog = 0;

    m_ProgPending[ ch ].bPending = true;
    m_ProgPending[ ch ].prog = prog;
    m_pProgramDisplay[ ch ]->SetPat( m_CurrentProg[ ch ], false );
    m_pProgramDisplay[ ch ]->SetPat( prog, true );
}


//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void SEQ_6x32x16::step() 
{
    bool bClk, bClkTrig, bClockAtZero = false, bGlobalClk = false, bGlobalProg = false, bTrigOut;
    float fout = 0.0;
    int iclk = 0;

    if( !m_bInitialized )
        return;

    if( inputs[ IN_GLOBAL_CLK_RESET ].active )
        bGlobalClk = m_SchTrigGlobalClkReset.process( inputs[ IN_GLOBAL_CLK_RESET ].value );

    if( inputs[ IN_GLOBAL_PAT_CHANGE ].active )
        bGlobalProg= m_SchTrigGlobalProg.process( inputs[ IN_GLOBAL_PAT_CHANGE ].value );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        if( bGlobalClk )
            m_bGlobalClocked[ ch ] = true;

        bClkTrig = false;
        bClk = false;
        bTrigOut = false;
        bClockAtZero = true;

        if( inputs[ IN_CLK + ch ].active )
        {
            bClockAtZero = false;
            m_ClockTick[ ch ]++;

            // time the clock tick
            if( m_bGlobalClocked[ ch ] )
            {
                m_bGlobalClocked[ ch ] = false;
                m_SwingCount[ ch ] = m_SwingLen[ ch ];
                m_pPatternDisplay[ ch ]->ClockReset();
                bClkTrig = true;
                bClockAtZero = true;
                bClk = true;
            }

            if( m_SchTrigClock[ ch ].process( inputs[ IN_CLK + ch ].value ) )
            {
                m_SwingLen[ ch ] = m_ClockTick[ ch ] + (int)( (float)m_ClockTick[ ch ] * params[ PARAM_SWING_KNOB + ch ].value );
                m_ClockTick[ ch ] = 0;
                bClkTrig = true;
            }

            if( params[ PARAM_SWING_KNOB + ch ].value < 0.05 )
            {
                bClk = bClkTrig;
            }
            else
            {
                // beat 1, wait for count to shorten every other note
                if( !( m_pPatternDisplay[ ch ]->m_PatClk & 1 ) )
                {
                    if( --m_SwingCount[ ch ] <= 0 )
                        bClk = true;
                }
                // beat 0, wait for actual clock
                else if( bClkTrig )
                {
                    m_SwingCount[ ch ] = m_SwingLen[ ch ];
                    bClk = true;
                }
            }

            if( !m_bPauseState[ ch ] )
            {
                if( bGlobalProg || m_SchTrigProg[ ch ].process( inputs[ IN_PAT_TRIG + ch ].value ) )
                    SetPendingProg( ch, -1 );

                // clock in
                if( bClk )
                {
                    if( !bClockAtZero )
                        bClockAtZero = m_pPatternDisplay[ ch ]->ClockInc();

                    bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
                }
            }
        }

        iclk = m_pPatternDisplay[ ch ]->m_PatClk;

        // auto inc pattern
        if( m_bAutoPatChange[ ch ] && bClockAtZero )
        {
            SetPendingProg( ch, -1 );
            m_ProgPending[ ch ].bPending = false;
            ChangeProg( ch, m_ProgPending[ ch ].prog, true );
            bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
        }
        // resolve any left over phrase triggers
        else if( m_ProgPending[ ch ].bPending && bClockAtZero )
        {
            m_ProgPending[ ch ].bPending = false;
            ChangeProg( ch, m_ProgPending[ ch ].prog, false );
            bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
        }

        // trigger the beat 0 output
        if( bClockAtZero )
            m_gateBeatPulse[ ch ].trigger(1e-3);

        outputs[ OUT_BEAT1 + ch ].value = m_gateBeatPulse[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        // trigger out
        if( bTrigOut )
            m_gatePulse[ ch ].trigger(1e-3);

        outputs[ OUT_TRIG + ch ].value = m_gatePulse[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        int level = m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ iclk ];

        // get actual level from knobs
        switch( level )
        {
        case 1:
            fout = params[ PARAM_LO_KNOB + ch ].value;
            break;
        case 2:
            fout = params[ PARAM_MD_KNOB + ch ].value;
            break;
        case 3:
            fout = params[ PARAM_HI_KNOB + ch ].value;
            break;
        default:
            if( m_bHoldCVState[ ch ] )
                continue;
            else
                fout = 0.0f;
        }

        // bidirectional convert to -1.0 to 1.0
        if( m_bBiLevelState[ ch ] )
            fout = ( fout * 2 ) - 1.0;

        outputs[ OUT_LEVEL + ch ].value = CV_MAX * fout;
    }
}
