import atexit
import glob
import json
import os
import re
import shlex
import shutil
import tempfile
from urllib.parse import urldefrag, urlsplit

import requests

from . import config
from .common import info, run, warn


EXTRACTION_DIR = tempfile.mkdtemp()
atexit.register(shutil.rmtree, EXTRACTION_DIR)

REACHABLE_URLS = set()


with open(os.path.join(os.path.dirname(__file__), "spdx.json"), encoding="utf-8") as f:
	SPDX_IDS = set(json.load(f))

with open(os.path.join(os.path.dirname(__file__), "tags.json"), encoding="utf-8") as f:
	VALID_TAGS = {tag.lower() for group in json.load(f) for tag in group}


def is_valid_slug(slug):
	return bool(re.fullmatch(r"[A-Za-z0-9](?:[A-Za-z0-9_-]*[A-Za-z0-9])?", slug))


def is_valid_url(url):
	if any(character.isspace() for character in url):
		return False
	try:
		parts = urlsplit(url)
		return parts.scheme in ("http", "https") and bool(parts.hostname)
	except ValueError:
		return False


def review_url(url):
	if not is_valid_url(url):
		return f"URL `{url}` is not a valid HTTP or HTTPS URL."
	request_url = urldefrag(url).url
	if request_url in REACHABLE_URLS:
		return None
	headers = {"User-Agent": config.HTTP_USER_AGENT}
	try:
		with requests.get(request_url, headers=headers, timeout=config.HTTP_TIMEOUT, stream=True) as response:
			response.raise_for_status()
	except requests.RequestException as error:
		return f"URL `{url}` could not be reached. {error}"
	REACHABLE_URLS.add(request_url)
	return None


def version_key(version):
	parts = []
	for part in version.split('.'):
		try:
			parts.append((1, int(part)))
		except ValueError:
			parts.append((0, part))
	return parts


