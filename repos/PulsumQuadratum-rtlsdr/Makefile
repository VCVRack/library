SLUG = PulsumQuadratum-SDR
VERSION = 0.6.0

PKGCONFIG= pkg-config
PACKAGES= libusb-1.0 librtlsdr

# FLAGS will be passed to both the C and C++ compiler
FLAGS += $(shell $(PKGCONFIG) --cflags $(PACKAGES))
CFLAGS +=
CXXFLAGS +=

# Add .cpp and .c files to the build
SOURCES = $(wildcard src/*.cpp src/*.c src/*/*.cpp src/*/*.c)

# Must include the VCV plugin Makefile framework
include ../../plugin.mk

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
ifeq ($(ARCH), lin)
	# WARNING: static compilation is broken on Linux
	LDFLAGS +=$(shell $(PKGCONFIG) --libs $(PACKAGES))
endif

ifeq ($(ARCH), mac)
	LDFLAGS +=$(shell $(PKGCONFIG) --variable=libdir libusb-1.0)/libusb-1.0.a
	LDFLAGS +=$(shell $(PKGCONFIG) --variable=libdir librtlsdr)/librtlsdr.a
endif

ifeq ($(ARCH), win)
	LDFLAGS +=$(shell $(PKGCONFIG) --variable=libdir librtlsdr)/librtlsdr_static.a
	LDFLAGS +=$(shell $(PKGCONFIG) --variable=libdir libusb-1.0)/libusb-1.0.a
endif

DISTRIBUTABLES += $(wildcard LICENSE*) res

# If RACK_DIR is not defined when calling the Makefile, default to two levels above
RACK_DIR ?= ../..

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
