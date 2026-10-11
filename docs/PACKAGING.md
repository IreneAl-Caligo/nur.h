# Package Manager & Build System Integration

`nur.h` is designed for frictionless adoption across all standard C and C++ build environments.

---

## 1. CMake Integration

### Method A: Installed Package (`find_package`)

If `nur.h` is installed on your system:

```cmake
cmake_minimum_required(VERSION 3.14)
project(MyProject LANGUAGES C)

# Locate installed nur package
find_package(nur CONFIG REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE nur::nur)
```

### Method B: Git Submodule or Vendored Directory (`add_subdirectory`)

```cmake
add_subdirectory(vendor/nur)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE nur::nur)
```

### Method C: FetchContent (Automated Download)

```cmake
include(FetchContent)
FetchContent_Declare(
    nur
    GIT_REPOSITORY https://github.com/IreneAl-Caligo/nur.h.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(nur)

target_link_libraries(my_app PRIVATE nur::nur)
```

---

## 2. pkg-config Integration

`nur.h` includes a standard `nur.pc` file.

### Makefile usage:
```makefile
CFLAGS += $(shell pkg-config --cflags nur)
```

### Direct GCC / Clang command line:
```bash
gcc $(pkg-config --cflags nur) -o app main.c
```

---

## 3. Meson Build System

`nur.h` includes native `meson.build` definitions:

```meson
project('my_project', 'c', default_options : ['c_std=c11'])

nur_dep = dependency('nur', required : true)

executable('my_app', 'main.c', dependencies : nur_dep)
```

---

## 4. vcpkg Integration

Add `nur` to your `vcpkg.json`:

```json
{
  "name": "my-project",
  "version-string": "1.0.0",
  "dependencies": [
    "nur"
  ]
}
```

Or install directly via CLI:
```bash
vcpkg install nur
```

---

## 5. Direct Single-Header Vendoring

Since `nur.h` is completely self-contained with **zero dependencies**:

1. Copy `src/nur.h` directly into your project's `include/` directory.
2. Add `-Iinclude` to your compiler flags.
3. `#include "nur.h"` anywhere in your codebase.
