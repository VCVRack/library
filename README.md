
# VCV community

[![Build Status](https://travis-ci.org/VCVRack/community.svg?branch=master)](https://travis-ci.org/VCVRack/community)

*Manifest repository for VCV Rack plugins*

### Contributing your plugin

Create or edit your plugin's manifest file in `plugins/` using the template and documentation below.
You can do this without cloning by clicking GitHub's "Create new file" button or by clicking the pencil icon to edit a file.

### Manifest template

Filename should be `YourSlug.json`.

```json
{
  "slug": "AudibleInstruments",
  "name": "Audible Instruments",
  "author": "VCV",
  "license": "BSD 3-clause",
  "version": "0.5.0",
  "homepage": "https://vcvrack.com/",
  "donation": "https://www.paypal.me/",
  "manual": "https://vcvrack.com/manual/AudibleInstruments.html",
  "source": "https://github.com/VCVRack/AudibleInstruments",
  "downloads": {
    "win": {
      "download": "https://example.com/AudibleInstruments-0.5.0-win.zip",
      "sha256": "9372ce3f8ef42d7e874beda36f7c051b3d7de9c904e259a5fc9dba8dc664bf65"
    },
    "lin": {
      "download": "https://example.com/AudibleInstruments-0.5.0-lin.zip",
      "sha256": "238145156cc4e11b3ca6d750df38ca2daf3e09648d9c7db5f23e9518c1ccf5dc"
    },
    "mac": {
      "download": "https://example.com/AudibleInstruments-0.5.0-mac.zip",
      "sha256": "c19fcdfd07dc6184ce30953bf9adb2b4a77d20ef66d2b1c6a6024c2ca4ff505b"
    }
  }
}
```

### Manifest properties

Keys with * are required.

- **slug**\*: Unique identifier for your plugin. Should never change. Avoid spaces. Prefix with your company or developer name if you plan to release multiple plugins, e.g. "VCV-PulseMatrix". But most importantly, should never change.
- **name**: Human-readable display name for your plugin. Defaults to the slug if omitted. May change on a whim.
- **author**: Your name, company, alias, or GitHub username, whichever you prefer
- **version**\*: Your plugin's version, using the guidelines at https://github.com/VCVRack/Rack/issues/266. Do not include the "v" prefix.
- **license**: Abbreviation of the license(s) of your plugin. Use "proprietary" if all rights are reserved.
- **homepage**: URL of the landing page of your plugin. Omit if redundant with the source URL.
- **donation**: URL of your donation page.  Use this to provide a URL to users who wish to donate.
- **manual**: URL of the manual of your plugin. HTML, PDF, or GitHub readme/wiki are fine.
- **source**: URL of the source code landing page.
- **downloads**: Mapping of supported architectures to downloads.
  - **download**: Direct URL to the compiled binary ZIP. If supported architectures are merged into one ZIP file, you can copy the same link into each of the architecture sections. The name of the zip file does not matter.  However, each zip must contain a single folder equal to the name of the plugin's slug. GitHub releases is a good choice for open-source plugins.
  - **sha256**: Hex SHA256 string of the binary ZIP file for verifying downloads. Use `sha256sum [FILE]` or `sha256 [FILE]` or https://www.virustotal.com
