# VCV Library database

This repo is the plugin database for the [VCV Library](https://library.vcvrack.com/).

All plugins submitted here must follow the [VCV Third-party Module Guidelines](https://vcvrack.com/manual/PluginLicensing.html#vcv-plugin-ethics-guidelines).


## Adding your open-source plugin to the VCV Library

[Open an issue](https://github.com/VCVRack/library/issues/new/choose) for your plugin with the title matching your plugin's slug (not name).
All of your modules should be contained in a single plugin, unless there are technical or legal reasons to publish them in separate plugins.

Include the following information in the issue body:
- Git URL (for example, https://github.com/VCVRack/Fundamental)
- Commit hash you wish to publish (for example, e0ade9a23a9b41124ea3f4ec9aa2cb0b98c2529e)

A Library maintainer will handle your request and close the issue after publishing your plugin or requesting additional information.

**To publish an update of your plugin**, increment the `"version"` in your `plugin.json` file.
In your *existing* plugin issue, post a comment with the commit hash you wish to publish.
A Library maintainer will handle your request.


## Adding your closed-source plugin to the VCV Library

To publish your **closed-source freeware** or **premium/commercial** plugin on the VCV Library, contact [VCV Support](https://vcvrack.com/support) for submission instructions.
You do *not* need to open an issue in this repository.
