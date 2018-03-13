#include "mscHack.hpp"
#include "mscHack_Controls.hpp"

Plugin *plugin;

void init(rack::Plugin *p) 
{
	plugin = p;
	plugin->slug = "mscHack";
	plugin->website = "https://github.com/mschack/VCV-Rack-Plugins";

    p->addModel( createModel<MasterClockx4_Widget>   ( "mscHack", "MasterClockx4", "Master CLOCK x 4", CLOCK_TAG, QUAD_TAG ) );
    p->addModel( createModel<Seq_3x16x16_Widget>     ( "mscHack", "Seq_3ch_16step", "SEQ 3 x 16", SEQUENCER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<SEQ_6x32x16_Widget>     ( "mscHack", "Seq_6ch_32step", "SEQ 6 x 32", SEQUENCER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Seq_Triad_Widget>       ( "mscHack", "TriadSeq", "SEQ Triad OBSOLETE!", SEQUENCER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Seq_Triad2_Widget>      ( "mscHack", "TriadSeq2", "SEQ Triad 2", SEQUENCER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<ARP700_Widget>          ( "mscHack", "ARP700", "ARP 700", SEQUENCER_TAG, OSCILLATOR_TAG ) );
    p->addModel( createModel<SynthDrums_Widget>      ( "mscHack", "SynthDrums", "SYNTH Drums", DRUM_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<XFade_Widget>           ( "mscHack", "XFade", "MIXER Cross Fader 3 Channel", MIXER_TAG, MULTIPLE_TAG) );
    p->addModel( createModel<Mix_1x4_Stereo_Widget>  ( "mscHack", "Mix_1x4_Stereo", "MIXER 1x4 Stereo/Mono", MIXER_TAG, EQUALIZER_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Mix_2x4_Stereo_Widget>  ( "mscHack", "Mix_2x4_Stereo", "MIXER 2x4 Stereo/Mono", MIXER_TAG, EQUALIZER_TAG, DUAL_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Mix_4x4_Stereo_Widget>  ( "mscHack", "Mix_4x4_Stereo(2)", "MIXER 4x4 Stereo/Mono", MIXER_TAG, EQUALIZER_TAG, QUAD_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Mix_4x4_Stereo_Widget_old>  ( "mscHack", "Mix_4x4_Stereo", "MIXER 4x4 (old) OBSOLETE!", MIXER_TAG, QUAD_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<PingPong_Widget>        ( "mscHack", "PingPong_Widget", "DELAY Ping Pong", DELAY_TAG, PANNING_TAG ) );
    p->addModel( createModel<Osc_3Ch_Widget>         ( "mscHack", "Osc_3Ch_Widget", "OSC 3 Channel", SYNTH_VOICE_TAG, OSCILLATOR_TAG, MULTIPLE_TAG ) );
    p->addModel( createModel<Compressor_Widget>      ( "mscHack", "Compressor1", "COMP Basic Compressor", DYNAMICS_TAG ) );
}

//-----------------------------------------------------
// Procedure: JsonDataInt  
//
//-----------------------------------------------------
void JsonDataInt( bool bTo, std::string strName, json_t *root, int *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_integer( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_integer_value( js );
		    }
        }
    }
}

//-----------------------------------------------------
// Procedure: JsonDataBool  
//
//-----------------------------------------------------
void JsonDataBool( bool bTo, std::string strName, json_t *root, bool *pdata, int len )
{
    int i;
    json_t *jsarray, *js;

    if( !pdata || !root || len <= 0 )
        return;

    if( bTo )
    {
        jsarray = json_array();

        for ( i = 0; i < len; i++ )
        {
	        js = json_boolean( pdata[ i ] );
	        json_array_append_new( jsarray, js );
        }

        json_object_set_new( root, strName.c_str(), jsarray );
    }
    else
    {
        jsarray = json_object_get( root, strName.c_str() );

        if( jsarray )
        {
		    for ( i = 0; i < len; i++)
            {
			    js = json_array_get( jsarray, i );

			    if( js )
				    pdata[ i ] = json_boolean_value( js );
		    }
        }
    }
}
