
# Arable Instruments - Joni

Fork of Audible Instruments 'Texture Synthesizer', based on Mutable Instrument's Clouds.

Features:
- Implements the alternative modes and quality options of Clouds as a menu options 
- Inputs and knobs for the 3 addition 'Blend' parameters (Stereo Spread, Feedback, and Reverb)
- Dedicated Freeze button


# Parable Instruments - Neil

This repository also builds an alternative version, based on the 'Parasites' alternative firmware for Clouds (https://mqtthiqs.github.io/parasites/clouds.html).

Parasites has many features and tweaks compared to the vanilla firmware, including:
- Options for sharper and asymmetrical grain envelopes
- New alternative modes (Oliverb reverb and Resonestor resonator)
- Reverse option for some modes (implemented here as a dedicated button)

IMPORTANT NOTE:
This repository includes code for both the vanilla firmware (Joni) and Parasites firmware (Neil), but only builds one at a time as separate libraries. If you want to maintain both side by side in a development branch, you can have two clones of the repository under different names in the plugins folder.
The parasites firmware is build using the makefile 'Makefile.parasite'.
To build the parasites firmware, run 'make clean', and then build using the makefile Makefile.parasite:
  make -f Makefile.parasite
  make -f Makefile.parasite dist
  
# Building Instructions:
After cloning the repo, do a submodule update with
  git submodule update --init --recursive

To build the plugin:
  make
  
To package:
  make dist

To build the parasites firmware, run 'make clean', and then build using the makefile Makefile.parasite:
  make -f Makefile.parasite
  make -f Makefile.parasite dist

Possible future plans:
- Support for changing buffer size via menu (this has been tested, but lead to crashes in some alternative modes, to enable for testing purposes you can compile with BUFFERRESIZING defined)

Many thanks to Olivier Gillet for open sourcing the Clouds firmware, Andrew Belt for the orignal VCV Rack ports, and Matthias Puech for the Parasites firmware.



