#include "Autodafe.hpp"
#include <math.h>
#include "dsp/digital.hpp"
#include "dsp/decimator.hpp"
#include "dsp/fft.hpp"
#include "dsp/filter.hpp"
#include "dsp/fir.hpp"
#include "dsp/frame.hpp"
#include "dsp/minblep.hpp"
#include "dsp/ode.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/samplerate.hpp"

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = "Autodafe - Drum Kit";
	plugin->name = "Autodafe - Drum Kit";
	plugin->homepageUrl = "https://www.autodafe.net";
		
        createModel<DrumsKickWidget>(plugin, "Drums - Kick", "Drums - Kick");
        createModel<DrumsSnareWidget>(plugin, "Drums - Snare", "Drums - Snare");
        createModel<DrumsHiHatClosedWidget>(plugin, "Drums - Closed Hats", "Drums - Closed Hats");
        createModel<DrumsHiHatOpenWidget>(plugin, "Drums - Open Hats", "Drums - Open Hats");

        
        createModel<DrumsClapsWidget>(plugin, "Drums - Claps", "Drums - Claps");
        createModel<DrumsCymbalsWidget>(plugin, "Drums - Cymbals", "Drums - Cymbals");
        createModel<DrumsRideWidget>(plugin, "Drums - Ride", "Drums - Ride");
        createModel<DrumsRimClavesWidget>(plugin, "Drums - Rim/Claves", "Drums - Rim/Claves");
     createModel<DrumsMixerWidget>(plugin, "Drums - 8-Channel Mixer", "Drums - 8-Channel Mixer");

		printf("%p %p\n", plugin, &plugin);
		

	}

