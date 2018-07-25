import sys
import os
import glob
import json
import time
import build_plugin


plugin_dirs = sys.argv[1:]

if not plugin_dirs:
	plugin_dirs = glob.glob("repos/*")

built_slugs = []

for plugin_dir in plugin_dirs:
	slug = os.path.basename(plugin_dir)
	manifest_filename = f"manifests/{slug}.json"
	with open(manifest_filename, "r") as f:
		manifest = json.load(f)

	# We need a repoVersion to build
	if 'repoVersion' not in manifest:
		continue
	# Skip if update is not needed
	if 'latestVersion' in manifest and manifest['latestVersion'] == manifest['repoVersion']:
		continue

	# Build repo
	success = build_plugin.build(plugin_dir)
	if not success:
		print(f"{slug} failed")
		input()
		continue

	build_plugin.system('mv -vi stage/* ../downloads/')
	build_plugin.delete_stage()

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
