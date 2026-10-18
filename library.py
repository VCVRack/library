#!/usr/bin/env python3

import argparse
import sys
import os
import json
import shutil
import webbrowser
from concurrent.futures import ThreadPoolExecutor, as_completed

from py.command import run
from py.console import choose, error, info, warn
from py import config, manifest_cache, modulargrid
from py.plugin import Plugin

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


def resolve_path(path):
	# Accept package paths
	if path.endswith(".vcvplugin"):
		return path

	# Accept existing directories
	if os.path.isdir(path):
		return path

	# Accept explicit paths even if they do not exist
	if os.path.dirname(path):
		return path

	# Convert slug to repository path
	return os.path.join(config.REPOS_DIR, path)


def resolve_paths(paths=None):
	if paths:
		# Transform slugs to repo paths.
		paths = [resolve_path(path) for path in paths]

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
	return paths


def load(paths=None, commit=None, repo_url=None):
	"""Loads source directories, package paths, or repository slugs into Plugins that are new to the library or have newer versions."""
	if commit or repo_url:
		if not paths or len(paths) != 1:
			raise ValueError("--commit and --git require exactly one source directory or slug.")
	paths = resolve_paths(paths)

	# Create Plugins from paths.
	plugins = []
	packaged_plugins = {}
	for path in paths:
		if os.path.isdir(path) or repo_url:
			plugin = Plugin()
			plugin.set_source_dir(path)
			plugin.repo_url = repo_url
			plugin.repo_commit = commit
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
			error(f"{path!r} is not a directory or a .vcvplugin archive.")

	# Load manifests and select Plugins to update.
	loaded_plugins = []
	for plugin in plugins:
		try:
			plugin.checkout_repo()
			plugin.load_manifest()
			plugin.load_library_manifest()

			# Skip plugins that are not newer than the library manifest
			if not plugin.is_new_version():
				continue
			if any(loaded_plugin.get_slug() == plugin.get_slug() for loaded_plugin in loaded_plugins):
				raise ValueError(f"Plugin {plugin.get_slug()!r} is already loaded.")
			loaded_plugins.append(plugin)
		except Exception as exception:
			error(f"{plugin.source_dir or plugin.package_paths}: {exception}")
	return loaded_plugins


def review(paths=None, commit=None, repo_url=None):
	pull_library()
	plugins = load(paths, commit, repo_url)
	if any(plugin.source_dir for plugin in plugins):
		clear_toolchain_build_dir()
	try:
		reviewed_plugins = review_plugins(plugins)
	finally:
		if any(plugin.source_dir for plugin in plugins):
			clear_toolchain_build_dir()
	if not reviewed_plugins:
		info("No plugins to review")
		return

	commit_paths = add_git_submodules(reviewed_plugins)
	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in reviewed_plugins)
	commit_library(commit_paths, f"Update {update_message}")


def review_plugins(plugins):
	reviewed_plugins = []
	for plugin in plugins:
		try:
			info(f"Reviewing {plugin.get_slug()}")
			warnings = []
			if warning := plugin.review_git():
				warnings.append(warning)
			if warning := plugin.review_source():
				warnings.append(warning)
			if warning := plugin.review_manifest():
				warnings.append(warning)
			if warnings:
				warn(f"{plugin.get_slug()} issues")
				warn("\n".join(warnings))
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
					continue

			# Show package manifest for approval.
			if not plugin.source_dir:
				print(json.dumps(plugin.manifest, indent="  "))
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
					continue

			reviewed_plugins.append(plugin)
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
			choose("[r]eject: ", "r")

	reviewed_plugins = review_plugins_with_cppcheck(reviewed_plugins)
	if agent and any(plugin.source_dir for plugin in reviewed_plugins):
		info("Reviewing source code with agent")
		reviewed_plugins = review_plugins_concurrently(reviewed_plugins, agent.review_source)
	built_plugins = build_plugins(reviewed_plugins)

	# Review submitted and built packages
	reviewed_plugins = []
	for plugin in built_plugins:
		try:
			if warnings := plugin.review_packages():
				warn(f"{plugin.get_slug()} issues\n{warnings}")
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
					continue
			reviewed_plugins.append(plugin)
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
			choose("[r]eject: ", "r")

	if agent and reviewed_plugins:
		info("Reviewing binaries with agent")
		reviewed_plugins = review_plugins_concurrently(reviewed_plugins, agent.review_dist)
	installed_plugins = install_plugins(reviewed_plugins)
	run_plugins(installed_plugins)
	return installed_plugins


def review_plugins_with_cppcheck(plugins):
	reviewed_plugins = []
	for plugin in plugins:
		try:
			warnings = plugin.review_source_with_cppcheck()
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
			choose("[r]eject: ", "r")
			continue
		if warnings:
			warn(f"{plugin.get_slug()} issues\n{warnings}")
			if choose("[a]pprove, [r]eject: ", "ar") == "r":
				continue
		reviewed_plugins.append(plugin)
	return reviewed_plugins


