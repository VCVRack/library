SLUG = "Aepelzens Modules"
VERSION = 0.6.0dev

#FLAGS += -D v_050_dev

LDFLAGS += -lsamplerate

SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
