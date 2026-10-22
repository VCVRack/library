#!/usr/bin/env python3

import argparse
import sys
import os
import json
import shutil
import webbrowser
from concurrent.futures import ThreadPoolExecutor, as_completed
from urllib.parse import urlsplit

import requests

from py.command import run
from py.console import choose, error, info, warn
from py import config, manifest_cache, modulargrid
from py.plugin import Plugin, is_valid_slug

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


def create_plugins(paths=None, commit=None, repo_url=None):
	"""Creates Plugins from source directories, package paths, or repository slugs."""
	# If a GitHub URL is given but no path, read the plugin slug from plugin.json.
	if repo_url and not paths:
		url = urlsplit(repo_url)
		if url.hostname != "github.com":
			raise ValueError(f"Repository URL {repo_url!r} is not a GitHub URL. A source directory or slug is required.")
		owner, repo = url.path.strip("/").split("/")
		repo = repo.removesuffix(".git")
		response = requests.get(f"https://api.github.com/repos/{owner}/{repo}/contents/plugin.json", params={"ref": commit}, headers={"Accept": "application/vnd.github.raw+json"}, timeout=config.HTTP_TIMEOUT)
		response.raise_for_status()
		slug = response.json().get("slug")
		if not slug:
			raise ValueError("`plugin.json` has no `slug` value.")
		if not is_valid_slug(slug):
			raise ValueError(f"`plugin.json` has an invalid `slug` value `{slug}`.")
		paths = [os.path.join(config.REPOS_DIR, slug)]

	if commit or repo_url:
		if not paths or len(paths) != 1:
			raise ValueError("--commit and --git require exactly one source directory or slug.")
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
	return plugins


def load_plugins(plugins):
	"""Loads Plugins and returns those with versions newer than the library."""
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


def cleanup_plugins(plugins):
	"""Undoes repository changes that were not accepted."""
	for plugin in plugins:
		try:
			plugin.undo_repo()
		except Exception as exception:
			warn(f"Could not restore {plugin.source_dir!r}. {exception}")


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
				if warnings:
					warnings = f"Agent reported issues with {plugin.get_slug()}.\n{warnings}"
			except Exception as exception:
				warnings = f"Agent review of {plugin.get_slug()} failed.\n{exception}"
			if warnings:
				warn(warnings)
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
					continue
			reviewed_plugins.append(plugin)
	return reviewed_plugins


def build_plugins(plugins):
	if any(plugin.source_dir for plugin in plugins):
		run("make", "plugin-build-clean", cwd=config.TOOLCHAIN_DIR, capture_stderr=True)

	built_plugins = []
	for plugin in plugins:
		try:
			if plugin.source_dir:
				info(f"Building {plugin.get_slug()}")
				plugin.build_source()
			built_plugins.append(plugin)
		except Exception as exception:
			error(str(exception))
			choose("[r]eject: ", "r")
	return built_plugins


def run_plugins(plugins):
	# Install packages to Rack plugin directory.
	installed_plugins = []
	for plugin in plugins:
		if plugin.install_package():
			installed_plugins.append(plugin)
	if not installed_plugins:
		return

	# Test plugins
	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in installed_plugins)
	choose(f"Press Enter to launch Rack and test the following packages: {update_message}", "\n")
	run_retry("./Rack", cwd=config.RACK_SYSTEM_DIR)

	# Show warnings and debug messages from Rack log.
	log_path = os.path.join(config.RACK_USER_DIR, "log.txt")
	output = run("grep", "-E", r"\b(warn|debug)\b", log_path, capture_stdout=True, raise_on_nonzero=False)
	if output:
		warn(output.rstrip("\n"))
		choose("[a]pprove: ", "a")


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

	if agent and any(plugin.source_dir for plugin in reviewed_plugins):
		info("Reviewing source code with agent")
		reviewed_plugins = review_plugins_concurrently(reviewed_plugins, agent.review_source)
	built_plugins = build_plugins(reviewed_plugins)

	# Review submitted and built packages
	reviewed_plugins = []
	for plugin in built_plugins:
		try:
			info(f"Reviewing {plugin.get_slug()} packages")
			if warnings := plugin.review_packages():
				warn(warnings)
				if choose("[a]pprove, [r]eject: ", "ar") == "r":
					continue
			reviewed_plugins.append(plugin)
		except Exception as exception:
			error(f"{plugin.get_slug()}: {exception}")
			choose("[r]eject: ", "r")

	if agent and reviewed_plugins:
		info("Reviewing binaries with agent")
		reviewed_plugins = review_plugins_concurrently(reviewed_plugins, agent.review_dist)
	run_plugins(reviewed_plugins)
	return reviewed_plugins


