# Changelog

All notable changes to nur.h will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

[1.0.0]: https://github.com/IreneAl-Caligo/nur.h/releases/tag/v1.0.0