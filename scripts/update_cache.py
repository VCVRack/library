import sys
import os
import glob
import json
import time

import common


PACKAGES_DIR = "../packages"


# Get the timestamp of the earliest commit touching the manifest file
def get_plugin_build(plugin):
	slug = plugin['slug']
	version = plugin['version']
	arch = 'lin'
	package_filename = f"{slug}-{version}-{arch}.zip"
	package_path = PACKAGES_DIR + "/" + package_filename
	return os.path.getmtime(package_path)


# Get the timestamp of the earliest commit touching the manifest file
def get_plugin_creation(manifest_filename):
	stdout = common.run(f"git log --format=%ct -- {manifest_filename} | tail -1")
	return float(stdout.strip())


# Get the timestamp of the earliest commit having the module slug
def get_module_creation(manifest_filename, module_slug):
	stdout = common.run(f"git log --format='%H %ct' -- {manifest_filename}")
	# Use current time as a fallback because if there's no commit with the module, it was added just now.
	earliestTime = time.time()
	for line in stdout.strip().split("\n"):
		hash, timestamp = line.split(" ")
		try:
			stdout = common.run(f"git show {hash}:{manifest_filename}")
			manifest = json.loads(stdout)
			# If the module exists in this commit, keep iterating
			if not common.find(manifest.get('modules', []), lambda module: module['slug'] == module_slug):
				break
		except:
			# If git fails, then the commit didn't actually touch the manifest file, so we can skip it.
			continue
		earliestTime = float(timestamp)
	return earliestTime


def update():
	# Load existing dataset
	cache_filename = "manifests-cache.json"
	try:
		with open(cache_filename) as f:
			cache = json.load(f)
	except:
		cache = {}

	# Iterate plugins
	for manifest_filename in glob.glob('manifests/*.json'):
		with open(manifest_filename) as f:
			plugin = json.load(f)

		plugin_slug = plugin['slug']
		cache_plugin = cache.get(plugin_slug, {})

		# Get plugin build
		print(f"Getting buildTimestamp for plugin {plugin_slug}")
		try:
			cache_plugin['buildTimestamp'] = get_plugin_build(plugin)
		except:
			pass

		# Get plugin creation
		if 'creationTimestamp' not in cache_plugin:
			print(f"Getting creationTimestamp for plugin {plugin_slug}")
			cache_plugin['creationTimestamp'] = get_plugin_creation(manifest_filename)

		# Iterate modules in plugin
		cache_modules = cache_plugin.get('modules', {})
		for module in plugin.get('modules', []):
			module_slug = module['slug']
			cache_module = cache_modules.get(module_slug, {})

			# Get module creation
			if 'creationTimestamp' not in cache_module:
				print(f"Getting creationTimestamp for plugin {plugin_slug} module {module_slug}")
				cache_module['creationTimestamp'] = get_module_creation(manifest_filename, module_slug)

			cache_modules[module_slug] = cache_module
		cache_plugin['modules'] = cache_modules

		cache[plugin_slug] = cache_plugin


	with open(cache_filename, 'w') as f:
		json.dump(cache, f, indent=2)


if __name__ == "__main__":
	update()
