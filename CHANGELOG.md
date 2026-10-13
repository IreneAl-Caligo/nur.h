# Changelog

All notable changes to nur.h will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.4.0] - 2026-10-13

### Added
- **Permanent ABI Layout Freeze Specification (`docs/ABI_SPECIFICATION.md`)**:
  - Formally specified and frozen byte sizes, struct member offsets, and alignment requirements across 64-bit (LP64, LLP64) and 32-bit (ILP32) architectures for all predefined Result and Option types.
  - Added dedicated ABI layout regression test suite (`test/test_abi_layout.c`) with static and runtime verification.
- **Ultra-Lean Primitives Header (`src/nur_core.h`)**:
  - Independent, zero-dependency Result and Option primitive implementation (<750 lines) requiring exclusively `<stdbool.h>`, `<stddef.h>`, and `<stdint.h>`.
  - 100% binary ABI compatible with `src/nur.h`.
- **Clang-Tidy & Clang-Format CI Quality Gates (`.github/workflows/ci.yml`)**:
  - Integrated Clang-Tidy static analysis (`--warnings-as-errors='*'`) and Clang-Format enforcement in GitHub Actions.
  - Added multi-compiler, multi-standard matrix (C89, C99, C11, C17, C23) with GCC, Clang, and MSVC.
  - Automated 32-bit and 64-bit ABI layout freeze verification jobs.
- **First-Class CMake `FetchContent` and Meson Wrap Support**:
  - Single-line integration: `FetchContent_Declare(nur ...)` exposes targets `nur::nur` and `nur::core`.
  - Added `subprojects/nur.wrap` and Meson dependency overrides (`nur`, `nur_core`).
- **MISRA-C & Embedded Zero-Macro Compliance Guide (`docs/MISRA_COMPLIANCE.md`)**:
  - Documented 100% macro-free functional API (`NUR_PURIST 1` / `NUR_NO_HIDDEN_RETURNS 1`) satisfying MISRA C:2012 / 2023 Rules 14.4, 15.5, 20.10, 21.3, and 21.6.
- **Package Manager Deployment Ready**:
  - Added Microsoft `vcpkg` registry port recipe (`ports/nur/portfile.cmake`, `ports/nur/vcpkg.json`).
  - Added Conan 2.0 package definition (`conanfile.py`) and step-by-step submission guide (`docs/PACKAGE_REGISTRY_SUBMISSION.md`).
- **Inline Message Lifetime Protection**:
  - Added `char inline_msg[64]` to `nur_error_t` and `NUR_ERROR_COPY` / `NUR_ERROR_FMT` macros preventing use-after-free from local stack strings.
- **Multi-TU Weak Symbol Synchronization**:
  - Replaced `static` storage for panic handler with weak linkage (`__attribute__((weak))` / `__declspec(selectany)`), ensuring seamless multi-translation unit panic synchronization without duplicate symbol errors.

### Changed
- **Removed `<math.h>` and `<time.h>` Dependencies**: Completely decoupled `src/nur.h` from standard math and time libraries; implemented Newton-Raphson approximation in `nur_sqrt`.
- Version synchronized to 1.4.0 across `VERSION`, `CMakeLists.txt`, `meson.build`, `nur.pc`, `vcpkg.json`, and `conanfile.py`.

## [1.3.2] - 2026-10-13

### Added
- **Built-in Safe Drop-in Helpers (`nur_sqrt`, `nur_strlen`)**: Added zero-allocation domain validation helpers:
  - `nur_sqrt(double x)`: Computes square root safely returning `NurResult_f64`; returns `NUR_E_INVALID_ARGUMENT` on negative numbers instead of generating silent `NaN`.
  - `nur_strlen(const char *str)`: Computes string length safely returning `NurResult_usize`; returns `NUR_E_NULL_POINTER` on `NULL` pointers instead of triggering a segmentation fault.
  - C++ `namespace nur` overloads `nur::sqrt(double)` and `nur::strlen(const char *)`.
