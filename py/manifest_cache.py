import json
import time

from . import config


def refresh_cache(plugins):
	cache_filename = config.MANIFESTS_CACHE_FILE
	try:
		with open(cache_filename) as f:
			cache = json.load(f)
	except FileNotFoundError:
		cache = {}

	# Refresh only the submitted plugin entries.
	publish_timestamp = time.time()
	for plugin in plugins:
		cache_plugin = cache.get(plugin.get_slug(), {})

		cache_plugin['buildTimestamp'] = publish_timestamp

		# If plugin is new, set its creation timestamp
		if 'creationTimestamp' not in cache_plugin:
			cache_plugin['creationTimestamp'] = publish_timestamp

		cache_modules = cache_plugin.get('modules', {})
		modules = plugin.manifest.get('modules', [])
		for module in modules:
			module_slug = module['slug']
			cache_module = cache_modules.get(module_slug, {})

			# If module is new, set its creation timestamp
			if 'creationTimestamp' not in cache_module:
				cache_module['creationTimestamp'] = publish_timestamp

			cache_modules[module_slug] = cache_module
		cache_plugin['modules'] = cache_modules

		cache[plugin.get_slug()] = cache_plugin

	with open(cache_filename, 'w') as f:
		json.dump(cache, f, indent=2)
