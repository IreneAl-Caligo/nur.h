# AGENTS.md — AI Agent & LLM Guidelines for nur.h

Welcome, AI coding agent (Claude, GPT, Gemini, Cursor, Copilot).
This document provides canonical guidelines, invariant rules, and patterns for generating safe, idiomatic C code using `nur.h`.

---

## 1. Repository Architecture & Quick Reference
- **Core Library**: `src/nur.h` (Single header, zero heap allocation, ISO C89/C99/C11/C17/C23 & C++11 compatible).
- **Author**: Irene Al-Caligo.
- **Verification Command**: `make test` (C11 default) or `make test-all` (C89, C99, C11, C17, C23 matrix).
- **Sanitizers**: `make test-asan` (AddressSanitizer + UndefinedBehaviorSanitizer).

---

## 2. Invariants & Critical Safety Rules

### Rule 1: Always Check `NUR_IS_OK` Before `NUR_VALUE`
Never access `.u.value` or call `NUR_VALUE(r)` without verifying `NUR_IS_OK(r)`.
```c
// CORRECT
NurResult_i32 r = divide(10, 2);
if (NUR_IS_OK(r)) {
    int val = NUR_VALUE(r);
    // use val
} else {
    nur_error_eprint(&NUR_ERROR_OF(r));
}

// FORBIDDEN (Undefined Behavior on error)
int val = NUR_VALUE(r);
```

### Rule 2: Memory Lifetime of `nur_error_t`
The `.message`, `.context`, `.file`, and `.func` pointers in `nur_error_t` are **borrowed references**.
- **DO**: Pass string literals (static duration).
- **DO NOT**: Pass pointers to local stack buffers (e.g., from `snprintf` in a function). When the function returns, the pointer becomes dangling (use-after-free).
- If dynamic strings are required, allocate them in the caller or use static lifetime buffers.

### Rule 3: Error Propagation & Early Return
`NUR_TRY_TO` and `NUR_TRY_AUTO` execute an **early `return`** from the enclosing function on error.
- If your function allocated heap memory, file handles, or mutex locks before the macro call, an early return will **leak resources**.
- For functions needing resource cleanup, use `NUR_TRY_GOTO`:

```c
// Leak-free cleanup pattern
FILE *f = fopen(path, "r");
int val;
nur_error_t err = {0};

NUR_TRY_GOTO(val, parse_file(f), err, cleanup);

// Success path
printf("Val: %d\n", val);

cleanup:
if (f) fclose(f);
if (err.code != NUR_E_NONE) return nur_err_i32(err);
```

### Rule 4: `NUR_RESULT_TYPE` Placement
- Declare `NUR_RESULT_TYPE(Type, suffix);` once in a **shared header file** at file scope.
- Do NOT generate duplicate result types in multiple `.c` files, which leads to type redeclaration warnings.
- Always append a semicolon `;` at the end for compatibility with formatters and standard C.

---

## 3. Canonical Patterns

### Pattern A: Basic Result Function
```c
#include "nur.h"

NUR_NODISCARD NurResult_i32 parse_port(const char *str) {
    if (str == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "port string is NULL"));
    }
    int port = atoi(str);
    if (port <= 0 || port > 65535) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, "port out of range", str));
    }
    return NUR_OK(i32, port);
}
```

### Pattern B: Void Result Function
```c
NUR_NODISCARD NurResult_void validate_config(const Config *cfg) {
    NUR_CHECK(cfg != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "config is NULL"));
    NUR_CHECK(cfg->timeout_ms > 0, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "timeout must be positive"));
    return NUR_OK_VOID();
}
```

### Pattern C: Non-GNU / MSVC Portable Expression
On MSVC, expression macros like `NUR_UNWRAP_OR` require an lvalue to avoid double-evaluating side effects:
```c
// Best portable practice on all compilers:
NurResult_i32 res = compute_value();
int val = NUR_UNWRAP_OR(res, 0);
```

---

## 4. Testing & Verification Checklist for AI Agents
Before submitting any code change:
1. Run `make test` — verify all tests pass without errors.
2. Run `make test-all` — verify all C standards (C99, C11, C17, C23) compile cleanly.
3. Check `-Wall -Wextra -Wpedantic` — zero warnings allowed.
4. Ensure no `abort()` is placed in public library paths without configurable override.

