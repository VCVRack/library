import sys
import os
import glob
import json
import re
import requests

import common


# Get the timestamp of the earliest commit touching the manifest file
def get_plugin_creation(manifest_filename):
	stdout = common.system(f"git log --format=%ct -- {manifest_filename} | tail -1")
	return float(stdout.strip())


# Get the timestamp of the earliest commit having the module slug
def get_module_creation(manifest_filename, module_slug):
	stdout = common.system(f"git log --format='%H %ct' -- {manifest_filename}")
	for line in stdout.strip().split("\n"):
		hash, time = line.split(" ")
		stdout = common.system(f"git show {hash}:{manifest_filename}")
		print(stdout)


def update():
	# Load existing dataset
	cache_filename = "manifest-cache.json"
	with open(cache_filename) as f:
		cache = json.load(f)

	# Iterate plugins
	for manifest_filename in glob.glob('manifests/*.json'):
		with open(manifest_filename) as f:
			plugin = json.load(f)

		plugin_slug = plugin['slug']
		cache_plugin = cache.get(plugin_slug, {})

		# Get plugin creation
		if 'creationTimestamp' not in cache_plugin:
			cache_plugin['creationTimestamp'] = get_plugin_creation(manifest_filename)

		# Iterate modules in plugin
		cache_modules = cache.get('modules', {})
		for module in plugin.get('modules', []):
			module_slug = module['slug']
			cache_module = cache_modules.get(module_slug, {})

			# TODO
			# Get module creation
			# get_module_creation()
			# exit()

			cache_modules[module_slug] = cache_module
		cache_plugin['modules'] = cache_modules

		cache[plugin_slug] = cache_plugin


	with open(cache_filename, 'w') as f:
		json.dump(cache, f, indent=2)


if __name__ == "__main__":
	update()
