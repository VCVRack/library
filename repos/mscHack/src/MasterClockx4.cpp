#include "mscHack.hpp"
#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
#include "CLog.h"

#define nCHANNELS 4
const int multdisplayval[ 25 ] = { 32, 24, 16, 12, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 16, 24, 32 };
//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct MasterClockx4 : Module 
{
	enum ParamIds 
    {
        PARAM_BPM,
        PARAM_MULT,
        PARAM_HUMANIZE  = PARAM_MULT + nCHANNELS,
        nPARAMS
    };

	enum InputIds 
    {
        INPUT_EXT_SYNC,
        INPUT_CHAIN     = INPUT_EXT_SYNC + nCHANNELS,
        nINPUTS         
	};

	enum OutputIds 
    {
        OUTPUT_CLK,
        OUTPUT_TRIG     = OUTPUT_CLK + ( nCHANNELS * 4 ),
        OUTPUT_CHAIN    = OUTPUT_TRIG + ( nCHANNELS * 4 ),
        nOUTPUTS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    float                m_fBPM = 120;

    MyLED7DigitDisplay  *m_pDigitDisplayMult[ nCHANNELS ] = {};
    MyLED7DigitDisplay  *m_pDigitDisplayBPM  = NULL;

    MyLEDButton         *m_pButtonGlobalStop = NULL;
    MyLEDButton         *m_pButtonGlobalTrig = NULL;
    MyLEDButton         *m_pButtonStop[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonTrig[ nCHANNELS ] = {};

    bool                m_bChannelSync[ nCHANNELS ] = {};
    bool                m_bGlobalSync = false;
    bool                m_bStopState[ nCHANNELS ] = {};
    bool                m_bGlobalStopState = false;

    int                 m_ChannelDivBeatCount[ nCHANNELS ] = {};
    float               m_fChannelBeatsPers[ nCHANNELS ] = {};
    float               m_fChannelClockCount[ nCHANNELS ] = {};
    int                 m_ChannelMultSelect[ nCHANNELS ] = {};

    float               m_fHumanize = 0;
    float               m_bWasChained = false;

    float               m_fBeatsPers;
    float               m_fMainClockCount;

    PulseGenerator      m_PulseClock[ nCHANNELS ];
    PulseGenerator      m_PulseSync[ nCHANNELS ];
    SchmittTrigger      m_SchmittSyncIn[ nCHANNELS ];

    ParamWidget        *m_pBpmKnob = NULL;
    ParamWidget        *m_pHumanKnob = NULL;

    // Contructor
	MasterClockx4() : Module(nPARAMS, nINPUTS, nOUTPUTS, 0){}

    //-----------------------------------------------------
    // MyHumanize_Knob
    //-----------------------------------------------------
    struct MyHumanize_Knob : Yellow2_Small
    {
        MasterClockx4 *mymodule;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule && !mymodule->inputs[ INPUT_CHAIN ].active )
                mymodule->GetNewHumanizeVal();

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyBPM_Knob
    //-----------------------------------------------------
    struct MyBPM_Knob : Yellow2_Huge
    {
        MasterClockx4 *mymodule;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule && !mymodule->inputs[ INPUT_CHAIN ].active  )
                mymodule->BPMChange( value, false );

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyMult_Knob
    //-----------------------------------------------------
    
    struct MyMult_Knob : Yellow2_Snap
    {
        MasterClockx4 *mymodule;
        int param, col;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule )
            {
                param = paramId - MasterClockx4::PARAM_MULT;

                if( mymodule->m_ChannelMultSelect[ param ] != (int)value )
                {
                    if( (int)value < 12 )
                    {
                        col = DWRGB( 0xFF, 0, 0 );
                    }
                    else if( (int)value > 12 )
                    {
                        col = DWRGB( 0, 0xFF, 0xFF );
                    }
                    else
                    {
                        col = DWRGB( 0xFF, 0xFF, 0xFF );
                    }

                    if( mymodule->m_pDigitDisplayMult[ param ] )
                    {
                        mymodule->m_ChannelMultSelect[ param ] = (int)value;
                        mymodule->m_pDigitDisplayMult[ param ]->SetLEDCol( col );
                        mymodule->m_pDigitDisplayMult[ param ]->SetInt( multdisplayval[ (int)value ] );
                        mymodule->CalcChannelClockRate( param );
                    }
                }
            }

		    RoundKnob::onChange( e );
	    }
    };

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    reset() override;

    void    GetNewHumanizeVal( void );
    void    BPMChange( float fbmp, bool bforce );
    void    CalcChannelClockRate( int ch );
};

