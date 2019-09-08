import sys
import json
import os
import glob

filenames = sys.argv[1:]

for filename in filenames:
	# Read plugin manifest
	try:
		with open(filename, "r") as f:
			manifest = json.load(f, strict=False)
	except Exception as e:
		print(f"Could not parse {filename}")
		print(e)
		continue

	slug = os.path.splitext(os.path.basename(filename))[0]
	# dest_filename = f"manifests/{slug}.json"
	# if os.path.isfile(dest_filename):
	# 	continue

	if manifest.get('manualUrl', '') and manifest.get('pluginUrl', '') and manifest.get('author', '') and manifest.get('authorEmail', '') and manifest.get('authorUrl', '') and manifest.get('sourceUrl', '') and manifest.get('donateUrl', ''):
		print(slug)

	# # Write library manifest
	# slug = manifest["slug"]
	# manifest_filename = f"manifests/{slug}.json"
	# with open(manifest_filename, "w") as f:
	# 	json.dump(manifest, f, indent="  ")
	# print(f"Copied {slug}")
