LOCAL = $(shell pwd)

# Arch-specifics
include ../../../../arch.mk

ifeq ($(ARCH),mac)
	MACOS_FLAGS = -mmacosx-version-min=10.7 -stdlib=libc++
	export CFLAGS = $(MACOS_FLAGS)
	export CXXFLAGS = $(MACOS_FLAGS)
	export CPPFLAGS = $(MACOS_FLAGS)
	export LDFLAGS = $(MACOS_FLAGS)
endif

# Commands
WGET = wget -nc
UNTAR = tar xf
UNZIP = unzip
ifeq ($(ARCH),win)
	CMAKE = cmake -G 'MSYS Makefiles'
else
	CMAKE = cmake
endif

# Libraries
ifeq ($(ARCH),lin)
	mpg123 = lib/libmpg123.so
endif

ifeq ($(ARCH),mac)
	mpg123 = lib/libmpg123.dylib
endif

ifeq ($(ARCH),win)
	mpg123 = bin/libmpg123-0.dll
endif

.NOTPARALLEL:

all: $(mpg123)
$(mpg123):
	$(WGET) https://www.mpg123.de/download/mpg123-1.25.8.tar.bz2
	$(UNTAR) mpg123-1.25.8.tar.bz2
	cd mpg123-1.25.8 && ./configure --prefix="$(LOCAL)" --with-cpu=generic --with-pic --disable-shared --enable-static
	$(MAKE) -C mpg123-1.25.8
	$(MAKE) -C mpg123-1.25.8 install

clean:
	git clean -ffdxi
