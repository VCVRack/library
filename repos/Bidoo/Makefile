SLUG = Bidoo
VERSION = 0.6.0dev

FLAGS += -I./src/dep/include -I./src/dep/audiofile -I./src/dep/filters

include ../../arch.mk


ifeq ($(ARCH), lin)
	LDFLAGS += -L../../dep/lib -lglfw ../../dep/lib/libcurl.a src/dep/lib/libmpg123.a
endif

ifeq ($(ARCH), mac)
	LDFLAGS += -L../../dep/lib -lglfw ../../dep/lib/libcurl.a src/dep/lib/libmpg123.a
endif

ifeq ($(ARCH), win)
	LDFLAGS += -L../../dep/lib -lglfw3dll -lcurl src/dep/lib/libmpg123.a -lshlwapi
endif

SOURCES = $(wildcard src/*.cpp src/dep/audiofile/*cpp src/dep/pffft/*c src/dep/filters/*cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

include ../../plugin.mk

dep:
	$(MAKE) -C src/dep
