#!/usr/bin/env python3
"""
Pre-flight verification and automation tool for submitting nur.h to:
- Microsoft vcpkg (microsoft/vcpkg)
- Conan Center Index (conan-io/conan-center-index)
- Meson WrapDB (mesonbuild/wrapdb)
"""

import sys
import json
import re
from pathlib import Path

ROOT = Path(__file__).parent.parent

def check_versions():
    print("[1] Verifying Version Synchronization...")
    version_file = (ROOT / "VERSION").read_text(encoding="utf-8").strip()

    vcpkg_json = json.loads((ROOT / "vcpkg.json").read_text(encoding="utf-8"))
    vcpkg_version = vcpkg_json.get("version") or vcpkg_json.get("version-string")

    port_vcpkg_json = json.loads((ROOT / "ports" / "nur" / "vcpkg.json").read_text(encoding="utf-8"))
    port_version = port_vcpkg_json.get("version")

    conan_content = (ROOT / "conanfile.py").read_text(encoding="utf-8")
    conan_match = re.search(r'version\s*=\s*"([^"]+)"', conan_content)
    conan_version = conan_match.group(1) if conan_match else None

    cmake_content = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    cmake_match = re.search(r'project\(nur\s+VERSION\s+([0-9.]+)', cmake_content)
    cmake_version = cmake_match.group(1) if cmake_match else None

    meson_content = (ROOT / "meson.build").read_text(encoding="utf-8")
    meson_match = re.search(r"version\s*:\s*'([^']+)'", meson_content)
    meson_version = meson_match.group(1) if meson_match else None

    print(f"  VERSION file:    {version_file}")
    print(f"  vcpkg.json:      {vcpkg_version}")
    print(f"  ports/nur:       {port_version}")
    print(f"  conanfile.py:    {conan_version}")
    print(f"  CMakeLists.txt:  {cmake_version}")
    print(f"  meson.build:     {meson_version}")

    versions = [version_file, vcpkg_version, port_version, conan_version, cmake_version, meson_version]
    if len(set(versions)) != 1:
        print("[!] ERROR: Version mismatch detected!", file=sys.stderr)
        sys.exit(1)
    print("  [OK] All package files are strictly synchronized to version", version_file)
    return version_file

def print_pr_commands(version):
    print("\n" + "="*70)
    print("PACKAGE REGISTRY SUBMISSION INSTRUCTIONS & COMMANDS")
    print("="*70)

    print(f"""
--- A. MICROSOFT VCPKG (microsoft/vcpkg) ---
1. Fork https://github.com/microsoft/vcpkg
2. Clone and prepare branch:
   git clone https://github.com/<your-username>/vcpkg.git
   cd vcpkg
   git checkout -b add-port-nur-{version}

3. Copy port directory:
   cp -r {ROOT / "ports" / "nur"} ports/nur

4. Add version and format:
   ./vcpkg format-manifest ports/nur/vcpkg.json
   ./vcpkg x-add-version nur

5. Open PR via GitHub CLI:
   git add ports/nur versions/
   git commit -m "[nur] Add port version {version}"
   git push origin add-port-nur-{version}
   gh pr create --repo microsoft/vcpkg --title "[nur] Add port version {version}" --body "Add nur.h zero-allocation Result and Option library."

--- B. CONAN CENTER (conan-io/conan-center-index) ---
1. Fork https://github.com/conan-io/conan-center-index
2. Clone and prepare branch:
   git clone https://github.com/<your-username>/conan-center-index.git
   cd conan-center-index
   git checkout -b add-nur-{version}

3. Create recipe directory:
   mkdir -p recipes/nur/all
   cp {ROOT / "conanfile.py"} recipes/nur/all/conanfile.py

4. Verify local package build:
   conan create recipes/nur/all/ --version={version}

5. Open PR:
   gh pr create --repo conan-io/conan-center-index --title "[nur] Add version {version}"

--- C. MESON WRAPDB (mesonbuild/wrapdb) ---
1. Fork https://github.com/mesonbuild/wrapdb
2. Clone and add wrap file:
   cp {ROOT / "subprojects" / "nur.wrap"} subprojects/nur.wrap
3. Open PR:
   gh pr create --repo mesonbuild/wrapdb --title "nur: new wrap for {version}"
""")

if __name__ == "__main__":
    v = check_versions()
    print_pr_commands(v)