- **Comprehensive Drop-in Safety & Domain Test Suite (`test/test_super_header_and_dropin.c`)**:
  - Validates zero-extra-header standard C compilation (`printf`, `sqrt`, `strlen`, `rand`, `time`).
  - Validates the 4 high-performance domain patterns (Pure numeric linear algebra, tight loops with validate-at-entry, trivial getters/setters with `NurOption`, and hardware ISRs with 0 heap allocation).
  - 100% ISO C89/C90 compatible, compiling cleanly across C89, C99, C11, C17, C23, and C++11 with zero warnings under `-Wall -Wextra -Wpedantic`.
- **Opt-In Safety Architecture Documentation**:
  - Expanded `README.md`, `AGENTS.md` (Section 7), and `AI_POLICY.md` (`NUR-AI-008`) with architectural clarity on when and how to apply `nur.h` with zero runtime overhead.
- **Zero-Friction Drop-in Terminal Demo (`assets/demo_hello_world.svg`)**: Added interactive animated SVG showing transition from `#include <stdio.h>` to `#include "nur.h"` with instant `printf("Hello, World!\n")` drop-in compatibility and zero heap allocation.

### Fixed
- **C++ Smoke Test Assertion Hardening under NDEBUG**: Replaced standard `assert(...)` in `test/test_cpp_smoke.cpp` with an explicit `EXPECT(...)` check that outputs failure diagnostics to `stderr` and returns a non-zero exit code (`1`), ensuring assertions are not silently lost when compiling Release or `-DNDEBUG` builds. Decoupled test from `<iostream>` and `<string>` to eliminate clangd standard library header parsing incompatibilities.
- **32-Bit Target Integer Type Identity (`unsigned long long` -> `NurResult_u64`)**: Mapped `unsigned long long` to `NurResult_u64` in C++ `nur_cpp_result_traits` and C11 `_Generic` (`nur_internal_gen_ullong`), eliminating 64-to-32-bit integer truncation via `size_t` on 32-bit platforms.
- **Typed Result Mapping Compatibility (`NUR_MAP_TO` & `NUR_MAP`)**: Fixed struct type mismatch in the `NUR_MAP_TO` ternary operator by constructing `nur_err_##suffix((result).u.error)` on error instead of returning `(result)`, guaranteeing both ternary branches evaluate to the exact same Result struct type. Documented and restricted `NUR_MAP` to same-type transformations.
- **Developer CLI Scaffolding & Linter Hardening (`tools/nur-cc`, `tools/nur-cc.c`)**:
  - Protected against accidental overwriting of existing projects if `main.c` or `Makefile` exists.
  - Verified that `src/nur.h` was successfully found and copied before reporting project creation success.
  - Removed duplicate `NUR_RESULT_TYPE(int32_t, i32);` declaration from `--wrap` migration recipes.
  - Enforced mandatory operands for `--new` and `--wrap`.
  - Upgraded C comment scanning to preserve `//` inside string literals and track multiline `/* ... */` block comments across lines.
  - Disallowed treating `NUR_IS_ERR` as a safe-value predicate before `NUR_VALUE`.
  - Re-scanned files after `--fix` to preserve unrepairable violations (e.g. `gets()`) and return non-zero exit codes under `--lint`.
