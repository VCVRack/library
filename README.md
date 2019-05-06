# VCV library repository

The VCV library maintainers are responsible for curating Rack plugins into the [VCV Plugin Manager](https://vcvrack.com/plugins.html).
[Anyone is welcome to become a maintainer.](https://github.com/VCVRack/library/issues/248)

All Rack plugins are welcome assuming they
- are not malware (i.e. harm users' computer or privacy)
- do not misuse intellectual property (legally or ethically)


## Adding your plugin to the VCV Plugin Manager (for open-source plugins)

Create exactly one thread in the [Issue Tracker](https://github.com/VCVRack/library/issues), with a title equal to your plugin slug (or multiple slugs, comma-separated, if you have more than one plugin).
This will be your permanent communication channel with VCV library maintainers.

Post a comment in your plugin's thread with the plugin name, license, all relevant URLs, and your email address if you want it to be public.

A library maintainer will handle your request and post a comment when updated.


#### Pushing an update

To inform us of an update to the plugin itself, make sure to increment `"version"` in your `plugin.json` file (e.g. from 1.2.12 to 1.2.13), and push a commit to your repository.
Post a comment in your plugin's thread with
- the new version
- the commit hash (given by `git log` or `git rev-parse HEAD`). Please do not just give the name of a branch like `master`.

A library maintainer will handle your request and post a comment when updated.


## Adding your plugin to the VCV Plugin Manager (for closed-source freeware and commercial plugins)

Email contact@vcvrack.com to be added to the VCV Plugin Manager or sold through the VCV Store.
You do not need to create a plugin thread.
