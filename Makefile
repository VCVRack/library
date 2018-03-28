# Convert relative Rack directory path to absolute
RACK_DIR := $(realpath $(RACK_DIR))


dist_all:
	for f in repos/*; do $(MAKE) -C "$$f" dist; done

# Only useful if you have the private keys to the vcvrack.com server
dist_upload:
	rsync repos/*/dist/*.zip vortico@vcvrack.com:downloads/ -uvz
