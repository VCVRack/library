
SOURCES = $(wildcard src/*.cpp)

include ../../plugin.mk


dist: all
	mkdir -p dist/mscHack	
	cp LICENSE* dist/mscHack/
	cp plugin.* dist/mscHack/
	cp -R res dist/mscHack/
