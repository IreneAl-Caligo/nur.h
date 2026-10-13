# nur.h v1.3.2 — Universal Compatibility, Security Hardening, and Pure-Function Architecture

A production-grade milestone release delivering zero-friction standard C drop-in compatibility, safe built-in math and string domain helpers (`nur_sqrt`, `nur_strlen`), universal C11+ and C++11+ single-token constructors (`NUR_H`), complete ISO C89/C90 compliance, compile-time security shielding against vulnerable legacy functions, and 100% zero-heap allocation across all standards.

---

### 🌟 Key Highlights & Features

1. **Zero-Friction Drop-in Super-Header (`#include "nur.h"`)**:
   - Replaces `#include <stdio.h>` with zero code changes needed.
   - Batteries-included access to standard I/O, stdlib, strings, math, time, and ctype.
   - Immediately equips existing codebases with `NurResult<T>`, `NurOption<T>`, safe arithmetic, and compiler-enforced `[[nodiscard]]` safety.
   - Interactive animated terminal demo added in `assets/demo_hello_world.svg`.

2. **Built-in Safe Domain Helpers (`nur_sqrt`, `nur_strlen`)**:
   - `nur_sqrt(double x)`: Computes square roots safely returning `NurResult_f64`; classifies negative inputs as `NUR_E_INVALID_ARGUMENT` rather than producing silent `NaN`.
   - `nur_strlen(const char *str)`: Calculates string lengths safely returning `NurResult_usize`; returns `NUR_E_NULL_POINTER` on `NULL` pointers instead of triggering memory faults.
   - C++ `namespace nur` overloads `nur::sqrt` and `nur::strlen`.

3. **Universal Single-Token Constructors (`NUR_H` & `NUR_OK`)**:
   - `NUR_H(val)`: C11 `_Generic` and C++ template traits auto-deduce result types (`i32`, `i64`, `usize`, `f32`, `f64`, `cstr`, `ptr`, `bool`) in a single token.
   - `NUR_OK(type, val)`: Full multi-standard backward compatibility across C89, C99, C11, C17, and C23.

4. **ISO C89 / ANSI C & C-Purist Macro-Free API**:
   - Zero-warning compilation under `-std=c89 -Wall -Wextra -Wpedantic`.
   - `NUR_PURIST` / `NUR_NO_HIDDEN_RETURNS` mode disables macros containing hidden control flow (MISRA-C Rule 15.5 compliance).
   - Macro-free pure inspection API (`nur_is_ok_<type>`, `nur_value_<type>`, `nur_result_is_ok`, `nur_error_code_of`).

5. **Modern C++ Interoperability (`namespace nur`)**:
   - Clean C++11+ type aliases and helpers (`nur::ok`, `nur::err`, `nur::value`, `nur::unwrap_or`).
   - Seamless compatibility with `std::nullopt_t` and standard C++ templates.

6. **Compile-Time Security Shield & Vulnerability Hardening**:
   - Built-in shields block inherently vulnerable functions (such as `gets()`, CWE-242) and common syntax typos (`prinf`) at compile time.
   - Hardened CLI tool (`tools/nur-cc`) using native directory creation APIs, eliminating shell overhead and CWE-78 risk.
   - Hardened `nur_file_read_text` with post-read single-byte verification, eliminating silent file truncation bugs.

7. **Zero Heap Allocation Guarantee**:
   - Exactly 0 bytes allocated on the heap across core types and error handling.
   - Verified clean with AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan).

---

### 📦 Artifacts & Distribution
- Single Header: `src/nur.h` (Drop-in ready, header-only, zero heap allocation)
- Verified Compilers: GCC 16.1.0, Clang, AppleClang, MSVC across C89, C99, C11, C17, C23, and C++11–C++23.
