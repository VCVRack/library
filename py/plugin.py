import atexit
import json
import os
import re
import shlex
import shutil
import tempfile
from urllib.parse import urldefrag, urlsplit

import requests

from . import config
from .command import run
from .console import info


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
		if not self.repo_url and not self.repo_commit:
			return

		if self.repo_url:
			# Clone repository into a new directory
			if os.path.exists(self.source_dir):
				raise FileExistsError(f"Repository directory {self.source_dir!r} already exists.")
			info(f"Cloning {self.source_dir}")
			run("git", "clone", "--", self.repo_url, self.source_dir, capture_stderr=True)
		else:
			# Check for uncommitted changes
			changes = run("git", "status", "--porcelain", cwd=self.source_dir, capture_stdout=True)
			if changes:
				raise RuntimeError(f"Repository {self.source_dir!r} has uncommitted changes.\n\n{changes}")

		# Check out requested commit hash
		if self.repo_commit:
			info(f"Checking out {self.source_dir} at {self.repo_commit}")
			run("git", "fetch", "--", "origin", self.repo_commit, cwd=self.source_dir, capture_stderr=True)
			run("git", "checkout", "--detach", "FETCH_HEAD", cwd=self.source_dir, capture_stderr=True)

		# Initialize submodules
		run("git", "submodule", "update", "--init", "--recursive", cwd=self.source_dir, capture_stderr=True)

	def get_slug(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		return self.manifest['slug']

	def get_version(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		return self.manifest['version']

	def extract_packages(self):
		if not self.package_paths:
			return
		if not self.extraction_dir:
			self.extraction_dir = tempfile.mkdtemp(dir=EXTRACTION_DIR)

		# Extract packages to temporary directory.
		for package_path in self.package_paths:
			dist_dir = self.get_dist_dir(package_path)
			if os.path.isdir(dist_dir):
				shutil.rmtree(dist_dir)
			os.mkdir(dist_dir)
			run("tar", "--zstd", "-xf", os.path.abspath(package_path), "-C", dist_dir, capture_stderr=True)

	def get_dist_dir(self, package_path):
		filename = os.path.basename(package_path)
		basename = os.path.splitext(filename)[0]
		return os.path.join(self.extraction_dir, basename)

	def load_manifest(self):
		# Load manifest from source directory or extracted packages.
		if self.source_dir:
			with open(os.path.join(self.source_dir, "plugin.json"), "r", encoding="utf-8") as f:
				self.manifest = json.load(f)
		elif self.package_paths:
			for package_path in self.package_paths:
				dist_dir = self.get_dist_dir(package_path)
				entries = list(os.scandir(dist_dir))
				if len(entries) != 1 or not entries[0].is_dir(follow_symlinks=False):
					raise ValueError(f"Extracted archive {dist_dir!r} does not contain a sole plugin directory: {[entry.name for entry in entries]!r}")
				with open(os.path.join(entries[0].path, "plugin.json"), encoding="utf-8") as f:
					package_manifest = json.load(f)
				if self.manifest is None:
					self.manifest = package_manifest
				elif package_manifest != self.manifest:
					raise ValueError(f"Package {package_path!r} manifest does not match manifest from other packages.")
		else:
			raise RuntimeError("No source directory or package paths set.")

		# Check slug and version fields.
		required_keys = {"slug", "version"}
		missing_keys = required_keys - self.manifest.keys()
		if missing_keys:
			raise ValueError(f"Manifest is missing required fields: {', '.join(sorted(missing_keys))}.")
		if not is_valid_slug(self.get_slug()):
			raise ValueError(f"Plugin slug {self.get_slug()!r} is invalid.")

	def load_library_manifest(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
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
		required_keys = {"name", "license", "author"}
		missing_keys = required_keys - manifest.keys()
		if missing_keys:
			raise ValueError(f"Manifest is missing required fields: {', '.join(sorted(missing_keys))}.")

		plugin_slug = self.get_slug()
		warnings = []

		# Check plugin major version.
		version_parts = self.get_version().split('.')
		if version_parts[0] != config.RACK_VERSION_MAJOR:
			warnings.append(f"Plugin {self.get_slug()!r} version must start with {config.RACK_VERSION_MAJOR}.")

		# Check license.
		license = manifest["license"]
		if license.lower() != "proprietary" and license not in SPDX_IDS and not is_valid_url(license):
			warnings.append(f"Plugin {plugin_slug!r} has an unrecognized license: {license!r}")

		# Check plugin description.
		if description := manifest.get("description"):
			if '\n' in description or '\r' in description:
				warnings.append(f"Plugin {plugin_slug!r} description contains a newline: {description!r}")

		# Check source URL suffix.
		if source_url := manifest.get("sourceUrl"):
			if source_url.rstrip('/').endswith('.git'):
				warnings.append(f"Plugin {plugin_slug!r} sourceUrl ends with '.git': {source_url!r}")

		# Collect module slugs already in library.
		library_module_slugs = set()
		if self.library_manifest:
			library_module_slugs = {module["slug"] for module in self.library_manifest.get("modules", [])}

		# Check module fields.
		module_slugs = set()
		for index, module in enumerate(manifest.get("modules", [])):
			module_label = f"Plugin {plugin_slug!r} module {module['slug']!r}" if "slug" in module else f"Plugin {plugin_slug!r} module at index {index}"
			required_keys = {"slug", "name"}
			missing_keys = required_keys - module.keys()
			if missing_keys:
				raise ValueError(f"{module_label} is missing required fields: {', '.join(sorted(missing_keys))}.")

			slug = module["slug"]
			if not is_valid_slug(slug):
				raise ValueError(f"{module_label} has an invalid slug.")
			if slug in module_slugs:
				raise ValueError(f"{module_label} slug appears more than once in the manifest.")
			module_slugs.add(slug)

			# Check whether the module slug contains the plugin slug or vice versa.
			if slug not in library_module_slugs:
				if plugin_slug.casefold() in slug.casefold():
					warnings.append(f"{module_label} slug contains the plugin slug.")
				elif slug.casefold() in plugin_slug.casefold():
					warnings.append(f"Plugin slug {plugin_slug!r} contains module slug {slug!r}.")

			for tag in module.get("tags", []):
				if tag.lower() not in VALID_TAGS:
					warnings.append(f"{module_label} has an unrecognized Rack tag: {tag!r}")
				if tag.lower() == "hardware clone" and not module.get("modularGridUrl"):
					warnings.append(f"{module_label} has the 'Hardware clone' tag but no modularGridUrl.")

			if description := module.get("description"):
				if '\n' in description or '\r' in description:
					warnings.append(f"{module_label} description contains a newline: {description!r}")

			if "disabled" in module:
				warnings.append(f"{module_label} uses obsolete 'disabled' instead of 'hidden'.")

		if warning := self.review_manifest_urls():
			warnings.append(warning)
		if warning := self.review_library_changes():
			warnings.append(warning)
		return '\n'.join(warnings) or None

	def review_manifest_urls(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		warnings = []

		# Check plugin URLs.
		for key in ("authorUrl", "pluginUrl", "manualUrl", "sourceUrl", "donateUrl", "changelogUrl"):
			if url := self.manifest.get(key):
				if warning := review_url(url):
					warnings.append(f"Plugin {self.get_slug()!r} {key}. {warning}")

		# Check module URLs.
		for module in self.manifest.get('modules', []):
			for key in ("manualUrl", "modularGridUrl"):
				if url := module.get(key):
					if warning := review_url(url):
						warnings.append(f"Plugin {self.get_slug()!r} module {module['slug']!r} {key}. {warning}")

		# If license is a URL, check it.
		license = self.manifest['license']
		if license.lower().startswith(("http://", "https://")):
			if warning := review_url(license):
				warnings.append(f"Plugin {self.get_slug()!r} license. {warning}")

		return '\n'.join(warnings) or None

	def review_library_changes(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		warnings = []
		if self.library_manifest is None:
			return f'Adding new plugin "{self.get_slug()}" to library.'

		# Check that manifest and library manifest have the same slug.
		manifest = self.manifest
		library_manifest = self.library_manifest
		if manifest["slug"] != library_manifest["slug"]:
			warnings.append(f"Plugin slug changed from {library_manifest['slug']!r} to {manifest['slug']!r}.")

		module_slugs = {module["slug"] for module in manifest.get("modules", [])}
		library_module_slugs = {module["slug"] for module in library_manifest.get("modules", [])}
		new_module_slugs = module_slugs - library_module_slugs
		if new_module_slugs:
			warnings.append(f"Plugin {self.get_slug()!r} adds modules: {', '.join(sorted(new_module_slugs))}.")
		for module in library_manifest.get("modules", []):
			if module["slug"] not in module_slugs:
				warnings.append(f"Plugin {self.get_slug()!r} removes library module {module['slug']!r}.")

		return '\n'.join(warnings) or None

	def review_git(self):
		if not self.source_dir:
			return None
		try:
			output = run("git", "ls-files", "-z", "--", ".", cwd=self.source_dir, capture_stdout=True)
		except RuntimeError:
			return None
		paths = [path for path in output.split('\0') if path]
		warnings = []
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
			junk_paths_string = ", ".join(f"`{path}`" for path in junk_paths)
			junk_paths_command = shlex.join(["git", "rm", "-r", "--cached", "--", *junk_paths])
			warnings.append(f"Junk files {junk_paths_string} are committed to Git. Run `{junk_paths_command}` to stage their removal without deleting them locally. Use `.gitignore` to ignore them.")

		if binary_paths:
			binary_paths_string = ", ".join(f"`{path}`" for path in binary_paths)
			binary_paths_command = shlex.join(["git", "rm", "--cached", "--ignore-unmatch", "--", *binary_paths])
			warnings.append(f"Binary files {binary_paths_string} are committed to Git. Run `{binary_paths_command}` to stage their removal without deleting them locally. Use `.gitignore` to ignore them.")

		if large_files:
			large_files.sort(key=lambda file: file["name"])
			large_files_string = ", ".join(f"`{file['name']}` ({file['size'] / 1024 ** 2:.1f} MiB)" for file in large_files)
			warnings.append(f"Large files {large_files_string} are committed to Git. Consider removing them from Git history. See https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github#removing-files-from-a-repositorys-history.")
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

	def review_source_with_cppcheck(self):
		if not self.source_dir:
			return None
		info(f"Reviewing {self.get_slug()} source with Cppcheck")
		try:
			run("make", "plugin-analyze", f"PLUGIN_DIR={os.path.abspath(self.source_dir)}", cwd=config.TOOLCHAIN_DIR, capture_stderr=True)
		except (RuntimeError, OSError) as error:
			return str(error)
		return None

	def review_packages(self):
		if not self.package_paths:
			return None
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		slug = self.get_slug()
		version = self.get_version()
		filenames = [os.path.basename(path) for path in self.package_paths]
		warnings = []
		# Check that every architecture has a package for this slug and version.
		for architecture in config.ARCHITECTURES:
			filename = f"{slug}-{version}-{architecture}.vcvplugin"
			if filename not in filenames:
				raise ValueError(f"Package {filename!r} is missing.")
			dist_dir = self.get_dist_dir(filename)
			if warning := self.review_dist(dist_dir, architecture):
				warnings.append(warning)
			filenames.remove(filename)
		if filenames:
			raise ValueError(f"Unexpected package filenames for {slug!r} {version!r}: {filenames!r}")
		return '\n'.join(warnings) or None

	def review_dist(self, dist_dir, architecture):
		# Check that dist contains one plugin directory matching manifest slug.
		slug = self.get_slug()
		entries = list(os.scandir(dist_dir))
		if len(entries) != 1:
			raise ValueError(f"Dist directory {dist_dir!r} contains {len(entries)} entries, but exactly one is required.")
		if not entries[0].is_dir(follow_symlinks=False):
			raise NotADirectoryError(f"Dist entry {entries[0].path!r} is not a directory.")
		if entries[0].name != slug:
			raise ValueError(f"Plugin directory {entries[0].name!r} in {dist_dir!r} does not match manifest slug {slug!r}.")

		# Check for platform's plugin binary.
		platform = architecture.split("-")[0]
		binary_filename = config.PLUGIN_BINARY_FILENAMES[platform]
		binary_path = os.path.join(dist_dir, slug, binary_filename)
		if not os.path.isfile(binary_path):
			raise FileNotFoundError(f"{binary_path!r} is missing.")

		# Scan dist directory for junk entries.
		warnings = []
		def review_dir(directory):
			with os.scandir(directory) as entries:
				for entry in entries:
					if entry.name in config.JUNK_ENTRIES:
						warnings.append(f"Dist directory contains junk entry {entry.path!r}.")
						continue
					if entry.is_dir(follow_symlinks=False):
						review_dir(entry.path)

		review_dir(dist_dir)
		return '\n'.join(warnings) or None

	def build_source(self):
		if not self.source_dir:
			raise RuntimeError("No source directory set.")
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")

		# Build source for each architecture.
		source_dir = os.path.abspath(self.source_dir)
		slug = self.get_slug()
		version = self.get_version()
		for architecture in config.ARCHITECTURES:
			run("make", f"-j{os.cpu_count() or 1}", f"plugin-build-{architecture}", f"PLUGIN_DIR={source_dir}", cwd=config.TOOLCHAIN_DIR, capture_stderr=True)

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
		if not self.package_paths:
			raise RuntimeError("No packages available.")
		# Install to Rack plugin directory
		package_path = next(path for path in self.package_paths if path.endswith(f"-{config.RACK_ARCHITECTURE}.vcvplugin"))
		filename = os.path.basename(package_path)
		shutil.copyfile(package_path, os.path.join(config.RACK_PLUGIN_DIR, filename))

	def publish_packages(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		if not self.package_paths:
			raise RuntimeError("No packages available.")
		# Copy packages into server package directory.
		for package_path in self.package_paths:
			filename = os.path.basename(package_path)
			shutil.copyfile(package_path, os.path.join(config.PACKAGES_DIR, filename))
