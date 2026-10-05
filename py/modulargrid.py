import json
import re

from . import config


def get_module_id(url):
	import requests
	response = requests.get(url, timeout=config.HTTP_TIMEOUT)
	response.raise_for_status()
	match = re.search(r'data-module-id\s*=\s*"(\d+)"', response.text)
	if not match:
		raise ValueError(f"ModularGrid page {url!r} contains no module ID.")
	return int(match.group(1))


def update(plugins):
	# This dataset is standardized by ModularGrid. Do not change the schema or filename.
	with open(config.MODULARGRID_FILE) as f:
		entries = json.load(f)

	for plugin in plugins:
		plugin_slug = plugin.get_slug()
		manifest = plugin.manifest

		for module in manifest.get('modules', []):
			module_slug = module['slug']
			url = module.get('modularGridUrl')
			if not url:
				continue
			if any(entry['pluginSlug'] == plugin_slug and 'moduleSlug' in entry and entry['moduleSlug'] == module_slug and entry['mgUrl'] == url for entry in entries):
				continue

			entry = {}
			entry['pluginSlug'] = plugin_slug
			entry['moduleSlug'] = module_slug
			entry['vcvUrl'] = f"https://library.vcvrack.com/{plugin_slug}/{module_slug}"
			entry['mgUrl'] = url
			entry['mgModuleId'] = get_module_id(url)
			entries.append(entry)
			print(entry)

	with open(config.MODULARGRID_FILE, 'w') as f:
		json.dump(entries, f, indent=2)