- **Union Member Type-Safety in `NUR_ASSERT_ERR`**: Fixed undefined behavior caused by reading `r.u.error` when `NUR_IS_OK(r)` evaluated to true (where the active union payload is `r.u.value`). Dispatched to distinct panic branches for Ok results vs code mismatches.
- **Semantic Error Classification in `NUR_UNWRAP_OPTION`**: Attached explicit `NUR_ERROR(NUR_E_INVALID_STATE, "Option is None")` rather than dummy `NUR_E_NONE` when unwrap aborts on None.
- **Native OS Directory Scaffolding**: Replaced shell invocation (`system("mkdir ...")`) in `tools/nur-cc.c` with native platform APIs (`_mkdir` / `mkdir`), eliminating command processor overhead and CWE-78 risk.
- **Structured JSON Error Buffer Consistency**: Harmonized `nur_error_log_json` to utilize `NUR_PRINT_BUFFER_SIZE` (1024 bytes) for consistent zero-heap error streaming.
- **Repository-Wide Version Synchronization**: Synchronized version 1.3.2 across `src/nur.h`, `VERSION`, `CMakeLists.txt`, `meson.build`, `nur.pc`, `tools/nur-cc.c`, and `tools/nur-cc`.
- **Reserved Identifier Compliance**: Eliminated all reserved global identifiers (`_nur_gen_*`, `_NUR_GET_MACRO_3`, `_NUR_OK_*`, `_nur_custom_panic_handler`, `_nur_cpp_result_traits`) in compliance with ISO C 7.1.3 and C++ standards.
- **Clangd / Clang-Tidy Diagnostics & Branch Cloning**: Stacked consecutive switch branches in Win32 and errno error code mappers to eliminate branch-clone warnings. Configured `.clangd` and `.clang-tidy` to eliminate false-positive swappable parameter and unused-include warnings.
- **Guarded `__has_c_attribute` Macro**: Safely decoupled and guarded C++17 attribute checking to prevent undefined function-like macro errors in C++ compilation.
- **Single-Evaluation Context Chaining (`NUR_ERROR_CONTEXT_AT`)**: Replaced macro with inline helper function `nur_error_context_at`, preventing argument double-evaluation and eliminating `-Wsign-compare` compiler warnings.
- **Freestanding Integer Bounds Fallbacks (`INT32_MAX`, `INT32_MIN`)**: Added portable constant definitions when compiling in freestanding environments or with `NUR_NO_STDINT`.
- **Unused Panic Handler Variable Protection**: Annotated `nur_internal_custom_panic_handler` with `NUR_UNUSED` to suppress compiler `-Wunused-variable` diagnostics in isolated translation units.
- **Robust File Truncation Detection in `nur_file_read_text`**: Added explicit `ferror(f)` validation if `fgetc` encounters an unexpected error during the single-byte post-read verification, ensuring I/O errors are faithfully classified as `NUR_E_IO`.
- **Compiler Configuration & Buffer Safety in `tools/nur-cc.c`**: Added `CC` environment variable detection (falling back to `gcc`) and bounds-checked string termination in automated semicolon repair.
- **Test Include Relative Path**: Normalized header include to `../src/nur.h` in `test/test_super_header_and_dropin.c`.

## [1.3.1] - 2026-10-13

### Added
- **C++11/14/17/20/23 Universal Single-Token Constructor**: Full C++ template traits (`_nur_cpp_result_traits`) and overloads allowing `NUR_H(val)` and `NUR_OK(val)` to be used transparently in C++ without relying on C11 `_Generic`.
- **Standalone Header Smoke Test (`test/test_standalone_include.c`)**: Verifies that `src/nur.h` compiles with zero pre-included headers.
- **Multi-Translation Unit & ODR Safety Suite (`test/test_odr_tu1.c`, `test/test_odr_tu2.c`)**: Verifies multi-TU compilation and linking with zero duplicate symbol linker errors.
- **Dedicated C++ Smoke & Compatibility Suite (`test/test_cpp_smoke.cpp`)**: Verified across C++11 and C++17.
- **CLI Feature Parity**: Full parity between `tools/nur-cc.c` and `tools/nur-cc` bash script including `--lint`, `--new`, `--wrap`, and `--examples`.

### Fixed
- **Silent File Truncation Prevention (`nur_file_read_text`)**: Now inspects `fgetc(f) != EOF` when buffer capacity is reached; explicitly returns `NUR_E_TRUNCATED` error instead of silently returning truncated content.
- **CMake Packaging Order**: Moved `include(GNUInstallDirs)` before `target_include_directories` so `${CMAKE_INSTALL_INCLUDEDIR}` is properly defined, and added `INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}` for exported downstream targets.
- **NUR_FREESTANDING Support**: Explicit flag handling to decouple hosted headers in bare-metal environments.
- **ANSI C89 Pure Compatibility**: Added `NUR_NO_STDINT` typedef fallback for environments lacking `<stdint.h>`.
- **Case-Sensitive Linux Makefile Compatibility**: Fixed `TEST/` path to lowercase `test/` preventing Linux build breakages.
- **AI Linter Heuristic Clarification**: Comment stripping before syntax audit and clear heuristic identification in `tools/nur-cc`.
- **CI Workflow Location**: Moved workflow file from `.ci/ci.yml` to standard `.github/workflows/ci.yml`.

## [1.3.0] - 2026-10-13

### Added
- **ISO C89 / ANSI C Full Compatibility**: Support for compiling under `-std=c89 -Wall -Wextra -Wpedantic` with zero warnings. Added C89 fallback type definitions (`bool`, `true`, `false`), standard compliant inlining fallbacks, and strict function location tracking (`NUR_LOC_FUNC` fallback to `"unknown"` under ANSI mode).
- **C-Purist Safety Mode (`NUR_PURIST` / `NUR_NO_HIDDEN_RETURNS`)**: A dedicated compile-time enforcement switch for codebases that forbid hidden control flow inside macros (such as MISRA-C Rule 15.5 or single-exit guidelines). Disables all hidden early-return macros (`NUR_TRY*`, `NUR_CHECK*`, `NUR_RETURN_IF_ERR`) at compile time.
- **First-Class Pure-Function API (100% Macro-Free)**:
  - Typed inspection functions: `nur_is_ok_<type>`, `nur_is_err_<type>`, `nur_value_<type>`, `nur_error_<type>`, `nur_unwrap_or_<type>`.
  - Void inspection functions: `nur_is_ok_void`, `nur_is_err_void`, `nur_error_void_val`.
  - Universal pointer predicates: `nur_result_is_ok(&res)`, `nur_result_is_err(&res)`.
  - Pure error field extractors: `nur_error_code_of(&err)`, `nur_error_message_of(&err)`.
- **Dedicated C89 & C-Purist Test Suite**: Added `TEST/test_c89_and_purist.c` and `make test-c89`, `make test-purist`, verifying zero warnings across C89, C99, C11, C17, and C23.

### Fixed
- **C89 Variable Declaration Scoping**: Converted all inside-loop counter declarations (`for (size_t i = ...)` / `for (int i = ...)`) to block-scope top declarations.
- **Declaration-After-Statement Warnings**: Fixed statement-declaration ordering across internal memory, file, and JSON helpers.
- **ISO C90 `long long` Pedantic Warnings**: Constrained uintmax string sink formatting to `unsigned long` under C89 to eliminate `-Wlong-long` warnings.
- **Universal Pointer Predicate Collision**: Renamed universal result pointer predicates to `nur_result_is_ok` and `nur_result_is_err` to prevent naming collision with `nur_is_ok_ptr` for `NurResult_ptr`.

## [1.2.1] - 2026-10-13

### Added
- **Type-Level `nodiscard` Support**: Added C23 `[[nodiscard]]` directly to `NurResult_*` typedefs and provided the `NUR_FN(ReturnType, FuncName, ...)` helper macro for older standards.
- **Automated Compile-Fail Quality Gate**: Added `test/test_compile_fail_nodiscard.c` and `make test-compile-fail` to continuously verify that unused results trigger compiler rejections.
- **Multi-Level Breadcrumb Context Chaining**: Added fixed-depth `context_chain[NUR_MAX_CONTEXT_DEPTH]` (depth 4) to `nur_error_t` with zero heap allocation, preserving both intermediate breadcrumb layer contexts and root `.cause`.
- **Pure Freestanding & Embedded Decoupling**: Completely decoupled `NUR_PANIC`, `NUR_UNWRAP`, assertions, and panic handlers from `<stdio.h>` and `<stdlib.h>`. Added configurable hardware trap hook `NUR_PANIC_ABORT()`.
- **Lightweight Mode (`NUR_LITE`)**: Minimal footprint preset disabling optional math/file super-extensions and safety assertions for bare-metal/constrained microcontrollers.
- **AI / LLM Inviolable Policy**: Added machine-readable `AI_POLICY.md` (`NUR-AI-001` through `NUR-AI-007`) for automated agent code verification and IDE linter rules.
- **Rust to C Rosetta Stone**: Added `docs/RUST_TO_NUR.md` mapping Rust Result/Option paradigms directly to `nur.h`.
- **Structured Error Schema**: Added `docs/schema/nur_error_schema.json` conforming to JSON-Schema Draft-07 for logging and telemetry.
- **GitHub Actions Hardened CI**: Added multi-OS (Linux GCC/Clang, macOS AppleClang, Windows MSVC) workflow in `.github/workflows/ci.yml`.
- **CLI Enhancements (`tools/nur-cc`)**: Added project scaffolding (`--new`), migration recipes (`--wrap`), and bilingual linting (`--lang=en|bn`).

