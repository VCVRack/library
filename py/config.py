import os


RACK_VERSION_MAJOR = "2"
RACK_ARCHITECTURE = "lin-x64"
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
BINARY_EXTENSIONS = {".dll", ".so", ".dylib", ".exe", ".a", ".lib", ".o", ".obj", ".zip", ".tar", ".gz", ".tgz", ".bz2", ".xz", ".7z", ".rar", ".zst", ".vcvplugin"}
JUNK_ENTRIES = {".DS_Store", "Thumbs.db", ".env", ".git", ".vscode", ".idea", ".vs", ".eclipse", ".settings", "node_modules", "__pycache__", ".vagrant", "build", "dist", "dep"}

REPOS_DIR = "repos"
MANIFESTS_DIR = "manifests"
TOOLCHAIN_DIR = "../toolchain-v2"
PACKAGES_DIR = "../packages"
RACK_SYSTEM_DIR = "../Rack2"
RACK_USER_DIR = os.path.expanduser("~/.local/share/Rack2")
RACK_PLUGIN_DIR = os.path.join(RACK_USER_DIR, f"plugins-{RACK_ARCHITECTURE}")
RACK_SCREENSHOTS_DIR = os.path.join(RACK_USER_DIR, "screenshots")
SCREENSHOTS_DIR = "../screenshots"
MANIFESTS_CACHE_FILE = "manifests-cache.json"
MODULARGRID_FILE = "ModularGrid-VCVLibrary.json"
HTTP_TIMEOUT = 15
HTTP_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/154.0.0.0 Safari/537.36"
