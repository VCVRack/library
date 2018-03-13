SHELL:=/bin/bash -O extglob

# 03/03/2018 Changed to match https://github.com/VCVRack/Fundamental/blob/master/Makefile

SLUG = trowaSoft
VERSION = 0.6.5.2dev# Not really a full release yet

# Add .cpp and .c files to the build
SOURCES = \
		$(wildcard lib/oscpack/ip/*.cpp) \
		$(wildcard lib/oscpack/osc/*.cpp) \
		$(wildcard src/*.cpp) \
		$(wildcard src/*/*.cpp) \

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
include ../../arch.mk

ifeq ($(ARCH), win)
	SOURCES += $(wildcard lib/oscpack/ip/win32/*.cpp) 
	LDFLAGS += -lws2_32 -lwinmm
	# TODO: Figure out how to get this to compile in Win without this
	LDFLAGS +=  -L../../dep/lib -lglew32 -lglfw3dll
else
	SOURCES += $(wildcard lib/oscpack/ip/posix/*.cpp) 
endif

DISTRIBUTABLES += $(wildcard LICENSE*) res \
 pd other
# ^ add our other folders (supplementary files)

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk




