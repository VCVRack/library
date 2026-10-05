import json
import time

from . import config
from .console import info


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

		build_timestamp = plugin.get_package_timestamp()
		if build_timestamp is not None:
			cache_plugin['buildTimestamp'] = build_timestamp

		# Get plugin creation
		if 'creationTimestamp' not in cache_plugin:
			info(f"Getting creationTimestamp for plugin {plugin.get_slug()}")
			creation_timestamp = plugin.get_creation_timestamp()
			cache_plugin['creationTimestamp'] = creation_timestamp if creation_timestamp is not None else publish_timestamp

		cache_modules = cache_plugin.get('modules', {})
		modules = plugin.manifest.get('modules', [])
		missing_module_slugs = [module['slug'] for module in modules if 'creationTimestamp' not in cache_modules.get(module['slug'], {})]
		creation_timestamps = plugin.get_module_creation_timestamps(missing_module_slugs)
		for module in modules:
			module_slug = module['slug']
			cache_module = cache_modules.get(module_slug, {})

			# Get module creation
			if 'creationTimestamp' not in cache_module:
				info(f"Getting creationTimestamp for plugin {plugin.get_slug()} module {module_slug}")
				creation_timestamp = creation_timestamps[module_slug]
				cache_module['creationTimestamp'] = creation_timestamp if creation_timestamp is not None else publish_timestamp

			cache_modules[module_slug] = cache_module
		cache_plugin['modules'] = cache_modules

		cache[plugin.get_slug()] = cache_plugin

	with open(cache_filename, 'w') as f:
		json.dump(cache, f, indent=2)
