import sys
import os
import glob
import json
import time
import re
import tempfile
import random

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
FILES_DIR = "../files/packages"

BUILD_MAC_ARM64_ONLY = False


def package_exists(slug, version, os_, cpu):
	if os.path.isfile(os.path.join(PACKAGES_DIR, f"{slug}-{version}-{os_}-{cpu}.vcvplugin")):
		return True
	if cpu == 'x64':
		if os.path.isfile(os.path.join(PACKAGES_DIR, f"{slug}-{version}-{os_}.vcvplugin")):
			return True
	return False


# Get missing Mac ARM64 slugs
def print_missing_mac_arm64():
	emails = set()

	for manifest_path in glob.glob("manifests/*.json"):
		with open(manifest_path, "r") as f:
			manifest = json.load(f)
		slug = manifest['slug']
		version = manifest['version']
		author = manifest['author']
		license = manifest.get('license', "")
		email = manifest.get('authorEmail', "")

		if not package_exists(slug, version, 'mac', 'x64'):
			continue
		if package_exists(slug, version, 'mac', 'arm64'):
			continue

		print(f"{slug} {version}\t\t{email}\t\t{license}")

# print_missing_mac_arm64()
# exit(0)


# Update git before continuing
if not BUILD_MAC_ARM64_ONLY:
	common.system("git pull")
	common.system("git submodule sync --recursive --quiet")
	common.system("git submodule update --init --recursive")

plugin_paths = sys.argv[1:]

# Default to all repos, so all out-of-date repos are built
if not plugin_paths:
	plugin_paths = glob.glob("repos/*")

# Randomize repo order
plugin_paths = random.sample(plugin_paths, len(plugin_paths))

manifest_versions = {}

