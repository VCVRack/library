import glob
import json
import time
import os


def system(cmd):
	if os.system(cmd):
		raise Exception(f"Failed command: {cmd}")


def build_mac(slug):
	env = f'CC=x86_64-apple-darwin15-clang CXX=x86_64-apple-darwin15-clang++-libc++ STRIP=x86_64-apple-darwin15-strip RACK_DIR=../../Rack-SDK'
	make = f'{env} make -j$(nproc) -C repos/{slug}'
	system(f'{make} clean')
	system(f'{make} dist')


def build_win(slug):
	env = f'CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ STRIP=x86_64-w64-mingw32-strip RACK_DIR=../../Rack-SDK'
	make = f'{env} make -j$(nproc) -C repos/{slug}'
	system(f'{make} clean')
	system(f'{make} dist')


def build_lin(slug):
	env = f'-e RACK_DIR=../../Rack-SDK'
	make = f'make -j$(nproc) -C repos/{slug}'
	system(f'docker run --rm -v $(pwd):/mnt -u vortico {env} a0b9c87ec456 {make} clean')
	system(f'docker run --rm -v $(pwd):/mnt -u vortico {env} a0b9c87ec456 {make} dist')


def move_package(slug):
	system('mkdir -p downloads')
	system(f'mv repos/{slug}/dist/{slug}-*.zip downloads/')


def delete_package(slug):
	system(f'rm -f downloads/{slug}-*.zip')


for filename in glob.glob("manifests/*"):
	slug = os.path.splitext(os.path.basename(filename))[0]
	with open(filename, "r") as f:
		manifest = json.load(f)

	# Skip if update is not needed
	if 'repoVersion' not in manifest:
		continue
	if 'latestVersion' in manifest and manifest['latestVersion'] == manifest['repoVersion']:
		continue
	if not os.path.exists('repos/' + slug):
		continue

	try:
		build_mac(slug)
		move_package(slug)
		build_win(slug)
		move_package(slug)
		build_lin(slug)
		move_package(slug)
	except Exception as e:
		print(e)
		input("Enter to proceed")
		delete_package(slug)
		continue

	# Update build information
	manifest['latestVersion'] = manifest['repoVersion']
	manifest['buildTimestamp'] = round(time.time())
	manifest['status'] = "available"

	with open(filename, "w") as f:
		json.dump(manifest, f, indent="  ")