def review_plugins_concurrently(plugins, func):
	if not plugins:
		return []

	with ThreadPoolExecutor(max_workers=len(plugins)) as executor:
		futures = {executor.submit(func, plugin): plugin for plugin in plugins}

		reviewed_plugins = []
		for future in as_completed(futures):
			plugin = futures[future]
			try:
				warnings = future.result()
			except Exception as error:
				warnings = f"Review failed.\n\n{error}"
			if warnings:
				warn(f"{plugin.get_slug()} issues\n{warnings}")
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
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
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
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
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
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
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
			choose("[r]eject: ", "r")
	return published_plugins


def add_git_submodules(plugins):
	"""If any given Plugins are new submodules, adds them to Git and returns the paths to commit."""
	commit_paths = []
	for plugin in plugins:
		# Only include source directories inside the library's repos directory
		if not plugin.is_source_in_library():
			continue

		repo_path = os.path.relpath(plugin.source_dir)
		if plugin.repo_url:
			run("git", "submodule", "add", "--", plugin.repo_url, repo_path, capture_stderr=True)
			if ".gitmodules" not in commit_paths:
				commit_paths.append(".gitmodules")
		commit_paths.append(repo_path)
	return commit_paths


def commit_library(paths, message):
	if not paths:
		return
	run("git", "add", "--", *paths, capture_stderr=True)
	changes = run("git", "diff", "--cached", "--name-only", "--", *paths, capture_stdout=True)
	if not changes:
		info("No library changes to commit")
		return

	info("Committing library repo")
	run("git", "commit", "--only", "-m", message, "--", *paths, capture_stderr=True)
	info("Pushing library repo")
	run_retry("git", "push")


def pull_library():
	info("Pulling library repo")
	run_retry("git", "pull")


def publish(paths=None, commit=None, repo_url=None):
	pull_library()

	# Load, review, and publish plugins
	loaded_plugins = load(paths, commit, repo_url)
	if any(plugin.source_dir for plugin in loaded_plugins):
		clear_toolchain_build_dir()
	try:
		reviewed_plugins = review_plugins(loaded_plugins)
		published_plugins = publish_plugins(reviewed_plugins)
	finally:
		if any(plugin.source_dir for plugin in loaded_plugins):
			clear_toolchain_build_dir()

	if not published_plugins:
		info("No plugins to publish")
		return

	# Update manifest cache
	info("Refreshing manifest cache")
	manifest_cache.refresh_cache(published_plugins)

	# Update ModularGrid database
	while True:
		try:
			info("Updating ModularGrid database")
			modulargrid.update(published_plugins)
			break
		except Exception as error:
			warn(error)
			if choose("[r]etry, [i]gnore: ", "ri") == "i":
				break

	choose("Press Enter to generate screenshots, upload packages and screenshots, and commit/push the library repo", "\n")

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

	commit_paths = add_git_submodules(published_plugins)

	# Copy manifests to library
	for plugin in published_plugins:
		path = os.path.join(config.MANIFESTS_DIR, f"{plugin.get_slug()}.json")
		with open(path, "w", encoding="utf-8") as f:
			json.dump(plugin.manifest, f, indent="  ")
		commit_paths.append(path)

	commit_paths.append(config.MANIFESTS_CACHE_FILE)
	commit_paths.append(config.MODULARGRID_FILE)
	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in published_plugins)
	commit_library(commit_paths, f"Update manifest {update_message}")

	for plugin in published_plugins:
		if not plugin.is_source_in_library():
			continue
		# Open browser to plugin's GitHub library issue
		url = f"https://github.com/VCVRack/library/issues?q=is%3Aissue+sort%3Aupdated-desc+in%3Atitle+{plugin.get_slug()}"
		webbrowser.open(url)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.set_defaults(action="review")
	actions = parser.add_mutually_exclusive_group()
	actions.add_argument("-p", "--publish", dest="action", action="store_const", const="publish", help="Review plugins, then publish packages, screenshots, and library changes.")
	parser.add_argument("paths", nargs="*", metavar="PATH", help=f"Plugin slugs, source directories, or .vcvplugin archives. With no paths, scan {config.REPOS_DIR} for new versions.")
	parser.add_argument("-c", "--commit", metavar="HASH", help="Fetch and check out this commit in one source directory or slug.")
	parser.add_argument("-g", "--git", dest="repo_url", metavar="URL", help="Clone a repository or change its URL.")

	info("""\
⢿⣦  ⣴⡿ ⣠⣶⠿⠿⣶ ⢿⣦  ⣴⡿
⠈⢿⣧⣼⡿⠁ ⣿⡇⢾⡷  ⠈⢿⣧⣼⡿⠁
 ⠈⢿⡿⠁  ⠙⠿⣶⣶⠿  ⠈⢿⡿⠁ Library release script\n""")

	args = parser.parse_args()
	if len(sys.argv) == 1:
		parser.print_help()
	elif args.action == "review":
		review(args.paths, args.commit, args.repo_url)
	elif args.action == "publish":
		publish(args.paths, args.commit, args.repo_url)
