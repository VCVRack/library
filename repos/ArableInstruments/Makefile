VERSION ?= v0.5.0

FLAGS += \
	-DTEST \
	-I./eurorack \
	-Wno-unused-local-typedefs


SOURCES = $(wildcard src/*.cpp) \
	eurorack/stmlib/utils/random.cc \
	eurorack/stmlib/dsp/atan.cc \
	eurorack/stmlib/dsp/units.cc \
	eurorack/clouds/dsp/correlator.cc \
	eurorack/clouds/dsp/granular_processor.cc \
	eurorack/clouds/dsp/mu_law.cc \
	eurorack/clouds/dsp/pvoc/frame_transformation.cc \
	eurorack/clouds/dsp/pvoc/phase_vocoder.cc \
	eurorack/clouds/dsp/pvoc/stft.cc \
	eurorack/clouds/resources.cc 

include ../../plugin.mk


dist: all
	mkdir -p dist/ArableInstruments
	cp LICENSE* dist/ArableInstruments/
	cp plugin.* dist/ArableInstruments/
	cp -R res dist/ArableInstruments/
	rm -f dist/ArableInstruments/res/*.pdn
	cd dist && zip -5 -r ArableInstruments-$(VERSION)-$(ARCH).zip ArableInstruments