//-----------------------------------------------------
// MyLEDButton_GlobalStop
//-----------------------------------------------------
void MyLEDButton_GlobalStop( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bGlobalStopState = bOn;
}

//-----------------------------------------------------
// MyLEDButton_GlobalTrig
//-----------------------------------------------------
void MyLEDButton_GlobalTrig( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bGlobalSync = true;

    if( mymodule->m_pButtonTrig[ 0 ] )
        mymodule->m_pButtonTrig[ 0 ]->Set( true );

    if( mymodule->m_pButtonTrig[ 1 ] )
        mymodule->m_pButtonTrig[ 1 ]->Set( true );

    if( mymodule->m_pButtonTrig[ 2 ] )
        mymodule->m_pButtonTrig[ 2 ]->Set( true );

    if( mymodule->m_pButtonTrig[ 3 ] )
        mymodule->m_pButtonTrig[ 3 ]->Set( true );
}

//-----------------------------------------------------
// MyLEDButton_ChannelStop
//-----------------------------------------------------
void MyLEDButton_ChannelStop ( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bStopState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_ChannelSync
//-----------------------------------------------------
void MyLEDButton_ChannelSync( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bChannelSync[ id ] = true;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
MasterClockx4_Widget::MasterClockx4_Widget() 
{
    int ch, x, y;
	MasterClockx4 *module = new MasterClockx4();
	setModule(module);
	box.size = Vec( 15*25, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin( plugin, "res/MasterClockx4.svg")));
		addChild(panel);
	}

    //module->lg.Open("MasterClockx4.txt");

    // bpm knob
    module->m_pBpmKnob = createParam<MasterClockx4::MyBPM_Knob>( Vec( 9, 50 ), module, MasterClockx4::PARAM_BPM, 60.0, 220.0, 120.0 );
    addParam( module->m_pBpmKnob );

    // bpm display
    module->m_pDigitDisplayBPM = new MyLED7DigitDisplay( 5, 115, 0.055, DWRGB( 0, 0, 0 ), DWRGB( 0xFF, 0xFF, 0xFF ), MyLED7DigitDisplay::TYPE_FLOAT, 5 );
    addChild( module->m_pDigitDisplayBPM );

    // global stop switch
    module->m_pButtonGlobalStop = new MyLEDButton( 22, 144, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, MyLEDButton_GlobalStop );
	addChild( module->m_pButtonGlobalStop );

    // global sync button
    module->m_pButtonGlobalTrig = new MyLEDButton( 22, 202, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, MyLEDButton_GlobalTrig );
	addChild( module->m_pButtonGlobalTrig );

    // humanize knob
    module->m_pHumanKnob = createParam<MasterClockx4::MyHumanize_Knob>( Vec( 22, 235 ), module, MasterClockx4::PARAM_HUMANIZE, 0.0, 1.0, 0.0 );
    addParam( module->m_pHumanKnob );

    // add chain out
    addOutput(createOutput<MyPortOutSmall>( Vec( 30, 345 ), module, MasterClockx4::OUTPUT_CHAIN ) );

    // chain in
    addInput(createInput<MyPortInSmall>( Vec( 30, 13 ), module, MasterClockx4::INPUT_CHAIN ) );

    x = 74;
    y = 36;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        // clock mult knob
        addParam(createParam<MasterClockx4::MyMult_Knob>( Vec( x + 36, y + 8 ), module, MasterClockx4::PARAM_MULT + ch, 0, 24, 12 ) );

        // mult display
        module->m_pDigitDisplayMult[ ch ] = new MyLED7DigitDisplay( x + 33, y + 44, 0.07, DWRGB( 0, 0, 0 ), DWRGB( 0xFF, 0xFF, 0xFF ), MyLED7DigitDisplay::TYPE_INT, 2 );
        addChild( module->m_pDigitDisplayMult[ ch ] );

        // sync triggers
        module->m_pButtonTrig[ ch ] = new MyLEDButton( x + 192, y + 6, 19, 19, 15.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_ChannelSync );
	    addChild( module->m_pButtonTrig[ ch ] );

        addInput(createInput<MyPortInSmall>( Vec( x + 170, y + 7 ), module, MasterClockx4::INPUT_EXT_SYNC + ch ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 170, y + 33 ), module, MasterClockx4::OUTPUT_TRIG  + (ch * 4) + 0 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 170, y + 55 ), module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 1 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 193, y + 33 ), module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 2 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 193, y + 55 ), module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 3 ) );

        // clock out
        module->m_pButtonStop[ ch ] = new MyLEDButton( x + 192 + 56, y + 6, 19, 19, 15.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_ChannelStop );
	    addChild( module->m_pButtonStop[ ch ] );

        addOutput(createOutput<MyPortOutSmall>( Vec( x + 170 + 56, y + 33 ), module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 0 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 170 + 56, y + 55 ), module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 1 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 193 + 56, y + 33 ), module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 2 ) );
        addOutput(createOutput<MyPortOutSmall>( Vec( x + 193 + 56, y + 55 ), module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 3 ) );

        y += 80;
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
json_t *MasterClockx4::toJson() 
{
    bool *pbool;
	json_t *rootJ = json_object();

    // m_bGlobalStopState
    json_object_set_new( rootJ, "m_bGlobalStopState", json_boolean (m_bGlobalStopState) );

	// m_bStopState
    pbool = &m_bStopState[ 0 ];

	json_t *gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_boolean( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bStopState", gatesJ );

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void MasterClockx4::fromJson(json_t *rootJ) 
{
   bool *pbool;

    // m_bGlobalStopState
	json_t *revJ = json_object_get(rootJ, "m_bGlobalStopState");

	if (revJ)
		m_bGlobalStopState = json_is_true( revJ );

	// m_bPauseState
    pbool = &m_bStopState[ 0 ];

	json_t *StepsJ = json_object_get(rootJ, "m_bStopState");

	if (StepsJ) 
    {
		for (int i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_boolean_value( gateJ );
		}
	}

    m_pButtonGlobalStop->Set( m_bGlobalStopState );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_ChannelMultSelect[ ch ] = (int)params[ PARAM_MULT + ch ].value;

        m_pButtonStop[ ch ]->Set( m_bStopState[ ch ] );

        if( m_pDigitDisplayMult[ ch ] )
            m_pDigitDisplayMult[ ch ]->SetInt( multdisplayval[ (int)params[ PARAM_MULT + ch ].value ] );
    }

    m_fMainClockCount = 0;
    BPMChange( params[ PARAM_BPM ].value, true );

    if( m_pDigitDisplayBPM )
        m_pDigitDisplayBPM->SetFloat( m_fBPM );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void MasterClockx4::reset()
{
    if( !m_bInitialized )
        return;

    m_fBPM = 120;

    if( m_pDigitDisplayBPM )
        m_pDigitDisplayBPM->SetFloat( m_fBPM );

    m_bGlobalStopState = false;
    m_pButtonGlobalStop->Set( m_bGlobalStopState );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_ChannelMultSelect[ ch ] = (int)params[ PARAM_MULT + ch ].value;

        m_bStopState[ ch ] = false;
        m_pButtonStop[ ch ]->Set( m_bStopState[ ch ] );

        if( m_pDigitDisplayMult[ ch ] )
            m_pDigitDisplayMult[ ch ]->SetInt( multdisplayval[ (int)params[ PARAM_MULT + ch ].value ] );
    }

    BPMChange( m_fBPM, true );
}

//-----------------------------------------------------
// Procedure:   GetNewHumanizeVal
//
//-----------------------------------------------------
void MasterClockx4::GetNewHumanizeVal( void )
{
    m_fHumanize = randomf() * engineGetSampleRate() * 0.1 * params[ PARAM_HUMANIZE ].value;

    if( randomf() > 0.5 )
        m_fHumanize *= -1;
}

//-----------------------------------------------------
// Procedure:   BMPChange
//
//-----------------------------------------------------
void MasterClockx4::BPMChange( float fbpm, bool bforce )
{
    // don't change if it is already the same
    if( !bforce && ( (int)(fbpm * 1000.0f ) == (int)(m_fBPM * 1000.0f ) ) )
        return;

    m_fBPM = fbpm;
    m_fBeatsPers = fbpm / 60.0;

    if( m_pDigitDisplayBPM )
       m_pDigitDisplayBPM->SetFloat( m_fBPM );

    for( int i = 0; i < nCHANNELS; i++ )
        CalcChannelClockRate( i );
}

//-----------------------------------------------------
// Procedure:   CalcChannelClockRate
//
//-----------------------------------------------------
void MasterClockx4::CalcChannelClockRate( int ch )
{
    // for beat division just keep a count of beats
    if( m_ChannelMultSelect[ ch ] <= 12 )
        m_ChannelDivBeatCount[ ch ] = multdisplayval[ m_ChannelMultSelect[ ch ] ];
    else
        m_fChannelBeatsPers[ ch ] = m_fBeatsPers * (float)multdisplayval[ m_ChannelMultSelect[ ch ] ];
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void MasterClockx4::step() 
{
    int ch;
    float fSyncPulseOut, fClkPulseOut;
    bool bMainClockTrig = false, bChannelClockTrig;

    if( !m_bInitialized )
        return;

    // use the input chain trigger for our clock
    if( inputs[ INPUT_CHAIN ].active )
    {
        if( !m_bWasChained )
        {
            m_pHumanKnob->visible = false;
            m_pBpmKnob->visible = false;
        }

        m_bWasChained = true;

        // value of less than zero is a trig
        if( inputs[ INPUT_CHAIN ].value < 10.0 )
        {
            bMainClockTrig = true;
        }
        // values greater than zero are the bpm
        else
        {
            BPMChange( inputs[ INPUT_CHAIN ].value, false );
        }
    }
    else
    {
        // go back to our bpm if chain removed
        if( m_bWasChained )
        {
            m_pHumanKnob->visible = true;
            m_pBpmKnob->visible = true;
            m_bWasChained = false;
            BPMChange( params[ PARAM_BPM ].value, false );
        }

        // keep track of main bpm
        m_fMainClockCount += m_fBeatsPers;
        if( ( m_fMainClockCount + m_fHumanize ) >= engineGetSampleRate() )
        {
            m_fMainClockCount = ( m_fMainClockCount + m_fHumanize ) - engineGetSampleRate();

            GetNewHumanizeVal();

            bMainClockTrig = true;
        }
    }

    // send chain
    if( outputs[ OUTPUT_CHAIN ].active )
    {
        if( bMainClockTrig )
            outputs[ OUTPUT_CHAIN ].value = -1.0;
        else
            outputs[ OUTPUT_CHAIN ].value = m_fBPM;
    }

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        bChannelClockTrig = false;

        // do multiple clocks
        if( m_ChannelMultSelect[ ch ] > 12 )
        {
            m_fChannelClockCount[ ch ] += m_fChannelBeatsPers[ ch ];
            if( m_fChannelClockCount[ ch ] >= engineGetSampleRate() )
            {
                m_fChannelClockCount[ ch ] = m_fChannelClockCount[ ch ] - engineGetSampleRate();
                bChannelClockTrig = true;
            }
        }

        // sync all triggers to main clock pulse
        if( bMainClockTrig )
        {
            // divisions of clock will count beats
            if( m_ChannelMultSelect[ ch ] <= 12 )
            {
                if( ++m_ChannelDivBeatCount[ ch ] >= multdisplayval[ m_ChannelMultSelect[ ch ] ] )
                {
                    m_ChannelDivBeatCount[ ch ] = 0;
                    bChannelClockTrig = true;
                }
            }
            // multiples of clock will sync with every beat
            else
            {
                m_fChannelClockCount[ ch ] = 0;
                bChannelClockTrig = true;
            }
        }

        if( bChannelClockTrig )
            m_PulseClock[ ch ].trigger(1e-3);

        // blinky sync LED on input trig
        if( m_SchmittSyncIn[ ch ].process( inputs[ INPUT_EXT_SYNC + ch ].value ) )
        {
            m_bChannelSync[ ch ] = true;

            if( m_pButtonTrig[ ch ] )
                m_pButtonTrig[ ch ]->Set( true );
        }

        // sync triggers
        if( m_bChannelSync[ ch ] || m_bGlobalSync || m_SchmittSyncIn[ ch ].process( inputs[ INPUT_EXT_SYNC + ch ].value ) )
        {
            m_bChannelSync[ ch ] = false;
            m_PulseSync[ ch ].trigger(1e-3);
        }

        // syncs
        fSyncPulseOut = m_PulseSync[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;
        outputs[ OUTPUT_TRIG + (ch * 4) + 0 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 1 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 2 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 3 ].value = fSyncPulseOut;

        // clocks
        fClkPulseOut = m_PulseClock[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        if( !m_bGlobalStopState && !m_bStopState[ ch ] )
        {
            outputs[ OUTPUT_CLK + (ch * 4) + 0 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 1 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 2 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 3 ].value = fClkPulseOut;
        }
    }

    m_bGlobalSync = false;
}