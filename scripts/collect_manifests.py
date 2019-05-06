import sys
import json
import os
import glob

for filename in glob.glob("repos/*/plugin.json"):
	# Read plugin manifest
	try:
		with open(filename, "r") as f:
			manifest = json.load(f, strict=False)
	except Exception as e:
		print(e)
		print(f"Could not parse {filename}")
		continue

	# Write library manifest
	slug = manifest["slug"]
	manifest_filename = f"manifests/{slug}.json"
	with open(manifest_filename, "w") as f:
		json.dump(manifest, f, indent="  ")