class Plugin:
	source_dir = None
	repo_url = None
	repo_commit = None
	previous_repo_commit = None
	previous_repo_url = None
	created_repo = False
	extraction_dir = None
	manifest = None
	library_manifest = None

	def __init__(self):
		self.package_paths = []

	def set_source_dir(self, directory):
		self.source_dir = directory

	def is_source_in_library(self):
		"""Returns whether the source directory is inside config.REPOS_DIR."""
		if not self.source_dir:
			return False
		source_dir = os.path.abspath(self.source_dir)
		repos_dir = os.path.abspath(config.REPOS_DIR)
		return os.path.commonpath([repos_dir, source_dir]) == repos_dir

	def add_package_path(self, path):
		self.package_paths.append(path)

	def checkout_repo(self):
		"""Sets the repository URL, checks out the requested commit, and initializes nested submodules."""
		if not self.repo_url and not self.repo_commit:
			return

		if os.path.isdir(self.source_dir):
			changes = run("git", "status", "--porcelain", cwd=self.source_dir, capture_stdout=True)
			if changes:
				raise RuntimeError(f"Repository {self.source_dir!r} has uncommitted changes.\n\n{changes}")

			# If commit hash or tag is requested, try to get the currently checked out commit hash.
			if self.repo_commit and not self.previous_repo_commit and not self.created_repo:
				try:
					self.previous_repo_commit = run("git", "rev-parse", "HEAD", cwd=self.source_dir, capture_stdout=True).strip()
				except RuntimeError:
					pass

			# If URL change is requested, record its original URL so we can revert later.
			if self.repo_url and not self.previous_repo_url and not self.created_repo:
				if self.is_source_in_library():
					self.previous_repo_url = run("git", "config", "--file", ".gitmodules", "--get", f"submodule.{self.source_dir}.url", capture_stdout=True).strip()
				else:
					self.previous_repo_url = run("git", "remote", "get-url", "origin", cwd=self.source_dir, capture_stdout=True).strip()

		# Set repository URL or create repository
		if self.repo_url:
			if self.is_source_in_library():
				# Repository is inside library, so change its submodule URL or add it as a submodule.
				if os.path.isdir(self.source_dir):
					run("git", "submodule", "set-url", "--", self.source_dir, self.repo_url, capture_stderr=True)
				else:
					run("git", "submodule", "add", "--", self.repo_url, self.source_dir, capture_stderr=True)
					self.created_repo = True
			else:
				# Repository is outside library, so change its remote URL or clone it.
				if os.path.isdir(self.source_dir):
					run("git", "remote", "set-url", "origin", self.repo_url, cwd=self.source_dir, capture_stderr=True)
				else:
					run("git", "clone", "--", self.repo_url, self.source_dir, capture_stderr=True)
					self.created_repo = True

		# Check out requested commit hash
		if self.repo_commit:
			info(f"Checking out {self.source_dir} at {self.repo_commit}")
			run("git", "fetch", "--", "origin", cwd=self.source_dir, capture_stderr=True)
			run("git", "checkout", "--detach", self.repo_commit, "--", cwd=self.source_dir, capture_stderr=True)

		# Initialize submodules
		run("git", "submodule", "update", "--init", "--recursive", cwd=self.source_dir, capture_stderr=True)

	def accept_repo(self):
		"""Discards saved information for undoing repository changes."""
		self.previous_repo_commit = None
		self.previous_repo_url = None
		self.created_repo = False

	def undo_repo(self):
		"""Undoes repository changes made for this plugin."""
		# If repository was created, remove it.
		if self.created_repo:
			info(f"Removing {self.source_dir}")
			if self.is_source_in_library():
				git_dir = run("git", "rev-parse", "--absolute-git-dir", cwd=self.source_dir, capture_stdout=True).strip()
				run("git", "submodule", "deinit", "-f", "--", self.source_dir, capture_stderr=True)
				run("git", "rm", "-f", "--", self.source_dir, capture_stderr=True)
				shutil.rmtree(git_dir)
			else:
				shutil.rmtree(self.source_dir)
			self.accept_repo()
			return

		# If original commit hash is recorded, restore it.
		if self.previous_repo_commit:
			info(f"Restoring {self.source_dir} to {self.previous_repo_commit}")
			run("git", "checkout", "--detach", self.previous_repo_commit, cwd=self.source_dir, capture_stderr=True)
			run("git", "submodule", "update", "--init", "--recursive", cwd=self.source_dir, capture_stderr=True)

		# If original URL is recorded, restore it.
		if self.previous_repo_url:
			info(f"Restoring {self.source_dir} repository URL")
			if self.is_source_in_library():
				run("git", "submodule", "set-url", "--", self.source_dir, self.previous_repo_url, capture_stderr=True)
			else:
				run("git", "remote", "set-url", "origin", self.previous_repo_url, cwd=self.source_dir, capture_stderr=True)

		self.accept_repo()

	def get_slug(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		slug = self.manifest.get('slug')
		if not slug:
			raise ValueError("`plugin.json` has no `slug` value.")
		return slug

	def get_version(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		version = self.manifest.get('version')
		if not version:
			raise ValueError("`plugin.json` has no `version` value.")
		return version

	def get_package_path(self, architecture):
		"""Returns the package path for the architecture, or None if unavailable."""
		filename = f"{self.get_slug()}-{self.get_version()}-{architecture}.vcvplugin"
		return next((path for path in self.package_paths if os.path.basename(path) == filename), None)

	def is_open_source(self):
		"""Returns whether the manifest license is a recognized SPDX identifier."""
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		return self.manifest["license"] in SPDX_IDS

	def is_new_version(self):
		"""Returns whether this version is newer than the library version or the plugin is new to the library."""
		version = self.get_version()
		if self.library_manifest is None:
			return True
		return version_key(version) > version_key(self.library_manifest['version'])

	def extract_package(self, package_path):
		"""Extracts the package if needed and returns the directory containing its contents."""
		if not self.extraction_dir:
			self.extraction_dir = tempfile.mkdtemp(dir=EXTRACTION_DIR)

		filename = os.path.basename(package_path)
		basename = os.path.splitext(filename)[0]
		dist_dir = os.path.join(self.extraction_dir, basename)
		if os.path.isdir(dist_dir):
			return dist_dir

		os.mkdir(dist_dir)
		run("tar", "--zstd", "-xf", os.path.abspath(package_path), "-C", dist_dir, capture_stderr=True)
		return dist_dir

	def load_manifest(self):
		# Load manifest from source directory or extracted packages.
		if self.source_dir:
			with open(os.path.join(self.source_dir, "plugin.json"), "r", encoding="utf-8") as f:
				self.manifest = json.load(f)
		elif self.package_paths:
			for package_path in self.package_paths:
				dist_dir = self.extract_package(package_path)
				manifest_paths = glob.glob(f"{dist_dir}/*/plugin.json")
				if not manifest_paths:
					raise FileNotFoundError(f"Package `{os.path.basename(package_path)}` has no `plugin.json` file.")
				with open(manifest_paths[0], encoding="utf-8") as f:
					package_manifest = json.load(f)
				if self.manifest is None:
					self.manifest = package_manifest
				elif package_manifest != self.manifest:
					raise ValueError(f"Package `{os.path.basename(package_path)}` has a `plugin.json` file that differs from the other packages.")
		else:
			raise RuntimeError("No source directory or package paths set.")

		# Check slug and version fields.
		slug = self.get_slug()
		if not is_valid_slug(slug):
			raise ValueError(f"`plugin.json` has an invalid `slug` value `{slug}`.")
		self.get_version()

	def load_library_manifest(self):
		self.library_manifest = None
		path = os.path.join(config.MANIFESTS_DIR, f"{self.get_slug()}.json")
		try:
			with open(path, "r", encoding="utf-8") as f:
				self.library_manifest = json.load(f)
		except FileNotFoundError:
			pass

	def review_manifest(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")

		# Check required plugin fields.
		manifest = self.manifest
		for key in ("name", "license", "author"):
			if not manifest.get(key):
				raise ValueError(f"`plugin.json` has no `{key}` value.")

		plugin_slug = self.get_slug()
		warnings = []

		# Check plugin major version.
		version_parts = self.get_version().split('.')
		if version_parts[0] != config.RACK_VERSION_MAJOR:
			warnings.append(f"Plugin major version is `{version_parts[0]}` but must be `{config.RACK_VERSION_MAJOR}`.")

		# Check license.
		license = manifest["license"]
		if license.lower() != "proprietary" and license not in SPDX_IDS and not is_valid_url(license):
			warnings.append(f"`plugin.json` has an invalid `license` value `{license}`. See https://spdx.org/licenses/ for valid license IDs.")

		# Check module fields.
		module_slugs = set()
		for module in manifest.get("modules", []):
			slug = module.get("slug")
			if not slug:
				raise ValueError("A module is missing its `slug` property.")
			if not is_valid_slug(slug):
				raise ValueError(f"Module `{slug}` has an invalid slug.")
			if slug in module_slugs:
				raise ValueError(f"Module `{slug}` has a duplicate slug.")
			module_slugs.add(slug)
			if not module.get("name"):
				raise ValueError(f"Module `{slug}` has no `name` value.")

		# Find new module slugs.
		library_module_slugs = set()
		if self.library_manifest:
			library_module_slugs = {module["slug"] for module in self.library_manifest.get("modules", [])}
		new_module_slugs = module_slugs - library_module_slugs

		# Check whether new module slugs contain the plugin slug.
		module_slugs_with_plugin_slug = {slug for slug in new_module_slugs if plugin_slug.casefold() in slug.casefold()}
		if module_slugs_with_plugin_slug:
			slugs_string = ", ".join(f"`{slug}`" for slug in sorted(module_slugs_with_plugin_slug))
			if len(module_slugs_with_plugin_slug) == 1:
				warnings.append(f"Module slug {slugs_string} contains plugin slug `{plugin_slug}`. Modules are already identified by the plugin they are in, so please remove `{plugin_slug}` from {slugs_string}.")
			else:
				warnings.append(f"Module slugs {slugs_string} contain plugin slug `{plugin_slug}`. Modules are already identified by the plugin they are in, so please remove `{plugin_slug}` from these module slugs.")

		# Check whether the plugin slug contains new module slugs.
		module_slugs_in_plugin_slug = {slug for slug in new_module_slugs - module_slugs_with_plugin_slug if slug.casefold() in plugin_slug.casefold()}
		if module_slugs_in_plugin_slug:
			slugs_string = ", ".join(f"`{slug}`" for slug in sorted(module_slugs_in_plugin_slug))
			if len(module_slugs_in_plugin_slug) == 1:
				warnings.append(f"Plugin slug `{plugin_slug}` contains module slug {slugs_string}. The plugin slug should name the collection of modules rather than one module, so please remove {slugs_string} from `{plugin_slug}`.")
			else:
				warnings.append(f"Plugin slug `{plugin_slug}` contains module slugs {slugs_string}. The plugin slug should name the collection of modules, so please remove these module slugs from `{plugin_slug}`.")

		# Check module tags.
		for module in manifest.get("modules", []):
			invalid_tags = sorted({tag for tag in module.get("tags", []) if tag.lower() not in VALID_TAGS})
			if not invalid_tags:
				continue
			tags_string = ", ".join(f"`{tag}`" for tag in invalid_tags)
			if len(invalid_tags) == 1:
				warnings.append(f"Module `{module['slug']}` has an invalid tag {tags_string}. See https://vcvrack.com/manual/Manifest#modules-tags for valid tags.")
			else:
				warnings.append(f"Module `{module['slug']}` has invalid tags {tags_string}. See https://vcvrack.com/manual/Manifest#modules-tags for valid tags.")

		# Check ModularGrid URLs for hardware clones.
		missing_modulargrid_url_slugs = set()
		for module in manifest.get("modules", []):
			if module.get("modularGridUrl"):
				continue
			if any(tag.lower() == "hardware clone" for tag in module.get("tags", [])):
				missing_modulargrid_url_slugs.add(module["slug"])
		if missing_modulargrid_url_slugs:
			slugs_string = ", ".join(f"`{slug}`" for slug in sorted(missing_modulargrid_url_slugs))
			if len(missing_modulargrid_url_slugs) == 1:
				warnings.append(f"Module {slugs_string} has a `Hardware clone` tag but no `modularGridUrl` property. This would link its VCV Library page to ModularGrid and vice versa.")
			else:
				warnings.append(f"Modules {slugs_string} have a `Hardware clone` tag but no `modularGridUrl` property. This would link their VCV Library pages to ModularGrid and vice versa.")

		# Check deprecated module properties.
		disabled_module_slugs = {module["slug"] for module in manifest.get("modules", []) if "disabled" in module}
		if disabled_module_slugs:
			slugs_string = ", ".join(f"`{slug}`" for slug in sorted(disabled_module_slugs))
			if len(disabled_module_slugs) == 1:
				warnings.append(f"Module {slugs_string} has a deprecated `disabled` property. Replace `disabled` with `hidden`.")
			else:
				warnings.append(f"Modules {slugs_string} have a deprecated `disabled` property. Replace `disabled` with `hidden`.")

		if warning := self.review_manifest_urls():
			warnings.append(warning)

		# Check for new plugins and changed module slugs
		library_manifest = self.library_manifest
		if library_manifest is None:
			warnings.append(f'Adding new plugin "{plugin_slug}" to library.')
		else:
			if plugin_slug != library_manifest["slug"]:
				warnings.append(f"Plugin slug changed from `{library_manifest['slug']}` to `{plugin_slug}`, which breaks existing patches that use this plugin. Restore the original slug `{library_manifest['slug']}`.")
			if new_module_slugs:
				new_module_slugs_string = ", ".join(f"`{slug}`" for slug in sorted(new_module_slugs))
				if len(new_module_slugs) == 1:
					warnings.append(f"Module {new_module_slugs_string} is new.")
				else:
					warnings.append(f"Modules {new_module_slugs_string} are new.")
			removed_module_slugs = library_module_slugs - module_slugs
			if removed_module_slugs:
				slugs_string = ", ".join(f"`{slug}`" for slug in sorted(removed_module_slugs))
				if len(removed_module_slugs) == 1:
					warnings.append(f"Module {slugs_string} has been removed, which breaks existing patches that use it. Restore the module with its original slug.")
				else:
					warnings.append(f"Modules {slugs_string} have been removed, which breaks existing patches that use them. Restore the modules with their original slugs.")
		return '\n'.join(warnings) or None

	def review_manifest_urls(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		warnings = []

		# If plugin is open source, check that it has a sourceUrl.
		if self.is_open_source():
			if not self.manifest.get("sourceUrl"):
				warnings.append("`plugin.json` has an open-source license but no `sourceUrl`. Add the public source repository URL.")
		else:
			if self.manifest.get("sourceUrl"):
				warnings.append("`plugin.json` has a `sourceUrl` but its license is not open-source. Remove the `sourceUrl` property.")

		# Check plugin URLs.
		for key in ("authorUrl", "pluginUrl", "manualUrl", "sourceUrl", "donateUrl", "changelogUrl"):
			if url := self.manifest.get(key):
				if warning := review_url(url):
					warnings.append(f"`plugin.json` property `{key}`: {warning}")

		# Check module URLs.
		for module in self.manifest.get('modules', []):
			for key in ("manualUrl", "modularGridUrl"):
				if url := module.get(key):
					if warning := review_url(url):
						warnings.append(f"Module `{module['slug']}` property `{key}`: {warning}")

		# If license is a URL, check it.
		license = self.manifest['license']
		if license.lower().startswith(("http://", "https://")):
			if warning := review_url(license):
				warnings.append(f"`plugin.json` property `license`: {warning}")

		return '\n'.join(warnings) or None

	def review_git(self):
		if not self.source_dir:
			return None
		try:
			output = run("git", "ls-files", "-z", "--", ".", cwd=self.source_dir, capture_stdout=True)
		except RuntimeError:
			return None
		warnings = []

		# Check for uncommitted changes.
		changes = run("git", "status", "--porcelain", cwd=self.source_dir, capture_stdout=True)
		if changes:
			warnings.append(f"Repository {self.source_dir!r} has uncommitted changes.\n{changes}")

		paths = [path for path in output.split('\0') if path]
		junk_paths = set()
		binary_paths = []
		large_files = []
		for path in paths:
			# Find outermost junk entry in tracked path.
			filename = os.path.basename(path)
			junk_path = path if filename in config.JUNK_ENTRIES else None
			directory = path
			while directory := os.path.dirname(directory):
				if os.path.basename(directory) in config.JUNK_ENTRIES:
					junk_path = directory
			if junk_path:
				junk_paths.add(junk_path)

			# Check file extension.
			abs_path = os.path.abspath(os.path.join(self.source_dir, path))
			extension = os.path.splitext(filename)[1].lower()
			if extension in config.BINARY_EXTENSIONS:
				binary_paths.append(path)

			# Check file size.
			if not os.path.isfile(abs_path):
				continue
			size = os.path.getsize(abs_path)
			if size > config.LARGE_FILE_SIZE:
				large_files.append({"name": path, "size": size})

		if junk_paths:
			junk_paths = sorted(junk_paths)
			junk_paths_command = shlex.join(["git", "rm", "-r", "--cached", "--", *junk_paths])
			if len(junk_paths) == 1:
				warnings.append(f"Junk file or directory is committed to Git. Consider adding it to `.gitignore`. Run this command to stage its removal without deleting it locally: `{junk_paths_command}`.")
			else:
				warnings.append(f"Junk files or directories are committed to Git. Consider adding them to `.gitignore`. Run this command to stage their removal without deleting them locally: `{junk_paths_command}`.")

		if binary_paths:
			binary_paths_command = shlex.join(["git", "rm", "--cached", "--ignore-unmatch", "--", *binary_paths])
			if len(binary_paths) == 1:
				warnings.append(f"Binary file is committed to Git. Consider adding it to `.gitignore`. Run this command to stage its removal without deleting it locally: `{binary_paths_command}`.")
			else:
				warnings.append(f"Binary files are committed to Git. Consider adding them to `.gitignore`. Run this command to stage their removal without deleting them locally: `{binary_paths_command}`.")

		if large_files:
			large_files.sort(key=lambda file: file["name"])
			large_files_string = ", ".join(f"`{file['name']}` ({file['size'] / 1024 ** 2:.1f} MiB)" for file in large_files)
			if len(large_files) == 1:
				warnings.append(f"Large file is committed to Git. Consider [removing it from Git history](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github#removing-files-from-a-repositorys-history) to reduce clone size. {large_files_string}")
			else:
				warnings.append(f"Large files are committed to Git. Consider [removing them from Git history](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github#removing-files-from-a-repositorys-history) to reduce clone size. {large_files_string}")
		return '\n'.join(warnings) or None

	def review_source(self):
		if not self.source_dir:
			return None
		path = os.path.join(self.source_dir, "Makefile")
		if not os.path.isfile(path):
			raise FileNotFoundError(f"{path!r} is missing.")

		# Check that source directory name matches plugin slug.
		source_dir = os.path.abspath(self.source_dir)
		directory_name = os.path.basename(source_dir)
		if directory_name != self.get_slug():
			return f"Source directory name {directory_name!r} does not match plugin slug {self.get_slug()!r}."
		return None

	def review_packages(self):
		if not self.package_paths:
			raise RuntimeError("No packages available.")
		slug = self.get_slug()
		version = self.get_version()
		filenames = {os.path.basename(path) for path in self.package_paths}
		warnings = []

		# Check that every architecture has a package for this slug and version.
		filenames_by_architecture = {architecture: f"{slug}-{version}-{architecture}.vcvplugin" for architecture in config.ARCHITECTURES}
		expected_filenames = set(filenames_by_architecture.values())
		missing_filenames = expected_filenames - filenames
		if missing_filenames:
			warnings.append(f"Missing packages: {', '.join(sorted(missing_filenames))}.")
		unexpected_filenames = filenames - expected_filenames
		if unexpected_filenames:
			raise ValueError(f"Unexpected packages: {', '.join(sorted(unexpected_filenames))}.")
		if len(filenames) != len(self.package_paths):
			raise ValueError("Multiple packages have the same filename.")

		# Extract all packages
		dist_dirs_by_architecture = {}
		for architecture in config.ARCHITECTURES:
			package_path = self.get_package_path(architecture)
			if not package_path:
				continue
			dist_dirs_by_architecture[architecture] = self.extract_package(package_path)

		# Review all extracted contents
		for architecture, dist_dir in dist_dirs_by_architecture.items():
			if warning := self.review_dist(dist_dir, architecture):
				warnings.append(warning)
		return '\n'.join(warnings) or None

	def review_dist(self, dist_dir, architecture):
		"""Reviews package's extracted contents."""
		warnings = []
		package_filename = f"{self.get_slug()}-{self.get_version()}-{architecture}.vcvplugin"

		# Check that dist contains one plugin directory matching manifest slug.
		slug = self.get_slug()
		entries = list(os.scandir(dist_dir))
		if not (len(entries) == 1 and entries[0].is_dir() and entries[0].name == slug):
			entries_string = ", ".join(f"`{entry.name}`" for entry in entries)
			warnings.append(f"Package `{package_filename}` must contain a single `{slug}` directory with all plugin files inside it. The root of the package contains {entries_string}.")

		# Check for platform's plugin binary.
		platform = architecture.split("-")[0]
		binary_filename = config.PLUGIN_BINARY_FILENAMES[platform]
		binary_path = os.path.join(dist_dir, slug, binary_filename)
		if not os.path.isfile(binary_path):
			warnings.append(f"Package `{package_filename}` is missing `{slug}/{binary_filename}`.")

		# Scan dist directory for junk entries.
		junk_paths = []
		def review_dir(directory):
			with os.scandir(directory) as entries:
				for entry in entries:
					if entry.name in config.JUNK_ENTRIES:
						junk_paths.append(os.path.relpath(entry.path, dist_dir))
						continue
					if entry.is_dir(follow_symlinks=False):
						review_dir(entry.path)

		review_dir(dist_dir)
		if junk_paths:
			junk_paths_string = ", ".join(f"`{path}`" for path in sorted(junk_paths))
			warnings.append(f"Update your `Makefile` to exclude junk files and directories from `{package_filename}`, then rebuild the package. The package includes {junk_paths_string}.")
		return '\n'.join(warnings) or None

	def build_source(self):
		"""Builds plugin source for all architectures and stores the packages in package_paths."""
		if not self.source_dir:
			raise RuntimeError("No source directory set.")

		# Build source for each architecture.
		source_dir = os.path.abspath(self.source_dir)
		slug = self.get_slug()
		version = self.get_version()
		for architecture in config.ARCHITECTURES:
			try:
				run("make", f"-j{os.cpu_count() or 1}", f"plugin-build-{architecture}", f"PLUGIN_DIR={source_dir}", cwd=config.TOOLCHAIN_DIR, capture_stderr=True)
			except Exception as exception:
				raise RuntimeError(f"Building for {architecture} failed.\n{exception}") from exception

		# Record the packages left in the toolchain output directory.
		package_paths = []
		for architecture in config.ARCHITECTURES:
			filename = f"{slug}-{version}-{architecture}.vcvplugin"
			package_path = os.path.join(config.TOOLCHAIN_DIR, "plugin-build", filename)
			if not os.path.isfile(package_path):
				raise FileNotFoundError(f"{package_path!r} is missing.")
			package_paths.append(package_path)
		self.package_paths = package_paths

	def install_package(self):
		"""Installs the native package and returns whether it was available."""
		if not self.package_paths:
			raise RuntimeError("No packages available.")
		# Install to Rack plugin directory
		package_path = self.get_package_path(config.RACK_ARCHITECTURE)
		if not package_path:
			warn(f"{self.get_slug()} {self.get_version()} has no {config.RACK_ARCHITECTURE} package, so plugin was not installed.")
			return False
		filename = os.path.basename(package_path)
		shutil.copyfile(package_path, os.path.join(config.RACK_PLUGIN_DIR, filename))
		return True

	def publish_packages(self):
		"""Copies all packages to server package directory."""
		if not self.package_paths:
			raise RuntimeError("No packages available.")
		# Copy packages into server package directory.
		for package_path in self.package_paths:
			filename = os.path.basename(package_path)
			shutil.copyfile(package_path, os.path.join(config.PACKAGES_DIR, filename))
