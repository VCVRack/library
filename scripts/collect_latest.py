import sys
import os
import glob
import json


def system(cmd):
	if os.system(cmd):
		raise Exception(f"Failed command: {cmd}")


PACKAGES_DIR = '../packages'
DOWNLOADS_DIR = '../downloads'


for manifest_filename in glob.glob('manifests/*.json'):
	slug = os.path.splitext(os.path.basename(manifest_filename))[0]
	with open(manifest_filename, "r") as f:
		manifest = json.load(f)

	if "authorEmail" in manifest:
		print(manifest["authorEmail"])

	# version = manifest['version']
	# arch = 'win'
	# package_filename = os.path.join(PACKAGES_DIR, f"{slug}-{version}-{arch}.zip")

	# try:
	# 	system(f'cp {package_filename} {DOWNLOADS_DIR}')
	# except Exception as e:
	# 	print(e)
