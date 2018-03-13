DIST_NAME=VCVRack-Simple

SOURCES = src/Simple.cpp					\
		  src/ClockDivider.cpp				\
		  src/ButtonTrigger.cpp				\
		  src/Recorder.cpp					\
		  src/Clock.cpp						\
		  src/ClockWidget.cpp				\
		  utils/LightControl.cpp			\
		  utils/PulseGate.cpp				\
		  utils/WavWriter.cpp				\
		  utils/StateMachine.cpp			\
		  utils/Path.cpp					\
		  utils/TextDisplay.cpp	            \
		  utils/VuMeter.cpp					\
		  utils/write_wav.c


FLAGS += -I"."

include ../../plugin.mk

dist: all
ifndef VERSION
	$(error VERSION must be defined when making distributables)
endif
	mkdir -p dist/$(DIST_NAME)
	cp LICENSE* dist/$(DIST_NAME)/
	cp $(TARGET) dist/$(DIST_NAME)/
	cp -R res dist/$(DIST_NAME)/
	cd dist && zip -5 -r $(DIST_NAME)-$(VERSION)-$(ARCH).zip $(DIST_NAME)

serve:
	jekyll serve --watch --trace -s ./docs -d ./docs/_site
