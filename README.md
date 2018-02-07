# VCV Community repository


## Package team

### Building repos

Clone all repos with `git submodule update --init --recursive`

Then build all repos with `RACK_DIR=<path to Rack directory> make dist_all`

### Adding a repo

The folder name should match the slug, even if it is not the repository name.
Be sure to check out the correct branch.

`git submodule add -b v0.6 https://github.com/VCVRack/Fundamental.git Fundamental`
