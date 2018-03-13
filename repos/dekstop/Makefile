# this flag selects the rack version to compile against.
# 
# possible values are v040 v_050_dev

# FLAGS += -D v040
FLAGS += -D v_050_dev

SOURCES = $(wildcard src/*.cpp portaudio/*.c)

include ../../plugin.mk

FLAGS += -Iportaudio

dist: all
	mkdir -p dist/dekstop
	cp LICENSE* dist/dekstop/
	cp plugin.* dist/dekstop/
	cp -R res dist/dekstop/
