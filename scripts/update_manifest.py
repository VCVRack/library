#!/usr/bin/env python3

#
# Update plugin manifest.json from information contained in repository submodule
#

import os
import sys
import re
import json

# Community repository root
REPO_ROOT = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")

def get_plugin_version(plugin_path):
    with open(os.path.join(plugin_path, "Makefile"), "r") as f:
        version = [line for line in f.readlines() if "VERSION" in line][0]
        return version.split("=")[1].split()[0].strip() # Ignore comments on line

def write_plugin_version(manifest_path, version):
    # Use OrderedDict to preserve key order in json file
    from collections import OrderedDict
    mj = None
    with open(manifest_path, "r") as f:
        mj = json.loads(f.read(), object_pairs_hook=OrderedDict)

    with open(manifest_path, "w") as f:
        mj["latestVersion"] = version
        json.dump(mj, f, indent=2)

def validate_version(version):
    # Valid Rack plugin version has three digits, e.g. 0.6.0
    return re.match(r"^[0-9]\.[0-9]\.[0-9]$", version)


def main():
    errors = False
    for repo in os.listdir(os.path.join(REPO_ROOT, "repos")):
        repo_path = os.path.join(REPO_ROOT, "repos", repo)
        plugin_version = get_plugin_version(repo_path)
        if (validate_version(plugin_version)):
            manifest_path = os.path.join(REPO_ROOT, "manifests", repo+".json")
            if os.path.exists(manifest_path):
                write_plugin_version(manifest_path, plugin_version)
            else:
                print("[%s] Manifest does not exist" % repo)
                errors = True
                continue
        else:
            print("[%s] Invalid version: %s" % (repo, plugin_version))
            errors = True
            continue

    return 0 if not errors else 1

if __name__ == "__main__":
    sys.exit(main())
