import sys
import os
import glob
import json
import time
import build_plugin


built_slugs = []

for manifest_filename in glob.glob("manifests/*.json"):
	slug = os.path.basename(manifest_filename).split('.')[0]
	with open(manifest_filename, "r") as f:
		manifest = json.load(f)

	# We need a repoVersion to build
	if 'repoVersion' not in manifest:
		continue
	# Skip if update is not needed
	if 'latestVersion' in manifest and manifest['latestVersion'] == manifest['repoVersion']:
		continue

	# Build repo
	plugin_dir = f"repos/{slug}"
	success = build_plugin.build(plugin_dir)
	if not success:
		print(f"{slug} failed")
		input()
		continue

	# Update build information
	manifest['latestVersion'] = manifest['repoVersion']
	manifest['buildTimestamp'] = round(time.time())
	manifest['status'] = "available"

	with open(manifest_filename, "w") as f:
		json.dump(manifest, f, indent="  ")

	built_slugs.append(slug)


if built_slugs:
	print()
	print("Built " + ", ".join(built_slugs))
else:
	print("Nothing to build")
