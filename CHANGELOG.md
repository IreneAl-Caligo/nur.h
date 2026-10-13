# Changelog

All notable changes to nur.h will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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