import os
import platform


RACK_VERSION_MAJOR = "2"
RACK_PLATFORM = {"Linux": "lin", "Darwin": "mac", "Windows": "win"}[platform.system()]
RACK_CPU = {"x86_64": "x64", "amd64": "x64", "arm64": "arm64", "aarch64": "arm64"}[platform.machine().lower()]
RACK_ARCHITECTURE = None
ARCHITECTURES = (
	"mac-arm64",
	"mac-x64",
	"win-x64",
	"lin-x64",
)
PLUGIN_BINARY_FILENAMES = {
	"mac": "plugin.dylib",
	"win": "plugin.dll",
	"lin": "plugin.so",
}

LARGE_FILE_SIZE = 1024 * 1024
BINARY_EXTENSIONS = {".dll", ".so", ".dylib", ".exe", ".elf", ".a", ".lib", ".o", ".obj", ".zip", ".tar", ".gz", ".tgz", ".bz2", ".xz", ".7z", ".rar", ".zst", ".vcvplugin"}
JUNK_ENTRIES = {".DS_Store", "Thumbs.db", ".env", ".git", ".vscode", ".idea", ".vs", ".eclipse", ".settings", "node_modules", "__pycache__", ".vagrant", "build", "dist", "dep"}

REPOS_DIR = "repos"
MANIFESTS_DIR = "manifests"
TOOLCHAIN_DIR = "../toolchain-v2"
PACKAGES_DIR = "../packages"
RACK_SYSTEM_DIR = "../Rack2"
RACK_USER_DIR = None
RACK_PLUGIN_DIR = None
RACK_SCREENSHOTS_DIR = None
SCREENSHOTS_DIR = "../screenshots"
MANIFESTS_CACHE_FILE = "manifests-cache.json"
MODULARGRID_FILE = "ModularGrid-VCVLibrary.json"
HTTP_TIMEOUT = 15
HTTP_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/154.0.0.0 Safari/537.36"

# Apply optional local configuration overrides
try:
	from .config_overrides import *
except ModuleNotFoundError:
	pass

if RACK_ARCHITECTURE is None:
	RACK_ARCHITECTURE = f"{RACK_PLATFORM}-{RACK_CPU}"
if RACK_USER_DIR is None:
	if RACK_PLATFORM == "mac":
		RACK_USER_DIR = os.path.expanduser("~/Library/Application Support/Rack2")
	elif RACK_PLATFORM == "win":
		RACK_USER_DIR = os.path.join(os.environ["LOCALAPPDATA"], "Rack2")
	elif RACK_PLATFORM == "lin":
		RACK_USER_DIR = os.path.expanduser("~/.local/share/Rack2")
if RACK_PLUGIN_DIR is None:
	RACK_PLUGIN_DIR = os.path.join(RACK_USER_DIR, f"plugins-{RACK_ARCHITECTURE}")
if RACK_SCREENSHOTS_DIR is None:
	RACK_SCREENSHOTS_DIR = os.path.join(RACK_USER_DIR, "screenshots")
