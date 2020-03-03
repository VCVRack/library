import sys
import os
import glob
import json
import re
import requests


# Load existing dataset
mg_filename = "ModularGrid-VCVLibrary.json"
with open(mg_filename) as f:
	mg = json.load(f)

# Iterate plugins
for manifest_filename in glob.glob('manifests/*.json'):
	with open(manifest_filename) as f:
		plugin = json.load(f)
	plugin_slug = plugin['slug']

	# Iterate modules in plugin
	for module in plugin.get('modules', []):
		module_slug = module['slug']
		mg_url = module.get('modularGridUrl')
		if not mg_url:
			continue

		if [x for x in mg if x.get('mgUrl') == mg_url]:
			continue

		mg_data = {}
		mg_data['pluginSlug'] = plugin_slug
		mg_data['moduleSlug'] = module_slug
		mg_data['vcvUrl'] = f"https://library.vcvrack.com/{plugin_slug}/{module_slug}"
		mg_data['mgUrl'] = mg_url
		mg.append(mg_data)
		print(mg_data)


# Iterate dataset
for mg_data in mg:
	if mg_data.get('mgModuleId'):
		continue

	# Scrape ModularGrid website for ID
	mg_url = mg_data['mgUrl']
	r = requests.get(mg_url)
	m = re.search(r'data-module-id = "(\d+)"', r.text)
	mg_id = m.group(1)
	if not mg_id:
		print(f"No ModularGrid ID found for {plugin_slug} {module_slug}")
		continue
	mg_id = int(mg_id)
	mg_data['mgModuleId'] = mg_id
	print(mg_data)


with open(mg_filename, 'w') as f:
	json.dump(mg, f, indent=2)
