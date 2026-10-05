#!/usr/bin/env python3

import argparse
import os
import json
import shutil
from concurrent.futures import ThreadPoolExecutor

from py.command import run
from py.console import choose, info, warn
from py import config, manifest_cache, modulargrid
from py.plugin import Plugin, version_key

try:
	from py import agent
except ImportError:
	agent = None


def run_retry(command, *args, cwd=None):
	while True:
		try:
			run(command, *args, cwd=cwd, capture_stderr=True)
			return
		except Exception as error:
			warn(error)
			if choose("[r]etry, [i]gnore: ", "ri") == "i":
				return


def clear_toolchain_build_dir():
	info("Clearing toolchain build directory")
	run("make", "plugin-build-clean", cwd=config.TOOLCHAIN_DIR, capture_stderr=True)


def load(paths=None):
	# If no paths are given, use directories in config.REPOS_DIR that contain plugin.json.
	if not paths:
		info(f"Checking {config.REPOS_DIR} for new plugin versions")
		paths = []
		for entry in os.scandir(config.REPOS_DIR):
			if not entry.is_dir():
				continue
			manifest_path = os.path.join(entry.path, "plugin.json")
			if os.path.isfile(manifest_path):
				paths.append(entry.path)
		paths.sort()

	# Create Plugins from paths.
	plugins = []
	packaged_plugins = {}
	for path in paths:
		if os.path.isdir(path):
			plugin = Plugin()
			plugin.set_source_dir(path)
			plugins.append(plugin)
		elif path.endswith(".vcvplugin"):
			filename = os.path.basename(path)
			basename = os.path.splitext(filename)[0]
			parts = basename.split("-")
			key = "-".join(parts[:-2])
			# Add package to existing plugin if SLUG-VERSION matches, or create new Plugin.
			if key not in packaged_plugins:
				packaged_plugins[key] = Plugin()
				plugins.append(packaged_plugins[key])
			packaged_plugins[key].add_package_path(path)
		else:
			warn(f"{path!r} is not a directory or a .vcvplugin archive.")

	# Load manifests and select Plugins to update.
	loaded_plugins = []
	for plugin in plugins:
		try:
			plugin.extract_packages()
			plugin.load_manifest()
			plugin.load_library_manifest()

			# Skip plugins that are not newer than the library manifest
			if plugin.library_manifest is not None and version_key(plugin.get_version()) <= version_key(plugin.library_manifest['version']):
				continue
			if any(loaded_plugin.get_slug() == plugin.get_slug() for loaded_plugin in loaded_plugins):
				raise ValueError(f"Plugin {plugin.get_slug()!r} is already loaded.")
			loaded_plugins.append(plugin)
		except Exception as error:
			warn(f"{plugin.source_dir or plugin.package_paths}: {error}")
	return loaded_plugins


def review(paths=None):
	plugins = load(paths)
	has_source_plugins = any(plugin.source_dir for plugin in plugins)
	if has_source_plugins:
		clear_toolchain_build_dir()
	try:
		reviewed_plugins = review_plugins(plugins)
	finally:
		if has_source_plugins:
			clear_toolchain_build_dir()


def review_plugins(plugins):
	reviewed_plugins = []
	for plugin in plugins:
		try:
			info(f"Reviewing {plugin.get_slug()}")
			has_warnings = False
			if warnings := plugin.review_manifest():
				warn(warnings)
				has_warnings = True
			if warnings := plugin.review_packages():
				warn(warnings)
				has_warnings = True
			if warnings := plugin.review_git():
				warn(warnings)
				has_warnings = True
			if warnings := plugin.review_source():
				warn(warnings)
				has_warnings = True
			if has_warnings and choose("[a]pprove, [r]eject: ", "ar") == "r":
				continue

			# Show package manifest for approval.
			if not plugin.source_dir:
				print(json.dumps(plugin.manifest, indent="  "))
				choose("Press Enter to approve manifest: ", "\n")

			reviewed_plugins.append(plugin)
		except Exception as error:
			warn(f"{plugin.get_slug()}: {error}")
			choose("[r]eject: ", "r")

	reviewed_plugins = review_plugins_with_cppcheck(reviewed_plugins)
	if agent and any(plugin.source_dir for plugin in reviewed_plugins):
		info("Reviewing source code with agent")
		reviewed_plugins = review_plugins_concurrently(reviewed_plugins, agent.review_source)
	built_plugins = build_plugins(reviewed_plugins)
	if agent and built_plugins:
		info("Reviewing binaries with agent")
		built_plugins = review_plugins_concurrently(built_plugins, agent.review_dist)
	installed_plugins = install_plugins(built_plugins)
	run_plugins(installed_plugins)
	return installed_plugins


def approve_warnings(plugin, warnings):
	if not warnings:
		return True
	warn(f"{plugin.get_slug()}: {warnings}")
	return choose("[a]pprove, [r]eject: ", "ar") == "a"


def review_plugins_with_cppcheck(plugins):
	reviewed_plugins = []
	for plugin in plugins:
		try:
			warnings = plugin.review_source_with_cppcheck()
		except Exception as error:
			warn(f"{plugin.get_slug()}: {error}")
			choose("[r]eject: ", "r")
			continue
		if not approve_warnings(plugin, warnings):
			continue
		reviewed_plugins.append(plugin)
	return reviewed_plugins


def review_plugins_concurrently(plugins, func):
	if not plugins:
		return []

	with ThreadPoolExecutor(max_workers=len(plugins)) as executor:
		futures = [executor.submit(func, plugin) for plugin in plugins]

	reviewed_plugins = []
	for plugin, future in zip(plugins, futures):
		try:
			warnings = future.result()
		except Exception as error:
			warnings = f"Review failed.\n\n{error}"
		if not approve_warnings(plugin, warnings):
			continue
		reviewed_plugins.append(plugin)
	return reviewed_plugins