for plugin_path in plugin_paths:
	plugin_path = os.path.abspath(plugin_path)
	(plugin_basename, plugin_ext) = os.path.splitext(os.path.basename(plugin_path))

	# Get manifest
	# Source dir
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

	# Plugin package
	elif plugin_ext == ".vcvplugin":
		m = re.match(r'^(.*)-(\d\..*?)-(.*?)-(.*?)$', plugin_basename)
		if not m:
			raise Exception(f"Filename {plugin_path} invalid format")
		slug = m[1]
		version = m[2]
		os_ = m[3]
		cpu = m[4]
		# Extract ZIP to temp dir
		try:
			tempdir = tempfile.mkdtemp()
			common.system(f'zstd -d < "{plugin_path}" | tar -x -C "{tempdir}"')
			# Read manifest
			manifest_filename = os.path.join(tempdir, slug, "plugin.json")
			with open(manifest_filename) as f:
				manifest = json.load(f)
			if manifest['slug'] != slug:
				raise Exception(f"Manifest slug {manifest['slug']} does not match filename slug {slug}")
			if manifest['version'] != version:
				raise Exception(f"Manifest version {manifest['version']} does not match filename version {version}")
		finally:
			common.system(f'rm -rf "{tempdir}"')

	else:
		raise Exception(f"Plugin {plugin_path} is not a valid format")

	# Slug blacklist
	# if slug == 'questionablemodules':
	# 	continue

	# Get library manifest
	library_manifest_filename = os.path.join(MANIFESTS_DIR, f"{slug}.json")
	library_manifest = None
	try:
		with open(library_manifest_filename, "r") as f:
			library_manifest = json.load(f)
	except IOError:
		# Warn if manifest is new
		print(f"Manifest {slug} is new, press enter to approve.")
		if input() != '':
			continue
		pass

	# Source dir
	if os.path.isdir(plugin_path):
		# Check library manifest version
		if library_manifest and version == library_manifest['version']:
			# Build only if library manifest is out of date
			if not BUILD_MAC_ARM64_ONLY:
				continue
		else:
			# Build Mac ARM64 only if library manifest is up to date
			if BUILD_MAC_ARM64_ONLY:
				continue

		# Build repo
		if BUILD_MAC_ARM64_ONLY:
			if package_exists(slug, version, 'mac', 'arm64'):
				print(f"{slug} {version} Mac ARM64 package already exists")
				continue
			if not package_exists(slug, version, 'mac', 'x64'):
				print(f"{slug} {version} Mac x64 doesn't exist, not building")
				continue

		print()
		print(f"Building {slug}")
		try:
			# Clean repo
			common.system(f'cd "{TOOLCHAIN_DIR}" && make plugin-build-clean')
			# Build repo for each arch
			common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-mac-arm64 PLUGIN_DIR="{plugin_path}"')
			if not BUILD_MAC_ARM64_ONLY:
				common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-mac-x64 PLUGIN_DIR="{plugin_path}"')
				common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-win-x64 PLUGIN_DIR="{plugin_path}"')
				common.system(f'cd "{TOOLCHAIN_DIR}" && make -j2 plugin-build-lin-x64 PLUGIN_DIR="{plugin_path}"')
			# Copy package to packages dir
			common.system(f'cp -v "{TOOLCHAIN_DIR}"/plugin-build/* "{PACKAGES_DIR}"/')
			# Copy Mac ARM64 package to files dir for testing
			common.system(f'cp -v "{TOOLCHAIN_DIR}"/plugin-build/*-mac-arm64.vcvplugin "{FILES_DIR}"/')
			# Install Linux package for testing
			if not BUILD_MAC_ARM64_ONLY:
				common.system(f'cp -v "{TOOLCHAIN_DIR}"/plugin-build/*-lin-x64.vcvplugin "{PLUGIN_DIR}"/')
		except Exception as e:
			print(e)
			print(f"{slug} build failed")
			input()
			continue
		finally:
			common.system(f'cd "{TOOLCHAIN_DIR}" && make plugin-build-clean')

		# Open plugin issue thread
		if not BUILD_MAC_ARM64_ONLY:
			os.system(f"xdg-open 'https://github.com/VCVRack/library/issues?q=is%3Aissue+in%3Atitle+{slug}' &")

	# Plugin package
	elif plugin_ext == ".vcvplugin":
		# Review manifest for errors
		print(json.dumps(manifest, indent="  "))
		print("Press enter to approve manifest")
		input()

		# Copy package
		common.system(f'cp "{plugin_path}" "{PACKAGES_DIR}/"')
		# Update file timestamp
		package_filename = os.path.basename(plugin_path)
		common.system(f'touch "{PACKAGES_DIR}/{package_filename}"')
		# Copy Mac ARM64 package to files dir for testing
		if os_ == 'mac' and cpu == 'arm64':
			common.system(f'cp -v "{plugin_path}" "{FILES_DIR}"/')
		# Install Linux package for testing
		if os_ == 'lin':
			common.system(f'cp "{plugin_path}" "{PLUGIN_DIR}/"')

	# Copy manifest
	if not BUILD_MAC_ARM64_ONLY:
		with open(library_manifest_filename, "w") as f:
			json.dump(manifest, f, indent="  ")

	if not BUILD_MAC_ARM64_ONLY:
		# Delete screenshot cache
		screenshots_dir = os.path.join(SCREENSHOTS_DIR, slug)
		common.system(f'rm -rf "{screenshots_dir}"')

	manifest_versions[slug] = version

manifest_versions_str = ", ".join(map(lambda pair: pair[0] + " to " + pair[1], manifest_versions.items()))


if BUILD_MAC_ARM64_ONLY:
	print()
	print(f"Test plugins on Mac ARM64 and press enter if approved: {manifest_versions_str}")
	input()
	common.system("cd ../packages && make upload")
	exit(0)

if not manifest_versions:
	print("Nothing to build")
	exit(0)

update_cache.update()
update_modulargrid.update()

# Upload data

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

# Commit git repo
common.system("git add manifests")
common.system("git add manifests-cache.json ModularGrid-VCVLibrary.json")
common.system(f"git commit -m 'Update manifest {manifest_versions_str}'")

# Upload packages
common.system("cd ../packages && make upload")

# Upload screenshots
common.system("cd ../screenshots && make upload")

# Push git repo
common.system("git push")

print()
print(f"Updated {manifest_versions_str}")
