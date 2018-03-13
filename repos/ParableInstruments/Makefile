
FLAGS += \
	-fshort-enums -DTEST \
	-I./parasites \
	-Wno-unused-local-typedefs


SOURCES = $(wildcard src/*.cpp) \
	parasites/stmlib/utils/random.cc \
	parasites/stmlib/dsp/atan.cc \
	parasites/stmlib/dsp/units.cc \
	parasites/clouds/dsp/correlator.cc \
	parasites/clouds/dsp/granular_processor.cc \
	parasites/clouds/dsp/mu_law.cc \
	parasites/clouds/dsp/pvoc/frame_transformation.cc \
	parasites/clouds/dsp/pvoc/phase_vocoder.cc \
	parasites/clouds/dsp/pvoc/stft.cc \
	parasites/clouds/resources.cc 


include ../../plugin.mk


dist: all
	mkdir -p dist/ParableInstruments
	cp LICENSE* dist/ParableInstruments/
	cp plugin.* dist/ParableInstruments/
	cp -R res dist/ParableInstruments/
	cd dist && zip -5 -r ParableInstruments-$(VERSION)-$(ARCH).zip ParableInstruments
