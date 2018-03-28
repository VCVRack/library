# VCV community repository

The VCV community members are responsible for curating Rack plugins into a centralized database.
[Anyone is welcome to join.](https://github.com/VCVRack/community/issues/248)

All Rack plugins are welcome assuming they
- are not malware (i.e. harm your computer or your privacy)
- do not misuse intellectual property (legally or morally).


## For plugin developers: Adding/updating your plugin

To add your plugin(s) to the Plugin Manager, update its information, or inform us of new builds, please create an [issue](https://github.com/VCVRack/community/issues) with the title equal to your plugin slug (or slugs, comma-separated).
Give us each plugin name, your desired author name, license information, relevant URLs, and anything else under the *Manifest* section below.
A Library Team member will handle your request.


## [Library team](https://github.com/VCVRack/community/issues/352)

Tasks:
- Keep plugin manifests correct and up-to-date.
- Handle issues opened by plugin developers who want to add/update their plugin.
- Seek new plugins/updates when developers don't notify us.

#### Manifest

The filename of each manifest should be `YourSlug.json`.
See [Fundamental.json](manifests/Fundamental.json) for an example.

All properties are optional. URLs should not be redundant across different keys, meaning you must choose the most relevant key for a particular URL.

- **name**: Human-readable display name for your plugin. You can change this on a whim, unlike slugs.
- **author**: Your name, company, alias, or GitHub username.
- **license**: The license type of your plugin. Use "proprietary" if all rights are reserved. If your license is in the [SPDX license list](https://spdx.org/licenses/), use its abbreviation in the "Identifier" column.
- **contactEmail**: Your email address for support inquiries.
- **pluginUrl**: Homepage featuring the plugin itself.
- **authorUrl**: Homepage of the author.
- **manualUrl**: The manual of your plugin. HTML, PDF, or GitHub readme/wiki are fine.
- **sourceUrl**: The source code homepage. E.g. GitHub repo.
- **donateUrl**: Link to donation page for users who wish to donate. E.g. PayPal URL.
- **latestVersion**: Your plugin's latest version, using the guidelines at https://github.com/VCVRack/Rack/issues/266. Do not include the "v" prefix.
- **productId**: ID for plugins sold through the VCV Store.
- **status**: *TODO*


## [Review team](https://github.com/VCVRack/community/issues/354)

*TODO*


## [Repair team](https://github.com/VCVRack/community/issues/269)

Tasks:
- Fix broken plugins, in particular caused by Rack API updates, by opening issues, sending PRs, or even forking and adopting plugins.


## [Build team](https://github.com/VCVRack/community/issues/353)

*TODO*

#### Building repos

Clone all repos with `git submodule update --init --recursive`

Then build all repos with `RACK_DIR=<path to Rack directory> make dist_all`

#### Adding a repo

The folder name should match the slug, even if it is not the repository name.
Be sure to check out the correct branch.

```
cd repos
git submodule add -b v0.6 https://github.com/VCVRack/Fundamental.git Fundamental
```