def build_plugins(plugins):
	built_plugins = []
	for plugin in plugins:
		try:
			if plugin.source_dir:
				info(f"Building {plugin.get_slug()}")
				plugin.build_source()
			built_plugins.append(plugin)
		except Exception as error:
			warn(f"{plugin.get_slug()}: {error}")
			choose("[r]eject: ", "r")
	return built_plugins


def install_plugins(plugins):
	# Install packages to Rack plugin directory.
	installed_plugins = []
	for plugin in plugins:
		try:
			info(f"Installing {plugin.get_slug()} {plugin.get_version()}")
			plugin.install_package()
			installed_plugins.append(plugin)
		except Exception as error:
			warn(f"{plugin.get_slug()}: {error}")
			choose("[r]eject: ", "r")
	return installed_plugins


def run_plugins(plugins):
	if not plugins:
		return

	# Test plugins
	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in plugins)
	print()
	choose(f"Press Enter to launch Rack and test the following packages: {update_message}", "\n")
	run_retry("./Rack", cwd=config.RACK_SYSTEM_DIR)

	# Show warnings and debug messages from Rack log.
	log_path = os.path.join(config.RACK_USER_DIR, "log.txt")
	output = run("grep", "-E", r"\bwarn|debug\b", log_path, capture_stdout=True, raise_on_nonzero=False)
	if output:
		warn(output.rstrip("\n"))
		choose("[a]pprove: ", "a")


def publish_plugins(plugins):
	published_plugins = []
	for plugin in plugins:
		try:
			info(f"Publishing {plugin.get_slug()} {plugin.get_version()}")
			plugin.publish_packages()
			published_plugins.append(plugin)
		except Exception as error:
			warn(f"{plugin.get_slug()}: {error}")
			choose("[r]eject: ", "r")
	return published_plugins


def publish(paths=None):
	# Pull library repo and update submodules
	info("Pulling library repo")
	run_retry("git", "pull")
	info("Synchronizing submodule URLs")
	run("git", "submodule", "sync", "--recursive", "--quiet", capture_stderr=True)
	info("Updating submodules")
	run("git", "submodule", "update", "--init", "--recursive", capture_stderr=True)

	# Load, review, and publish plugins
	loaded_plugins = load(paths)
	has_source_plugins = any(plugin.source_dir for plugin in loaded_plugins)
	if has_source_plugins:
		clear_toolchain_build_dir()
	try:
		reviewed_plugins = review_plugins(loaded_plugins)
		published_plugins = publish_plugins(reviewed_plugins)
	finally:
		if has_source_plugins:
			clear_toolchain_build_dir()

	if not published_plugins:
		info("No plugins to publish.")
		return

	# Update manifest cache and ModularGrid database
	while True:
		try:
			info("Refreshing manifest cache")
			manifest_cache.refresh_cache(published_plugins)
			info("Updating ModularGrid database")
			modulargrid.update(published_plugins)
			break
		except Exception as error:
			warn(error)
			if choose("[r]etry, [i]gnore: ", "ri") == "i":
				break

	choose("Press Enter to generate screenshots, upload packages and screenshots, and commit/push the library repo: ", "\n")

	# Delete plugin's old screenshots
	for plugin in published_plugins:
		screenshots_dir = os.path.join(config.RACK_SCREENSHOTS_DIR, plugin.get_slug())
		try:
			shutil.rmtree(screenshots_dir)
		except FileNotFoundError:
			pass
	info("Generating screenshots")
	run_retry("./Rack", "-t", "4", cwd=config.RACK_SYSTEM_DIR)

	# Resize screenshots
	info("Resizing screenshots")
	run_retry("make", f"-j{os.cpu_count() or 1}", cwd=config.SCREENSHOTS_DIR)

	# Upload packages
	info("Uploading packages")
	run_retry("make", "upload", cwd=config.PACKAGES_DIR)

	# Upload screenshots
	info("Uploading screenshots")
	run_retry("make", "upload", cwd=config.SCREENSHOTS_DIR)

	# Commit and push library repo
	info("Committing manifests")
	manifest_paths = []
	for plugin in published_plugins:
		path = os.path.join(config.MANIFESTS_DIR, f"{plugin.get_slug()}.json")
		with open(path, "w", encoding="utf-8") as f:
			json.dump(plugin.manifest, f, indent="  ")
		manifest_paths.append(path)
	run("git", "add", "--", *manifest_paths, config.MANIFESTS_CACHE_FILE, config.MODULARGRID_FILE, capture_stderr=True)
	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in published_plugins)
	run_retry("git", "commit", "-m", f"Update manifest {update_message}")

	info("Pushing library repo")
	run_retry("git", "push")

	info(f"Updated {update_message}")
	for plugin in published_plugins:
		# Open browser to plugin's GitHub library issue
		os.system(f"xdg-open 'https://github.com/VCVRack/library/issues?q=is%3Aissue+sort%3Aupdated-desc+in%3Atitle+{plugin.get_slug()}' &")


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	commands = parser.add_subparsers(dest="command")

	review_parser = commands.add_parser("review", help="Review source directories and package archives")
	review_parser.add_argument("paths", nargs="*", metavar="PATH", help="Source directory or .vcvplugin archive")

	publish_parser = commands.add_parser("publish", help="Review and publish source directories and package archives")
	publish_parser.add_argument("paths", nargs="*", metavar="PATH", help="Source directory or .vcvplugin archive")

	args = parser.parse_args()
	if not args.command:
		parser.print_help()
	elif args.command == "review":
		review(args.paths)
	elif args.command == "publish":
		publish(args.paths)
