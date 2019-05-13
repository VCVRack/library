import os
import sys


RACK_SDK = os.path.abspath("Rack-SDK")


def system(cmd):
	if os.system(cmd):
		raise Exception(f"Failed command: {cmd}")


def stage_package(plugin_dir):
	system('mkdir -p stage')
	system(f'mv {plugin_dir}/dist/*.zip stage/')


def delete_stage():
	system(f'rm -rf stage')


def build_mac(plugin_dir):
	env = f'CC=x86_64-apple-darwin15-clang CXX=x86_64-apple-darwin15-clang++-libc++ STRIP=x86_64-apple-darwin15-strip RACK_DIR={RACK_SDK}'
	make = f'{env} make -j$(nproc) -C {plugin_dir}'
	system(f'{make} clean')
	system(f'{make} dist')
	stage_package(plugin_dir)
	system(f'{make} clean')


def build_win(plugin_dir):
	env = f'CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ STRIP=x86_64-w64-mingw32-strip RACK_DIR={RACK_SDK}'
	make = f'{env} make -j$(nproc) -C {plugin_dir}'
	system(f'{make} clean')
	system(f'{make} dist')
	stage_package(plugin_dir)
	system(f'{make} clean')


def build_lin(plugin_dir):
	env = f'-e RACK_DIR=/Rack-SDK'
	make = f'make -j$(nproc)'
	plugin_abs = os.path.abspath(plugin_dir)
	# TODO Make this Docker image publicly available
	# It's essentially just Ubuntu 16.04 with plugin build dependencies installed, the workdir, and a user account set up so it matches my own machine's UID to solve file permissions issues.
	docker = f'docker run --rm -v {RACK_SDK}:/Rack-SDK -v {plugin_abs}:/workdir -w /workdir -u vortico {env} d91c37409ded'
	system(f'{docker} {make} clean')
	system(f'{docker} {make} dist')
	stage_package(plugin_dir)
	system(f'{docker} {make} clean')


def build(plugin_dir):
	try:
		build_lin(plugin_dir)
		build_win(plugin_dir)
		build_mac(plugin_dir)
	except Exception as e:
		delete_stage()
		print(e)
		return False
	return True


if __name__ == "__main__":
	plugin_dir = sys.argv[1]
	if not plugin_dir:
		raise "No plugin_dir given"
	build(plugin_dir)
