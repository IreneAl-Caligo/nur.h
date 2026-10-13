# Package Manager & Build System Integration

`nur.h` (and `nur_core.h`) is designed for frictionless adoption across all standard C and C++ build environments.

---

## 1. CMake Integration

### Method A: Single-Line `FetchContent` (Recommended)

No manual copying required. Add this directly to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
    nur
    GIT_REPOSITORY https://github.com/IreneAl-Caligo/nur.h.git
    GIT_TAG        v1.4.0
)
FetchContent_MakeAvailable(nur)

# Link full batteries-included nur (includes standard math/string/stdio wrappers)
target_link_libraries(my_app PRIVATE nur::nur)

# OR link ultra-lean, dependency-free primitives (nur_core.h only)
target_link_libraries(my_app PRIVATE nur::core)
```

### Method B: Installed Package (`find_package`)

If `nur` has been installed system-wide or via package managers (`vcpkg`, `conan`, `make install`):

```cmake
cmake_minimum_required(VERSION 3.14)
project(MyProject LANGUAGES C)

# Locate installed nur package
find_package(nur CONFIG REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE nur::nur)
# or nur::core
```

### Method C: Git Submodule or Vendored Directory (`add_subdirectory`)

```cmake
add_subdirectory(vendor/nur)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE nur::nur)
```

---

## 2. Meson Build System & Meson Wrap

### Using Meson Wrap (Single-Step Subproject)

Place [subprojects/nur.wrap](file:///subprojects/nur.wrap) into your project's `subprojects/` folder:

```ini
[wrap-git]
url = https://github.com/IreneAl-Caligo/nur.h.git
revision = v1.4.0
depth = 1

[provide]
nur = nur_dep
nur_core = nur_core_dep
```

Then in your `meson.build`:

```meson
project('my_project', 'c', default_options : ['c_std=c11'])

nur_dep = dependency('nur')
# or nur_core_dep = dependency('nur_core')

executable('my_app', 'main.c', dependencies : nur_dep)
```

---

## 3. Microsoft vcpkg

### Manifest Mode (`vcpkg.json`)
Add `nur` to your project's `vcpkg.json`:

```json
{
  "name": "my-project",
  "version-string": "1.0.0",
  "dependencies": [
    "nur"
  ]
}
```

### Classic CLI Mode:
```bash
vcpkg install nur
```

In CMake, configure with `-DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake`, then:
```cmake
find_package(nur CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE nur::nur)
```

---

## 4. Conan 2.0 (Conan Center)

Add to your `conanfile.txt`:
```ini
[requires]
nur/1.4.0

[generators]
CMakeDeps
CMakeToolchain
```

Or install directly via CLI:
```bash
conan install --requires="nur/1.4.0" --build=missing
```

---

## 5. pkg-config Integration

`nur.h` provides a standard `nur.pc` file:

### Makefile usage:
```makefile
CFLAGS += $(shell pkg-config --cflags nur)
```

### Direct GCC / Clang command line:
```bash
gcc $(pkg-config --cflags nur) -o app main.c
```

---

## 6. Direct Single-Header Vendoring

Since `nur.h` and `nur_core.h` are completely self-contained with **zero dependencies**:

1. Copy `src/nur.h` (or `src/nur_core.h`) directly into your project's `include/` directory.
2. Add `-Iinclude` to your compiler flags.
3. `#include "nur.h"` anywhere in your codebase.
