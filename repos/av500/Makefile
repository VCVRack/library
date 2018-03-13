
FLAGS += \
	-fshort-enums -DTEST \
	-I./eurorack \
	-Wno-unused-local-typedefs


SOURCES = $(wildcard src/*.cpp) \

include ../../plugin.mk


dist: all
ifndef VERSION
	$(error VERSION is not set.)
endif
	mkdir -p dist/av500
	cp LICENSE* dist/av500/
	cp plugin.* dist/av500/
	cp -R res dist/av500/
	cd dist && zip -5 -r av500-$(VERSION)-$(ARCH).zip av500
