# Central Package Registry Submission Guide

This guide provides step-by-step instructions and templates for submitting `nur.h` (and `nur_core.h`) to the official package registries: **vcpkg**, **Conan Center**, and **Meson WrapDB**.

---

## 1. Microsoft vcpkg Registry Submission

**Target Repository**: [microsoft/vcpkg](https://github.com/microsoft/vcpkg)  
**Port Directory**: Ready in [ports/nur/](file:///ports/nur/)

### Submission Steps:
1. Fork and clone `microsoft/vcpkg`:
   ```bash
   git clone https://github.com/<your-username>/vcpkg.git
   cd vcpkg
   git checkout -b add-port-nur
   ```
2. Copy the port directory:
   ```bash
   cp -r /path/to/nur.h/ports/nur ports/nur
   ```
3. Update SHA512 in `ports/nur/portfile.cmake`:
   ```bash
   ./vcpkg install nur --overlay-ports=ports/nur
   # If SHA mismatch occurs, copy the reported actual SHA512 hash into ports/nur/portfile.cmake
   ```
4. Format port files and update baseline:
   ```bash
   ./vcpkg format-manifest ports/nur/vcpkg.json
   ./vcpkg x-add-version nur
   ```
5. Commit and submit PR:
   - Branch: `add-port-nur`
   - PR Title: `[nur] Add port version 1.4.0`
   - PR Body:
     ```markdown
     ### Port Details
     - Package Name: `nur`
     - Version: `1.4.0`
     - License: MIT
     - Description: Zero-allocation, header-only Result<T> and Option<T> error handling for ISO C89/C99/C11/C17/C23 and C++11.
     - Upstream Repository: https://github.com/IreneAl-Caligo/nur.h
     ```

---

## 2. Conan Center Index Submission

**Target Repository**: [conan-io/conan-center-index](https://github.com/conan-io/conan-center-index)  

### Submission Steps:
1. Fork and clone `conan-io/conan-center-index`:
   ```bash
   git clone https://github.com/<your-username>/conan-center-index.git
   cd conan-center-index
   git checkout -b add-nur-1.4.0
   ```
2. Create recipe folder `recipes/nur/all/`:
   - Copy [conanfile.py](file:///conanfile.py) to `recipes/nur/all/conanfile.py`
   - Create `recipes/nur/config.yml`:
     ```yaml
     versions:
       "1.4.0":
         folder: all
     ```
   - Create `recipes/nur/all/conandata.yml`:
     ```yaml
     sources:
       "1.4.0":
         url: "https://github.com/IreneAl-Caligo/nur.h/archive/refs/tags/v1.4.0.tar.gz"
         sha256: "<SHA256_OF_RELEASE_TARBALL>"
     ```
   - Create `recipes/nur/all/test_package/`:
     - `test_package/CMakeLists.txt`
     - `test_package/test_package.c`
     - `test_package/conanfile.py`
3. Verify test package locally:
   ```bash
   conan create recipes/nur/all/ --version=1.4.0
   ```
4. Open PR titled `[nur] Add version 1.4.0`.

---

## 3. Meson WrapDB Submission

**Target Repository**: [mesonbuild/wrapdb](https://github.com/mesonbuild/wrapdb)  

### Submission Steps:
1. Fork and clone `mesonbuild/wrapdb`:
   ```bash
   git clone https://github.com/<your-username>/wrapdb.git
   cd wrapdb
   git checkout -b add-nur
   ```
2. Add upstream git wrap definition to `subprojects/nur.wrap`:
   ```ini
   [wrap-git]
   url = https://github.com/IreneAl-Caligo/nur.h.git
   revision = v1.4.0
   depth = 1

   [provide]
   nur = nur_dep
   nur_core = nur_core_dep
   ```
3. Test with Meson:
   ```bash
   meson setup build -Dwraps=nur
   ```
4. Submit PR titled `nur: new wrap for 1.4.0`.
