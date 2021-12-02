# VCV Library database

This repo contains the database for VCV Rack plugins, for [VCV Library](https://library.vcvrack.com/).

All plugins submitted here must
- follow the [VCV Plugin Ethics Guidelines](https://vcvrack.com/manual/PluginLicensing.html#vcv-plugin-ethics-guidelines).
- not harm the user's computer or privacy.


## Adding your plugin to the VCV Library (for open-source plugins)

Create exactly one thread in the [Issue Tracker](https://github.com/VCVRack/library/issues) per plugin, with a title equal to your plugin's slug (not name).
Post the URL to your source code, and follow the "Pushing an update" section below.
This will be your permanent communication channel with VCV Library maintainers.

A Library maintainer will handle your request and post a comment when updated.


### Pushing an update

To inform us of an update to your plugin, make sure to increment the `"version"` in your [`plugin.json` manifest file](https://vcvrack.com/manual/Manifest.html) (e.g. from 1.2.3 to 1.2.4), and push a commit to your repository.
Post a comment in your plugin's thread (we will re-open it for you) with
- the new version
- the commit hash (given by `git log` or `git rev-parse HEAD`). Please do not just give the name of a branch like `master`.

A Library maintainer will handle your request and post a comment when updated.
The issue will be closed when the build is updated.


## Adding your plugin to the VCV Library (for closed-source freeware and commercial plugins)

Email contact@vcvrack.com to be added to the VCV Library or sold through the VCV Store.
You do not need to create a plugin thread.
