# AGENTS.md — AI Agent & LLM Guidelines for nur.h

Welcome, AI coding agent (Claude, GPT, Gemini, Cursor, Copilot).
This document provides canonical guidelines, invariant rules, and patterns for generating safe, idiomatic C code using `nur.h`.

---

## 1. Repository Architecture & Quick Reference
- **Core Library**: `src/nur.h` (Single header, zero heap allocation, ISO C99/C11/C17/C23 & C++11 compatible).
- **Author**: Irene Al-Caligo.
- **Verification Command**: `make test` (C11 default) or `make test-all` (C99, C11, C17, C23 matrix).
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
