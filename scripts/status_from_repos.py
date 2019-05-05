import sys
import json
import os

for filename in sys.argv[1:]:
	slug = os.path.splitext(os.path.basename(filename))[0]
	with open(filename, "r") as f:
		manifest = json.load(f)

	if 'license' in manifest:
		print(f"{slug}:\t\t\t{manifest['license']}")
	else:
		print(f"{slug}:\t\t\tnone")
