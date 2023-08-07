import sys
import os
import glob
import json
import time
import re
import tempfile

import common
import update_modulargrid
import update_cache


TOOLCHAIN_DIR = "../toolchain-v2"
PACKAGES_DIR = "../packages"
MANIFESTS_DIR = "manifests"
RACK_SYSTEM_DIR = "../Rack2"
RACK_USER_DIR = "$HOME/.Rack2"
SCREENSHOTS_DIR = os.path.join(RACK_USER_DIR, "screenshots")
PLUGIN_DIR = os.path.join(RACK_USER_DIR, "plugins-lin-x64")

# Update git before continuing
common.system("git pull")
common.system("git submodule sync --quiet")
common.system("git submodule update --init --recursive")

plugin_paths = sys.argv[1:]

# Default to all repos, so all out-of-date repos are built
if not plugin_paths:
	plugin_paths = glob.glob("repos/*")

manifest_versions = {}

for plugin_path in plugin_paths:
	plugin_path = os.path.abspath(plugin_path)
	(plugin_basename, plugin_ext) = os.path.splitext(os.path.basename(plugin_path))
	# Extract manifest from plugin dir or package
	if os.path.isdir(plugin_path):
		manifest_filename = os.path.join(plugin_path, "plugin.json")
		try:
			# Read manifest
			with open(manifest_filename, "r") as f:
				manifest = json.load(f)
		except IOError:
			# Skip plugins without plugin.json
			continue
		slug = manifest['slug']
		version = manifest['version']
	# Extract manifest from .vcvplugin
	elif plugin_ext == ".vcvplugin":
		m = re.match(r'^(.*)-(2\..*?)-(.*)$', plugin_basename)
		if not m:
			raise Exception(f"Filename {plugin_path} invalid format")
		slug = m[1]
		version = m[2]
		arch = m[3]
		# Open ZIP
		tempdir = tempfile.mkdtemp()
		common.system(f'zstd -d < "{plugin_path}" | tar -x -C "{tempdir}"')
		# Unzip manifest
		manifest_filename = os.path.join(tempdir, slug, "plugin.json")
		with open(manifest_filename) as f:
			manifest = json.load(f)
		if manifest['slug'] != slug:
			raise Exception(f"Manifest slug does not match filename slug {slug}")
		if manifest['version'] != version:
			raise Exception(f"Manifest version does not match filename version {version}")
		common.system(f'rm -rf "{tempdir}"')
	else:
		raise Exception(f"Plugin {plugin_path} is not a valid format")

	# Get library manifest
	library_manifest_filename = os.path.join(MANIFESTS_DIR, f"{slug}.json")
	# Warn if manifest is new
	# if not os.path.exists(library_manifest_filename):
	# 	print(f"Manifest {slug} is new, press enter to approve.")
	# 	input()

	if os.path.isdir(plugin_path):
		# Check if the library manifest is up to date
		try:
			with open(library_manifest_filename, "r") as f:
				library_manifest = json.load(f)
			if library_manifest and version == library_manifest['version']:
				continue
		except IOError:
			pass

		# Build repo
		print()
		print(f"Building {slug}")
		try:
			common.system(f'cd "{TOOLCHAIN_DIR}" && make plugin-build-clean')
			common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-mac-arm64 PLUGIN_DIR="{plugin_path}"')
			common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-mac-x64 PLUGIN_DIR="{plugin_path}"')
			common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-win-x64 PLUGIN_DIR="{plugin_path}"')
			common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-lin-x64 PLUGIN_DIR="{plugin_path}"')
			common.system(f'cp -v "{TOOLCHAIN_DIR}"/plugin-build/* "{PACKAGES_DIR}"/')
			# Install Linux package for testing
			common.system(f'cp -v "{TOOLCHAIN_DIR}"/plugin-build/*-lin-x64.vcvplugin "{PLUGIN_DIR}"/')
		except Exception as e:
			print(e)
			print(f"{slug} build failed")
			input()
			continue
		finally:
			common.system(f'cd "{TOOLCHAIN_DIR}" && make plugin-build-clean')

		# Open plugin issue thread
		os.system(f"xdg-open 'https://github.com/VCVRack/library/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+in%3Atitle+{slug}' &")

	elif plugin_ext == ".vcvplugin":
		# Review manifest for errors
		print(json.dumps(manifest, indent="  "))
		print("Press enter to approve manifest")
		input()

		# Copy package
		package_filename = os.path.basename(plugin_path)
		common.system(f'cp "{plugin_path}" "{PACKAGES_DIR}/{package_filename}"')
		# Update file timestamp
		common.system(f'touch "{PACKAGES_DIR}/{package_filename}"')
		# Install Linux package for testing
		if arch == 'lin' or arch == 'lin-x64':
			common.system(f'cp "{plugin_path}" "{PLUGIN_DIR}/{package_filename}"')

	# Copy manifest
	with open(library_manifest_filename, "w") as f:
		json.dump(manifest, f, indent="  ")

	# Delete screenshot cache
	screenshots_dir = os.path.join(SCREENSHOTS_DIR, slug)
	common.system(f'rm -rf "{screenshots_dir}"')

	manifest_versions[slug] = version


if not manifest_versions:
	print("Nothing to build")
	exit(0)

update_cache.update()
update_modulargrid.update()

# Upload data

manifest_versions_str = ", ".join(map(lambda pair: pair[0] + " to " + pair[1], manifest_versions.items()))

print()
print(f"Press enter to launch Rack and test the following packages: {manifest_versions_str}")
input()
try:
	common.system(f"cd {RACK_SYSTEM_DIR} && ./Rack")
	common.system(f"cd {RACK_USER_DIR} && grep 'warn' log.txt || true")
except:
	print(f"Rack failed! Enter to continue if desired")

print(f"Press enter to generate screenshots, upload packages, upload screenshots, and commit/push the library repo.")
input()

# Generate screenshots
try:
	common.system(f"cd {RACK_SYSTEM_DIR} && ./Rack -t 4")
except:
	print(f"Rack failed! Enter to continue if desired")
common.system("cd ../screenshots && make -j$(nproc)")

# Upload packages
common.system("cd ../packages && make upload")

# Upload screenshots
common.system("cd ../screenshots && make upload")

# Commit repository
common.system("git add manifests")
common.system("git add manifests-cache.json ModularGrid-VCVLibrary.json")
common.system(f"git commit -m 'Update manifest {manifest_versions_str}'")
common.system("git push")

print()
print(f"Updated {manifest_versions_str}")
