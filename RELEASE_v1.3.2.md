# nur.h v1.3.2 Release Notes

**Release Date**: October 13, 2026  
**Tag**: `v1.3.2`  
**License**: MIT  
**Author**: Irene Al-Caligo  

---

## 🌟 Overview

`nur.h` version **1.3.2** represents a major milestone in portability, safety, and developer ergonomics. This release hardens the library against undefined behavior and CWE risks, completes 100% internationalization across all tools and documentation, expands native C++11+ integration (`namespace nur`), and introduces the **Opt-In Safety Architecture** — allowing bare-metal speed in numeric arithmetic, DSP inner loops, and hardware ISRs with zero runtime overhead and zero heap allocations.

---

## 🚀 Key Highlights & New Capabilities

### 1. Batteries-Included Super-Header & Drop-In Standard C
`nur.h` automatically includes standard C headers (`<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<math.h>`, `<time.h>`, `<ctype.h>`, `<stdbool.h>`, `<stdint.h>`, `<errno.h>`). Existing standard C code runs directly with **zero additional headers** and zero boilerplate.

### 2. High-Performance Domain Patterns
Clear architectural guidance and zero-cost patterns for high-frequency domains:
- **Pure Numeric Arithmetic**: Direct primitive types (`float`, `double`, `sin`, `cos`) with 0 overhead.
- **Tight Inner Loops**: Boundary validation at entry (`NUR_CHECK`), then 100% bare-metal vectorized loop execution.
- **Trivial Getters/Setters**: Direct scalar access or zero-overhead `NurOption` (`NUR_SOME`, `NUR_NONE`).
- **Hardware Interrupt Service Routines (ISRs)**: Guaranteed zero heap allocation (`malloc`/`free` are never called internally); freestanding support via `#define NUR_FREESTANDING 1`.

### 3. Built-In Safe Math & File I/O
- `nur_add_i32(a, b)`, `nur_sub_i32(a, b)`, `nur_mul_i32(a, b)`: Overflow-safe 32-bit arithmetic returning `NUR_E_OVERFLOW`.
- `nur_div_i32(a, b)`: Division-by-zero protection (`NUR_E_INVALID_ARGUMENT`) and `INT32_MIN / -1` overflow guard (`NUR_E_OVERFLOW`).
- `nur_file_read_text(path, buf, size)`: Zero-allocation file reader with automatic descriptor cleanup, null termination, and robust silent truncation detection.
- `nur_sqrt(double x)`: Domain-protected square root returning `NUR_E_INVALID_ARGUMENT` on negative numbers instead of generating silent `NaN`.
- `nur_strlen(const char *str)`: Null-safe string length returning `NUR_E_NULL_POINTER` instead of triggering a segmentation fault.

### 4. Modern C++11+ Integration (`namespace nur`)
When compiled with C++ compilers (`g++`, `clang++`, MSVC), `nur.h` provides native namespaces and overloaded methods:
- `nur::ok(val)`: Auto-deduces result type at compile time via template traits.
- `nur::err(e)`: Construct error results.
- `nur::is_ok(r)`, `nur::is_err(r)`, `nur::value(r)`, `nur::unwrap_or(r, fallback)`.
- `nur::sqrt(x)`, `nur::strlen(str)`.

### 5. Security & Vulnerability Hardening
- **CWE-476 / Union UB in `NUR_ASSERT_ERR`**: Fixed undefined behavior caused by accessing inactive `.u.error` while `.u.value` was live on success results. Dispatches to separate branches for Ok checks vs error code mismatches.
- **Single-Evaluation Context Chaining (`NUR_ERROR_CONTEXT_AT`)**: Replaced macro with `static inline const char *nur_error_context_at(...)`, eliminating argument double evaluation and `-Wsign-compare` warnings.
- **Freestanding Constant Definitions**: Added fallback definitions for `INT32_MAX` and `INT32_MIN` when compiling with `NUR_NO_STDINT` or `-ffreestanding`.
- **CWE-78 Command Injection Elimination in `tools/nur-cc.c`**: Replaced `system("mkdir ...")` with native OS filesystem APIs (`_mkdir` / `mkdir`). Added `CC` environment variable detection and buffer bounds validation.
- **Type Safety in Ternary Operator (`NUR_MAP_TO`)**: Fixed ternary branches to guarantee both branches evaluate to identical `NurResult_##suffix` types.
- **32-Bit Target Precision**: Corrected integer mapping in C++ traits and C11 `_Generic` to `NurResult_u64` on 32-bit platforms.

### 6. 100% Internationalization
- Completely eliminated localized strings and dual-language branches across `tools/nur-cc.c` and `tools/nur-cc`.
- Standardized all CLI diagnostics, linting messages, and templates to international English with ANSI formatting.

---

## 🧪 Verification Matrix

| Target | Command | Result | Standard Guarantee |
| :--- | :--- | :--- | :--- |
| **Multi-Standard Syntax** | `make check` | **PASS** | Zero warnings across C89, C99, C11, C17, C23 under `-Wall -Wextra -Wpedantic` |
| **All C Standards** | `make test-all` | **PASS** | 100% passing tests on ISO C89, C99, C11, C17, and C23 |
| **Core Test Suite** | `make test` | **PASS** | 29 core library tests passed |
| **C++ Compatibility** | `make test-cpp` | **PASS** | Clean compilation and assertion testing in C++11 |
| **Freestanding Mode** | `make test-freestanding` | **PASS** | Compiles with `-ffreestanding -DNUR_FREESTANDING` |
| **Multi-TU (ODR)** | `make test-odr` | **PASS** | 0 duplicate symbols across translation units |
| **Compile-Fail Safety** | `make test-compile-fail` | **PASS** | Compiler rejects discarded `NurResult` values via `[[nodiscard]]` |
| **Production Examples** | `make examples` | **PASS** | All 17 examples passed |
| **Microbenchmarks** | `make benchmark` | **PASS** | ~0.4 ns/op (equivalent to native primitive scalar returns) |

---

## 📦 Quick Installation

### Single-Header Direct Download
```bash
curl -fsSL https://raw.githubusercontent.com/IreneAl-Caligo/nur.h/v1.3.2/src/nur.h -o include/nur.h
```

### CMake
```cmake
find_package(nur CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE nur::nur)
```

### pkg-config
```bash
gcc $(pkg-config --cflags nur) main.c -o main
```

---

## 📜 Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for detailed commit history and migration notes.
