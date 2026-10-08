# Changelog

All notable changes to nur.h will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-10-07

### Added
- Initial production release of nur.h
- `NurResult<T>` type system for type-safe error handling
- 50+ predefined error codes with systematic taxonomy
- `NUR_ERROR`, `NUR_ERROR_CTX`, `NUR_ERROR_ERRNO` constructors
- Source location tracking (file, function, line) for all errors
- `NUR_TRY_TO` and `NUR_TRY_VOID` macros for error propagation
- `NUR_TRY_VOID_FROM_VOID` macro for void-to-void error chaining
- `NUR_CHECK` and `NUR_CHECK_AS` macros for condition validation
- `NurOption<T>` type for optional values
- `[[nodiscard]]` attributes for C23, with GCC/Clang/MSVC fallbacks
- `nur_malloc` and `nur_realloc` wrappers returning `NurResult_ptr`
- `nur_error_print` and `nur_error_eprint` utilities
- Multi-standard support: C99, C11, C17, C23
- Cross-platform: GCC, Clang, MSVC
- Comprehensive test suite (16 unit tests + 7 C23 feature tests)
- 12 example programs demonstrating core patterns
- Full documentation and tutorials
- Bengali language guide for IDE integration

### Core Features
- Zero heap allocation in core library
- Header-only architecture
- No global mutable state
- Complete type safety with compiler enforcement

### Documentation
- README with quick-start guide
- API reference documentation
- Getting started tutorial
- CONTRIBUTING guidelines
- CODE_OF_CONDUCT
- SECURITY policy
- Bengali IDE setup guide (Code::Blocks, VS Code, Kiro, CLion, Dev-C++)

### Testing
- All tests passing on GCC 16.1.0
- C99/C11/C17/C23 standard compliance verified
- Example programs tested and working

### Known Limitations
- `NUR_TRY` macro (statement expression version) requires GNU extensions
- Void-to-void error propagation requires `NUR_TRY_VOID_FROM_VOID` macro

[1.0.0]: https://github.com/nurhannu/nur.h/releases/tag/v1.0.0