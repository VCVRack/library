SLUG = MrLumps
VERSION = 0.5.2

SOURCES = $(wildcard src/*.cpp)

# Must include the VCV plugin Makefile framework
include ../../plugin.mk

dist: all
	mkdir -p dist/$(SLUG)
	cp LICENSE* dist/$(SLUG)/
	cp $(TARGET) dist/$(SLUG)/
	cp -R res dist/$(SLUG)/
	cd dist && zip -5 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)
