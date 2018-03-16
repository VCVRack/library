# Convert relative Rack directory path to absolute
RACK_DIR := $(shell realpath $(RACK_DIR))

REPOS ?= $(shell ls repos)

dist_all: $(REPOS)

$(REPOS):
	$(MAKE) -C repos/$@ dist

.PHONY: dist_all $(REPOS)