def get_repo_commit_paths(plugins):
	"""Returns paths to commit to update the plugins."""
	commit_paths = set()
	for plugin in plugins:
		# Only include source directories inside the library's repos directory
		if not plugin.is_source_in_library():
			continue

		if plugin.repo_url or plugin.repo_commit:
			commit_paths.add(plugin.source_dir)
		if plugin.repo_url:
			commit_paths.add(".gitmodules")
	return commit_paths


def commit_library(paths, message):
	if not paths:
		return

	info("Committing library repo")
	run("git", "add", "--", *paths, capture_stderr=True)
	run("git", "commit", "--only", "-m", message, "--", *paths, capture_stderr=True)


def pull_library():
	info("Pulling library repo")
	run_retry("git", "pull")

	info("Synchronizing submodule URLs")
	run("git", "submodule", "sync", "--recursive", capture_stderr=True)

	info("Updating library submodules")
	run("git", "submodule", "update", "--init", "--recursive", capture_stderr=True)


def review(paths=None, commit=None, repo_url=None, publish=False):
	pull_library()
	plugins = create_plugins(paths, commit, repo_url)
	try:
		loaded_plugins = load_plugins(plugins)
		reviewed_plugins = review_plugins(loaded_plugins)
		if not reviewed_plugins:
			info("No plugins to review")
			return

		update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in reviewed_plugins)
		if publish:
			choose(f"Press Enter to publish the following plugins: {update_message}", "\n")
			published_plugins = []
			for plugin in reviewed_plugins:
				try:
					plugin.publish_packages()
					published_plugins.append(plugin)
				except Exception as exception:
					error(f"{plugin.get_slug()}: {exception}")
					choose("[r]eject: ", "r")
			reviewed_plugins = published_plugins
			if not reviewed_plugins:
				return
		else:
			commit_paths = get_repo_commit_paths(reviewed_plugins)
			if not commit_paths:
				return
			info(f"Commit and push the following plugins to the library: {update_message}")
			if choose("[a]pprove, [r]eject: ", "ar") == "r":
				return

		for plugin in reviewed_plugins:
			plugin.accept_repo()
	finally:
		cleanup_plugins(plugins)

	if publish:
		commit_paths = get_repo_commit_paths(reviewed_plugins)

		# Update manifest cache
		info("Updating manifest cache")
		manifest_cache.refresh_cache(reviewed_plugins)

		# Update ModularGrid database
		while True:
			try:
				info("Updating ModularGrid database")
				modulargrid.update(reviewed_plugins)
				break
			except Exception as exception:
				warn(exception)
				if choose("[r]etry, [i]gnore: ", "ri") == "i":
					break

		# Delete plugin's old screenshots
		for plugin in reviewed_plugins:
			screenshots_dir = os.path.join(config.RACK_SCREENSHOTS_DIR, plugin.get_slug())
			try:
				shutil.rmtree(screenshots_dir)
			except FileNotFoundError:
				pass
		info("Generating screenshots with Rack")
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

		# Copy manifests to library
		for plugin in reviewed_plugins:
			path = os.path.join(config.MANIFESTS_DIR, f"{plugin.get_slug()}.json")
			with open(path, "w", encoding="utf-8") as f:
				json.dump(plugin.manifest, f, indent="  ")
			commit_paths.add(path)

		commit_paths.add(config.MANIFESTS_CACHE_FILE)
		commit_paths.add(config.MODULARGRID_FILE)

	update_message = ", ".join(f"{plugin.get_slug()} {plugin.get_version()}" for plugin in reviewed_plugins)
	commit_message = f"Update manifest {update_message}" if publish else f"Update {update_message}"
	commit_library(commit_paths, commit_message)
	info("Pushing library repo")
	run_retry("git", "push")

	if publish:
		for plugin in reviewed_plugins:
			if not plugin.is_source_in_library():
				continue
			# Open browser to plugin's GitHub library issue
			url = f"https://github.com/VCVRack/library/issues?q=is%3Aissue+sort%3Aupdated-desc+in%3Atitle+{plugin.get_slug()}"
			webbrowser.open(url)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-p", "--publish", action="store_true", help="Review plugins, then publish packages, screenshots, and library changes.")
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
		sys.exit()
	review(args.paths, args.commit, args.repo_url, publish=args.publish)
