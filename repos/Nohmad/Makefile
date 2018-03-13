
SOURCES = $(wildcard src/*.cpp)


include ../../plugin.mk


dist: all
	mkdir -p dist/Nohmad
	cp LICENSE* dist/Nohmad/
	cp $(TARGET) dist/Nohmad/
	cp -R res dist/Nohmad/