### Fixed
- **ISO C11 Strict Compatibility**: Replaced GNU `typeof` with `__typeof__` in `NUR_TRY_AUTO` for GCC/Clang in `-std=c11` strict pedantic mode.
- **Freestanding Header Independence**: Fixed implicit `memset` and `abort` declarations when compiling with `-DNUR_NO_STRING -DNUR_NO_STDIO -DNUR_NO_STDLIB`.
- **Non-GNU Fallback Single-Evaluation**: Eliminated double evaluation in non-GNU fallback expressions for `NUR_UNWRAP_OR`.
- **Flat Layout Parity**: Ensured `NurResult_void` layout and field naming symmetry across pre-C11 compilers.

## [1.2.0] - 2026-10-13

### Added
- Universal Single-Token Result Constructor `NUR_H(val)` using C11 `_Generic` covering `int32_t`, `int64_t`, `size_t`, `float`, `double`, `char*`, `const char*`, `void*`, and `bool`.
- Preprocessor macro overloading for `NUR_OK`:
  - 1-Argument: `NUR_OK(val)` dynamically dispatches via `NUR_H(val)`.
  - 2-Arguments: `NUR_OK(suffix, val)` maintains legacy/explicit dispatch across C99/C11/C17/C23.
- Drop-in Compile-Time Security Shield: compile-time assertion poisoning of obsolete `gets()` and typo protection intercepting `prinf()` with actionable compiler notes. Opt-out via `NUR_NO_SECURITY_SHIELD`.
- Batteries-Included Super-Header: automatically includes `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<math.h>`, `<time.h>`, `<ctype.h>`, `<stdbool.h>`, `<stdint.h>`, `<stddef.h>`, and `<errno.h>`.
- Batteries-Included Safe Math helpers: `nur_add_i32`, `nur_sub_i32`, `nur_mul_i32`, `nur_div_i32` with zero runtime overhead and overflow error safety.
- Batteries-Included Safe File I/O: `nur_file_read_text` for zero-heap, automatically-closed text file reading into caller buffer with null-termination guarantee.
- Universal Developer CLI Toolchain (`tools/nur-cc`, `tools/nur-cc.c`, `tools/nur-cc.ps1`):
  - Pre-flight scanner detecting missing semicolons on `return` statements and typos before compiler invocation.
  - Multi-language internationalization (Universal English default, Bengali locale auto-detection, `--lang=en` and `--lang=bn` flags).
  - `--fix` automated code correction and `.clang-format` integration.
- Comprehensive end-to-end test suites: `test/test_universal_type.c` and `test/test_all_features_and_errors.c`.

## [1.1.0] - 2026-10-12

### Fixed
- Fixed C99 `-Wpedantic` unnamed struct/union warning in `NurResult_void`, achieving 100% warning-free compilation under `-Wall -Wextra -Wpedantic` across ISO C99, C11, C17, and C23.

### Added
- Ergonomic Result combinators: `NUR_MAP` (value transformation), `NUR_AND_THEN` (monadic chaining), and `NUR_UNWRAP_OR` with C11 `_Generic` single-evaluation dispatch on MSVC.
- Zero-allocation Root Cause Error Chaining: `.cause` field in `nur_error_t`, `NUR_ERROR_WRAP`, `NUR_ERROR_CAUSE`, and `NUR_ERROR_CAUSED_BY`.
- Compile-time string literal verification macros: `NUR_ERROR_LIT` and `NUR_ERROR_LIT_CTX` preventing stack-buffer dangling pointer lifetime bugs.
- Scope-based automatic resource cleanup: `NUR_CLEANUP`, `NUR_AUTO_FREE`, and `NUR_AUTO_FILE` for GCC/Clang.
- Structured JSON logging streamers: `nur_error_log_json` and `nur_error_elog_json`.
- Ergonomic macros: `NUR_REQUIRE_OK`, `NUR_REQUIRE_OK_MSG`, `NUR_IF_ERR`, `NUR_VOID_IF_ERR`, `NUR_MATCH_AS`, `NUR_MATCH`, `NUR_OR_ELSE`, `NUR_VOID_ERROR_OF`.
- Production-grade real-world examples:
  - `examples/real-world/03_cli_arg_parser.c`: Zero-allocation CLI argument parsing with port validation.
  - `examples/real-world/04_json_parser.c`: Streaming zero-allocation JSON tokenizer & parser.
  - `examples/real-world/05_socket_server.c`: Cross-platform TCP socket server with leak-free `NUR_TRY_GOTO`.
  - `examples/real-world/06_http_client.c`: Wire-format HTTP 1.1 request serializer & response parser.
  - `examples/real-world/07_thread_safe_logging.c`: Thread-safe structured logging with RFC 8259 JSON serialization.
  - `examples/real-world/08_embedded_sensor.c`: Microcontroller / bare-metal hardware sensor driver with CRC-8 validation.
