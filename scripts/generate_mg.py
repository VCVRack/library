import sys
import os
import glob
import json
import re
import requests


mg_filename = "ModularGrid-VCVLibrary.json"
# with open(mg_filename) as f:
# 	mg = json.load(f)
mg = []

for manifest_filename in glob.glob('manifests/*.json'):
	slug = os.path.splitext(os.path.basename(manifest_filename))[0]
	with open(manifest_filename) as f:
		plugin = json.load(f)

	for module in plugin.get("modules", []):
		mg_url = module.get("modularGridUrl")
		if not mg_url:
			continue

		r = requests.get(mg_url)
		m = re.search(r'data-module-id = "(\d+)"', r.text)
		mg_id = m.group(1)
		if not mg_id:
			continue
		mg_id = int(mg_id)

		library_url = f"https://library.vcvrack.com/{plugin['slug']}/{module['slug']}"
		module_data = {"mgModuleId": mg_id, "vcvUrl": library_url}
		mg.append(module_data)
		print(mg_id)
		print(library_url)

with open(mg_filename, 'w') as f:
	json.dump(mg, f, indent=2)
