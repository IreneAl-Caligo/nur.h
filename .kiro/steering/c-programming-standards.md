---
description: C programming standards and best practices for the nur.h library. Enforces ISO C compliance, memory safety, and portable coding conventions across all C code generated or reviewed in this workspace.
inclusion: always
---

# C Programming Standards for nur.h

## Standard Compliance

- Default compilation target: **C11** (`-std=c11`). Optionally C17 or C23.
- Every header must compile cleanly with `-std=c99 -Wall -Wextra -Wpedantic` (lowest common denominator).
- Use `__STDC_VERSION__` guards for C11/C17/C23 features.
- Never use compiler-specific extensions without a fallback macro that disables them on unsupported compilers.

## Portability

- Target compilers: GCC ≥ 9, Clang ≥ 10, MSVC 2019+.
- Use `<stdint.h>` fixed-width types (`int32_t`, `uint64_t`) instead of `int`, `long`.
- Use `<stddef.h>` for `size_t` and `ptrdiff_t`.
- Use `<stdbool.h>` for `bool`; never assume `int` == `bool`.
- Avoid VLAs (Variable Length Arrays) — not guaranteed in C11+.

## Header-Only Library Rules

- All functions must be `static inline` to avoid ODR violations.
- Guard every header with `#ifndef NUR_H / #define NUR_H / ... / #endif`.
- No global mutable state. Thread safety is the default expectation.
- Avoid `#include <stdlib.h>` in the core header unless `NUR_NO_STDLIB` guard is provided.

## Memory Safety

- Never dereference a pointer without a NULL guard.
- All allocation wrappers must return `NurResult_ptr` — never raw NULL.
- Document ownership semantics on every function that allocates: who frees, and when.
- Use `(void)expr` to explicitly acknowledge intentionally discarded results.

## Macro Hygiene

- Wrap multi-statement macros in `do { ... } while (0)`.
- Parenthesise every macro parameter at every use site: `(val)` not `val`.
- Avoid macro parameter names that shadow common C keywords (`type`, `value`, `error`, `code`).
- Prefer `__auto_type` (GCC/Clang) or explicit typed temporaries to avoid double-evaluation.

## Error Handling

- All fallible functions return `NurResult_<name>` — never raw errno or `-1`.
- Reserve `abort()` / `exit()` for explicit `NUR_UNWRAP` / `NUR_EXPECT` panic paths.
- Capture `errno` **immediately** after a system call before any other call can overwrite it.

## Code Style

- 4-space indentation; no tabs.
- Snake_case for types, functions, and variables.
- `SCREAMING_SNAKE_CASE` for macros and enum constants.
- Keep lines ≤ 100 characters.
- Every public API element must have a `/* ... */` documentation comment.