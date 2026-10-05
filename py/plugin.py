import atexit
import json
import os
import re
import shutil
import tempfile
from urllib.parse import urlsplit

import requests

from . import config
from .command import run
from .console import info


EXTRACTION_DIR = tempfile.mkdtemp()
atexit.register(shutil.rmtree, EXTRACTION_DIR)


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
		return f"URL {url!r} is not a valid HTTP or HTTPS URL."
	headers = {"User-Agent": config.HTTP_USER_AGENT}
	try:
		with requests.get(url, headers=headers, timeout=config.HTTP_TIMEOUT, stream=True) as response:
			response.raise_for_status()
	except requests.RequestException as error:
		return f"URL {url!r} could not be reached. {error}"
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
	manifest = None
	library_manifest = None

	def __init__(self):
		self.package_paths = []

	def set_source_dir(self, directory):
		self.source_dir = directory

	def add_package_path(self, path):
		self.package_paths.append(path)

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

		# Extract packages to temporary directory.
		for package_path in self.package_paths:
			package_extraction_dir = os.path.join(EXTRACTION_DIR, os.path.basename(package_path))
			if os.path.isdir(package_extraction_dir):
				shutil.rmtree(package_extraction_dir)
			os.mkdir(package_extraction_dir)
			run("tar", "--zstd", "-xf", os.path.abspath(package_path), "-C", package_extraction_dir, capture_stderr=True)

	def load_manifest(self):
		# Load manifest from source directory or extracted packages.
		if self.source_dir:
			with open(os.path.join(self.source_dir, "plugin.json"), "r", encoding="utf-8") as f:
				self.manifest = json.load(f)
		elif self.package_paths:
			for package_path in self.package_paths:
				directory = os.path.join(EXTRACTION_DIR, os.path.basename(package_path))
				entries = list(os.scandir(directory))
				if len(entries) != 1 or not entries[0].is_dir(follow_symlinks=False):
					raise ValueError(f"Extracted archive {directory!r} does not contain a sole plugin directory: {[entry.name for entry in entries]!r}")
				with open(os.path.join(entries[0].path, "plugin.json"), encoding="utf-8") as f:
					package_manifest = json.load(f)
				if self.manifest is None:
					self.manifest = package_manifest
				elif package_manifest != self.manifest:
					raise ValueError(f"Package {package_path!r} manifest does not match manifest from other packages.")
		else:
			raise RuntimeError("No source directory or package paths set.")

		# Check slug and version fields.
		for key in ("slug", "version"):
			if key not in self.manifest:
				raise ValueError(f"Manifest is missing required field {key!r}.")
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
		for key in ("name", "license", "author"):
			if key not in manifest:
				raise ValueError(f"Manifest is missing required field {key!r}.")

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
			for key in ("slug", "name"):
				if key not in module:
					raise ValueError(f"{module_label} is missing required field {key!r}.")

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
			return None

		# Check that manifest and library manifest have the same slug.
		manifest = self.manifest
		library_manifest = self.library_manifest
		if manifest["slug"] != library_manifest["slug"]:
			warnings.append(f"Plugin slug changed from {library_manifest['slug']!r} to {manifest['slug']!r}.")

		module_slugs = {module["slug"] for module in manifest.get("modules", [])}
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
		for path in paths:
			# Find outermost junk entry in tracked path.
			filename = os.path.basename(path)
			junk_path = path if filename in config.JUNK_ENTRIES else None
			directory = path
			while directory := os.path.dirname(directory):
				if os.path.basename(directory) in config.JUNK_ENTRIES:
					junk_path = directory
			if junk_path:
				if junk_path not in junk_paths:
					warnings.append(f"Git tracks junk entry {os.path.join(self.source_dir, junk_path)!r}.")
					junk_paths.add(junk_path)

			# Check file extension.
			abs_path = os.path.abspath(os.path.join(self.source_dir, path))
			extension = os.path.splitext(filename)[1].lower()
			if extension in config.BINARY_EXTENSIONS:
				warnings.append(f"Git tracks binary file {abs_path!r}.")

			# Check file size.
			if not os.path.isfile(abs_path):
				continue
			size = os.path.getsize(abs_path)
			if size > config.LARGE_FILE_SIZE:
				warnings.append(f"Git tracks large file {abs_path!r} containing {size} bytes, exceeding {config.LARGE_FILE_SIZE} bytes.")
		return '\n'.join(warnings) or None

	def review_source(self):
		if not self.source_dir:
			return None
		path = os.path.join(self.source_dir, "Makefile")
		if not os.path.isfile(path):
			raise FileNotFoundError(f"{path!r} is missing.")

	def review_source_with_cppcheck(self):
		if not self.source_dir:
			return None
		info(f"Reviewing {self.get_slug()} {self.get_version()} source with Cppcheck")
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
			dist_dir = os.path.join(EXTRACTION_DIR, filename)
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

	def get_package_timestamp(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")

		# Read package modification time.
		version = self.get_version()
		architecture = config.RACK_ARCHITECTURE
		slug = self.get_slug()
		filenames = [f"{slug}-{version}-{architecture}.vcvplugin"]
		if architecture.endswith("-x64"):
			filenames.append(f"{slug}-{version}-{architecture.removesuffix('-x64')}.vcvplugin")
		for filename in filenames:
			try:
				return os.path.getmtime(os.path.join(config.PACKAGES_DIR, filename))
			except FileNotFoundError:
				pass
		return None

	def get_creation_timestamp(self):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")

		# Read timestamp of the first commit containing the manifest.
		manifest_path = os.path.join(config.MANIFESTS_DIR, f"{self.get_slug()}.json")
		timestamps = run("git", "log", "--format=%ct", "--", manifest_path, capture_stdout=True).splitlines()
		return float(timestamps[-1]) if timestamps else None

	def get_module_creation_timestamps(self, module_slugs):
		if self.manifest is None:
			raise RuntimeError("No manifest loaded.")
		timestamps = dict.fromkeys(module_slugs)
		if not timestamps:
			return timestamps

		# Find when each module first appeared in the manifest.
		manifest_path = os.path.join(config.MANIFESTS_DIR, f"{self.get_slug()}.json")
		history = run("git", "log", "--format=%H %ct", "--", manifest_path, capture_stdout=True)
		for line in history.splitlines():
			commit, timestamp = line.split()
			if not run("git", "ls-tree", "--name-only", commit, "--", manifest_path, capture_stdout=True).strip():
				continue
			manifest = json.loads(run("git", "show", f"{commit}:{manifest_path}", capture_stdout=True))
			for module in manifest.get("modules", []):
				if module["slug"] in timestamps:
					timestamps[module["slug"]] = float(timestamp)
		return timestamps
