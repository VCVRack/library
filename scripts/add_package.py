import sys
import json
import os
import glob
import zipfile
import shutil
import build


PACKAGES_DIR = "../packages"
MANIFESTS_DIR = "manifests"
plugin_packages = sys.argv[1:]
slugs = set()


for package_src in plugin_packages:
	# Open ZIP
	z = zipfile.ZipFile(package_src)
	manifest_src = [f for f in z.namelist() if f.endswith("/plugin.json")][0]

	# Read manifest
	with z.open(manifest_src) as f:
		manifest = json.load(f)

	# Review manifest
	print(json.dumps(manifest, indent="  "))
	input()

	# Write manifest
	slug = os.path.split(manifest_src)[0]
	manifest_dest = os.path.join(MANIFESTS_DIR, slug + ".json")
	with open(manifest_dest, "w") as f:
		json.dump(manifest, f, indent="  ")

	# Move package
	package_dest = os.path.join(PACKAGES_DIR, os.path.basename(package_src))
	shutil.move(package_src, package_dest)
	build.system(f"touch '{package_dest}'")

	slugs.add(slug)


# Upload packages
build.system(f"cd {PACKAGES_DIR} && make upload")

# Commit repository
build.system("git add manifests")
slugs_list = ", ".join(slugs)
build.system(f"git commit -m 'Update manifest for {slugs_list}'")
build.system("git push")
