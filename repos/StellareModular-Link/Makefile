SLUG = StellareModular-Link
VERSION = 0.5.2

FLAGS += -DSLUG=$(SLUG) -DVERSION=$(VERSION)
FLAGS += -Imodules/link/include -Imodules/link/modules/asio-standalone/asio/include -Ilink-wrapper

ifeq ($(OS),Windows_NT)
	# On Windows uses the wrapper DLL, needs to be build separately
	LDFLAGS += -Lsrc -llink-wrapper
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CXXFLAGS += -DLINK_PLATFORM_LINUX=1
    endif
    ifeq ($(UNAME_S),Darwin)
        CXXFLAGS += -DLINK_PLATFORM_MACOSX=1
    endif
endif

SOURCES = $(wildcard src/*.cpp)

ifneq ($(OS),Windows_NT)
	# On Mac and Windows directly compiles the wrapper
	SOURCES += $(wildcard link-wrapper/*.cpp)
endif

include ../../plugin.mk


# Convenience target for packaging files into a ZIP file
.PHONY: dist
dist: all
	mkdir -p dist/$(SLUG)
	cp LICENSE* dist/$(SLUG)/
	cp $(TARGET) dist/$(SLUG)/
	cp -R res dist/$(SLUG)/
	cd dist && zip -5 -r $(SLUG)-$(VERSION)-$(ARCH).zip $(SLUG)
