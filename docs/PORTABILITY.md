# Portability Guide for nur.h

## 1. Supported Compilers

| Compiler     | C99 | C11 | C17 | C23 | GNU Extensions |
|--------------|-----|-----|-----|-----|----------------|
| GCC >= 10    | Yes | Yes | Yes | Yes | Full           |
| Clang >= 12  | Yes | Yes | Yes | Yes | Full           |
| MSVC 2022    | Yes | Yes | Yes | Partial | None       |
| MinGW-w64    | Yes | Yes | Yes | Yes | Full           |

## 2. GNU Extension Dependency Map

| Macro               | GNU Required | Non-GNU Behaviour              |
|---------------------|-------------|-------------------------------|
| NUR_UNWRAP_OR       | No          | Double-eval warning            |
| NUR_AND_THEN        | No          | Double-eval warning            |
| NUR_MAP             | YES         | Passthrough stub on non-GNU    |
| NUR_MATCH           | YES         | Not defined on non-GNU         |
| NUR_TRY             | YES         | Not defined on non-GNU         |
| NUR_TRY_GOTO        | No          | Requires pre-assigned lvalue   |

## 3. Critical: Double-Evaluation on Non-GNU

On MSVC, macros expand 
 twice. Always pre-assign:

`c
// BAD on non-GNU: f() called twice
int val = NUR_UNWRAP_OR(f(), 0);

// GOOD: pre-assign first
NurResult_i32 _r = f();
int val = NUR_UNWRAP_OR(_r, 0);
`

## 4. Non-GNU NUR_TRY_GOTO

GNU:
`c
NUR_TRY_GOTO(val, parse_config(path), err, cleanup);  // OK
`

Non-GNU (pre-assign required):
`c
NurResult_i32 _r = parse_config(path);
NUR_TRY_GOTO(val, _r, err, cleanup);
`

## 5. NurResult_void Layout

Use NUR_VOID_ERROR_OF(r) not NUR_ERROR_OF(r) for NurResult_void.
The void result uses a flat .error field, not .u.error.

## 6. Recommended Flags

`ash
# GCC/Clang
gcc -std=c11 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined -I src

# MSVC
cl.exe /std:c11 /W4 /WX /I src your_file.c
`

## 7. musl libc Note

NUR_ENABLE_BACKTRACE requires execinfo.h (glibc only).
Do not define NUR_ENABLE_BACKTRACE when building against musl (Alpine Linux).

## 8. CI Matrix

`ash
make test-all   # C99/C11/C17/C23 matrix
make test-asan  # AddressSanitizer + UBSan
make analyze    # cppcheck
`