- Migration guide: `docs/MIGRATION.md` for refactoring legacy C (`errno`, `NULL`, sentinel returns).
- AI / LLM Safe Guidelines: `docs/AI_GUIDELINES.md` with usage contract and refactoring prompt pack.
- Machine-readable error taxonomy: `docs/ERROR_TAXONOMY.md` and `docs/errors.json`.
- Latency and zero-allocation performance documentation: `docs/BENCHMARKS.md`.
- Comprehensive packaging guide: `docs/PACKAGING.md` (CMake, pkg-config, Meson, vcpkg, Conan).
- Automated migration helper script: `scripts/wrap_legacy_c.py`.
- Modern CMake packaging with `nurConfig.cmake.in` and version compatibility checks.
- Native Meson build support (`meson.build`, `meson_options.txt`).
- Hardened GitHub Actions CI (`.github/workflows/ci.yml`) covering all standards, MSVC, sanitizers (ASan + UBSan), and static analysis.
- Project roadmap and governance: `ROADMAP.md`.

## [1.0.0] - 2026-10-11

### Added
- Production release of nur.h by Irene Al-Caligo
- `NurResult<T>` type system for type-safe error handling
- 36 semantic error codes with systematic taxonomy (+ NONE, USER_BASE)
- `NUR_ERROR`, `NUR_ERROR_CTX`, `NUR_ERROR_ERRNO`, `NUR_ERROR_FROM_WIN32` constructors
- Semantic errno mapping (`nur_error_code_from_errno`) for POSIX syscalls
- Full Win32 and Winsock error mapping with corrected EOF and WSAEWOULDBLOCK codes
- Source location tracking (file, function, line) with `NUR_NO_SOURCE_LOCATION` stripping option
- `NUR_TRY_GOTO` and `NUR_TRY_VOID_GOTO` macros for leak-free C cleanup
- `NUR_TRY_AUTO` and `NUR_TRY_VOID_AUTO` using `__typeof__` for strict C11/C99/C17 compatibility
- `NUR_TRY_TO` and `NUR_TRY_VOID` macros for portable error propagation
- `NUR_TRY_VOID_FROM_VOID` macro for void-to-void error chaining
- Flat `NurResult_void` layout without union aliasing mismatches
- Value-based `NUR_UNWRAP` on non-GNU preventing rvalue lvalue errors
- Configurable panic handler via `nur_set_panic_handler()`
- `noinline`, `cold`, and `noreturn` panic helpers reducing binary code size by ~11x
- `nur_error_json()` zero-allocation RFC 8259 JSON formatter
- `NUR_ASSERT_OK` and `NUR_ASSERT_ERR` unit test assertion helpers
- `NurOption<T>` type for optional values
- `[[nodiscard]]` attributes for C23 type-level safety, with GCC/Clang/MSVC fallbacks
- `nur_malloc` and `nur_realloc` wrappers returning `NurResult_ptr`
- `NUR_WRAP_ERROR` macro for error context chaining preserving root traces
- Zero-allocation string formatting engine (`nur_error_format`)
- CMake `INTERFACE` target (`nur::nur`) and `FetchContent` support
- pkg-config `nur.pc` specification
- Multi-standard support: C99, C11, C17, C23, and C++11+
- Comprehensive test suite (26 unit tests + 7 C23 feature tests)
- Full GitHub Actions CI matrix for GCC, Clang, and MSVC

[1.3.0]: https://github.com/IreneAl-Caligo/nur.h/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/IreneAl-Caligo/nur.h/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/IreneAl-Caligo/nur.h/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/IreneAl-Caligo/nur.h/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/IreneAl-Caligo/nur.h/releases/tag/v1.0.0