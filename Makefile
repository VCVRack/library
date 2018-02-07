# Convert relative Rack directory path to absolute
RACK_DIR := $(shell realpath $(RACK_DIR))


dist_all:
	for f in repos/*; do $(MAKE) -C "$$f" dist; done
