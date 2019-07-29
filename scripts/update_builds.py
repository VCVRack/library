import sys
import os
import glob
import json
import time
import build


build.system("git pull")
build.system("git submodule update --init --recursive")


plugin_dirs = sys.argv[1:]

if not plugin_dirs:
	plugin_dirs = glob.glob("repos/*")

built_slugs = []

for plugin_dir in plugin_dirs:
	manifest_filename = f"{plugin_dir}/plugin.json"
	try:
		with open(manifest_filename, "r") as f:
			manifest = json.load(f)
	except IOError:
		# Skip plugins without plugin.json
		continue

	slug = manifest['slug']
	library_manifest_filename = f"manifests/{slug}.json"
	library_manifest = None
	try:
		with open(library_manifest_filename, "r") as f:
			library_manifest = json.load(f)
	except IOError:
		pass

	# Check if the build is up to date
	if library_manifest and manifest['version'] == library_manifest['version']:
		continue

	# Build repo
	print()
	print(f"Building {slug}")
	try:
		build.build(plugin_dir)
	except Exception as e:
		print(e)
		print(f"{slug} build failed")
		input()
		continue

	# Copy manifest
	with open(library_manifest_filename, "w") as f:
		json.dump(manifest, f, indent="  ")

	built_slugs.append(slug)
	# Open plugin issue thread
	os.system(f"qutebrowser \"https://github.com/VCVRack/library/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+in%3Atitle+{slug}\" &")


if not built_slugs:
	raise Exception("Nothing to build")

# Upload packages
build.system("cd ../packages && make upload")

# Commit repository
build.system("git add manifests")
built_slugs_list = ", ".join(built_slugs)
build.system(f"git commit -m 'Update build for {built_slugs_list}'")
build.system("git push")


print()
print("Built " + ", ".join(built_slugs))
