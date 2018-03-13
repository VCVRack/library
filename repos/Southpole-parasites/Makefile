SLUG = Southpole
VERSION = 0.5.3

# FLAGS will be passed to both the C and C++ compiler
FLAGS += \
	-DTEST \
	-I./eurorack \
	-Wno-unused-local-typedefs

CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
# SOURCES += $(wildcard src/*.cpp)
SOURCES += src/Southpole.cpp
SOURCES += src/DSPUtilities.cpp
SOURCES += src/VAStateVariableFilter.cpp

SOURCES += src/Abr.cpp
SOURCES += src/Aux.cpp
SOURCES += src/Balaclava.cpp	
SOURCES += src/Bandana.cpp
SOURCES += src/Blanks.cpp
SOURCES += src/Falls.cpp
SOURCES += src/Aux_.cpp
SOURCES += src/But.cpp
SOURCES += src/Bytes.cpp
SOURCES += src/DeuxEtageres.cpp
SOURCES += src/Etagere.cpp
SOURCES += src/Falls.cpp
SOURCES += src/Fuse.cpp
SOURCES += src/Gnome.cpp
SOURCES += src/Piste.cpp
SOURCES += src/Riemann.cpp
SOURCES += src/Snake.cpp
SOURCES += src/Sns.cpp
SOURCES += src/Sssh.cpp
SOURCES += src/Wriggle.cpp

SOURCES += src/Splash.cpp
SOURCES += eurorack/stmlib/utils/random.cc
SOURCES += eurorack/stmlib/dsp/atan.cc
SOURCES += eurorack/stmlib/dsp/units.cc
SOURCES += eurorack/tides/generator.cc 
SOURCES += eurorack/tides/resources.cc

SOURCES += src/Cornrows.cpp	
SOURCES += eurorack/braids/macro_oscillator.cc
SOURCES += eurorack/stmlib/utils/random.cc
SOURCES += eurorack/braids/analog_oscillator.cc
SOURCES += eurorack/braids/digital_oscillator.cc
SOURCES += eurorack/braids/resources.cc

SOURCES += src/CornrowsX.cpp	

SOURCES += src/Annuli.cpp
SOURCES += eurorack/rings/dsp/fm_voice.cc
SOURCES += eurorack/rings/dsp/string_synth_part.cc
SOURCES += eurorack/rings/dsp/string.cc
SOURCES += eurorack/rings/dsp/resonator.cc
SOURCES += eurorack/rings/resources.cc
SOURCES += eurorack/rings/dsp/part.cc

SOURCES += src/Smoke.cpp
SOURCES += eurorack/clouds/dsp/correlator.cc
SOURCES += eurorack/clouds/dsp/granular_processor.cc
SOURCES += eurorack/clouds/dsp/mu_law.cc
SOURCES += eurorack/clouds/dsp/pvoc/frame_transformation.cc
SOURCES += eurorack/clouds/dsp/pvoc/phase_vocoder.cc
SOURCES += eurorack/clouds/dsp/pvoc/stft.cc
SOURCES += eurorack/clouds/resources.cc 

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV plugin Makefile framework
include ../../plugin.mk