---

## 5. Modern Universal Syntax (v1.2.0+) & Compile-Time Safety
- **Universal Single-Token Constructor**: In C11+, AI agents can write `return NUR_H(val);` or `return NUR_OK(val);`. The C11 `_Generic` engine automatically determines the target Result type (`i32`, `i64`, `usize`, `f32`, `f64`, `cstr`, `ptr`, `bool`).
- **Legacy Backward Compatibility**: 2-argument `NUR_OK(suffix, val)` is strictly supported across all standards (C89/C99/C11/C17/C23).
- **Batteries-Included Extensions**: Use `nur_add_i32()`, `nur_sub_i32()`, `nur_mul_i32()`, `nur_div_i32()` for overflow-safe arithmetic, and `nur_file_read_text()` for zero-leak buffer reading.
- **Security Shield**: `nur.h` blocks obsolete/vulnerable functions (e.g., `gets()`) and common typos (e.g., `prinf()`). Never bypass the security shield in production code.

---

## 6. ISO C89/C90 Support & C-Purist Pure-Function Mode (v1.3.0+)
- **ISO C89 / ANSI C Compatibility**: `nur.h` compiles with zero warnings under `-std=c89 -Wall -Wextra -Wpedantic`. C89 environments automatically fall back to standard-compliant inlining, type mappings (`bool`, `true`, `false`), and function location tracking.
- **C-Purist Safety Mode (`NUR_PURIST` / `NUR_NO_HIDDEN_RETURNS`)**:
  - Define `#define NUR_PURIST 1` (or `#define NUR_NO_HIDDEN_RETURNS 1`) before including `nur.h`.
  - Disables all macros that inject hidden early `return` statements (`NUR_TRY*`, `NUR_CHECK*`, `NUR_RETURN_IF_ERR`), preventing hidden exits and resource leaks.
- **Pure-Function Macro-Free API**:
  - For checking results without macros: `nur_is_ok_<type>(res)`, `nur_is_err_<type>(res)`.
  - For extracting values/errors without macros: `nur_value_<type>(res)`, `nur_error_<type>(res)`, `nur_unwrap_or_<type>(res, default_val)`.
  - Universal pointer predicates across any Result type: `nur_result_is_ok(&res)`, `nur_result_is_err(&res)`.
  - Error inspection: `nur_error_code_of(&err)`, `nur_error_message_of(&err)`.

---

## 7. Opt-In Safety & High-Performance Domain Patterns

`nur.h` operates on an **Opt-In Safety Philosophy**. Developers and AI agents are NOT required to force `NurResult` onto every line of C code. `nur.h` is a batteries-included super-header (bringing `<stdio.h>`, `<string.h>`, `<stdlib.h>`, `<stdbool.h>`, `<stdint.h>`, and `<errno.h>` out of the box), allowing bare-metal performance where failure is impossible:

### 1. Pure Numeric Arithmetic (Linear Algebra & Trigonometry)
- Use native standard C primitives (`float`, `double`, `sin()`, `cos()`, `sqrt()`) directly with `#include "nur.h"`. No extra includes or boilerplate wrappers required.
- **Built-in Safe Helper**: If input domain protection is desired, use `nur_sqrt(x)` which returns `NurResult_f64` and returns `NUR_E_INVALID_ARGUMENT` on negative numbers instead of producing `NaN`.

### 2. Tight Inner Loops (DSP, Audio, Matrix Multiplication)
- Do not check errors inside hot per-cycle inner loops.
- Apply the **Validate-at-Entry** pattern: check buffer pointers and bounds once at the function boundary using `NUR_CHECK(...)`, then execute the inner loop at 100% bare-metal speed.

### 3. Trivial Getters & Setters
- Direct struct member access (`return obj->field;`) or scalar setters.
- For optional fields, use zero-overhead `NurOption` (`NUR_SOME`, `NUR_NONE`).

### 4. Hardware Interrupt Service Routines (ISRs) & Bare Metal
- `nur.h` guarantees **zero heap allocations** (`malloc`/`free` are never called internally).
- In embedded environments with `#define NUR_FREESTANDING 1`, standard headers are omitted, and ISRs can execute bare register reads/writes with zero cycle latency.
