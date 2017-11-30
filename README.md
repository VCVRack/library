
# VCV community

*Manifest repository for VCV Rack plugins*


### Manifest template

Filename should be `YourSlug.json`.

```json
{
	"slug": "AudibleInstruments",
	"name": "Audible Instruments",
	"license": "BSD 3-clause",
	"version": "0.5.0",
	"homepage": "https://vcvrack.com/",
	"manual": "https://vcvrack.com/manual/AudibleInstruments.html",
	"source": "https://github.com/VCVRack/AudibleInstruments",
	"download": "https://example.com/AudibleInstruments-0.5.0.zip",
	"productId": "1234567890",
	"arch": ["win", "lin", "mac"],
	"sha256": "5993dbe15af246092678e00897dc7a4a1e2dabc494b83412fe31200d5bb58305",
}
```

### Manifest properties

Keys with * are required.

- **slug**\*: Unique identifier for your plugin. Should never change. Avoid spaces. Prefix with your company or developer name if you plan to release multiple plugins, e.g. "VCV-PulseMatrix". But most importantly, should never change.
- **name**\*: Human-readable display name for your plugin. May change on a whim.
- **version**\*: Your plugin's version, using the guidelines at https://github.com/VCVRack/Rack/issues/266. Do not include the "v" prefix.
- **license**: Abbreviation of the license(s) of your plugin. Use "proprietary" if all rights are reserved.
- **homepage**: URL of the landing page of your plugin. Omit if redundant with the source URL.
- **manual**: URL of the manual of your plugin. HTML, PDF, or GitHub readme/wiki are fine.
- **source**: URL of the source code landing page.
- **download**: URL of the compiled binary package. All supported architectures must be merged into one ZIP file. Must contain a single folder equal to the name of the plugin's slug. GitHub releases is a good choice for open-source plugins.
- **productId**: VCV store ID for commercial plugins.
- **arch**: List of supported architectures. Defaults to `["win", "mac", "lin"]` if omitted.
- **sha256**: Hex SHA256 string of the binary ZIP file for verifying downloads. Use `sha256sum [FILE]` or `sha256 [FILE]`.
