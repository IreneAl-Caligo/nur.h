# MISRA-C & Safety-Critical Zero-Macro Compliance Guide

**Target Standards**: MISRA C:2012 (Amendments 1 & 2), MISRA C:2023, ISO 26262 (ASIL D), DO-178C (DAL A), IEC 61508  
**Header Targets**: [src/nur.h](file:///src/nur.h), [src/nur_core.h](file:///src/nur_core.h)  

---

## 1. Executive Summary

In automotive, aerospace, railway, and medical embedded systems, coding standards (such as **MISRA C:2012** and **MISRA C:2023**) impose rigorous constraints on the C preprocessor, function return paths, and dynamic memory allocation.

`nur.h` and its minimal counterpart `nur_core.h` provide a **100% pure-function, zero-macro compliance mode** that satisfies the most stringent static analysis linters (Coverity, Polyspace, Axivion, PC-lint Plus, Clang-Tidy).

---

## 2. Enforcing Zero-Macro Mode (`NUR_PURIST`)

To disable all convenience macros that inject hidden control flow (`return` statements), define `NUR_PURIST` or `NUR_NO_HIDDEN_RETURNS` before including the header:

```c
#define NUR_PURIST 1
#include "nur.h" /* or "nur_core.h" */
```

### Disabled Macros in Purist Mode:
- ❌ `NUR_TRY(expr)`
- ❌ `NUR_TRY_TO(var, expr)`
- ❌ `NUR_TRY_AUTO(var, expr)`
- ❌ `NUR_TRY_GOTO(var, expr, err_out, label)`
- ❌ `NUR_CHECK(cond, err)`
- ❌ `NUR_RETURN_IF_ERR(res)`

Any attempt to use these macros in purist mode results in a compile-time `#error` explaining that macro-injected returns are disallowed.

---

## 3. Pure-Function API Reference

In zero-macro mode, developers interact exclusively with standard inline C functions:

### 3.1 Constructors
```c
/* Pure inline function constructors */
NurResult_i32  r1 = nur_ok_i32(42);
NurResult_i32  r2 = nur_err_i32(nur_internal_make_error(NUR_E_OUT_OF_RANGE, 0, "out of bounds", NULL, __FILE__, __func__, __LINE__));
NurResult_void v1 = nur_ok_void();
```

### 3.2 Result Inspection (Zero Macros)
```c
/* Standard boolean predicates (MISRA C Rule 14.4 compliant) */
if (nur_is_ok_i32(r1)) {
    int32_t val = nur_value_i32(r1);
    /* Safe processing */
} else {
    nur_error_t err = nur_error_i32(r1);
    nur_error_code_t code = nur_error_code_of(&err);
    const char *msg = nur_error_message_of(&err);
}

/* Universal pointer predicates (Any Result type) */
if (nur_result_is_ok(&r1)) {
    /* ... */
}
```

### 3.3 Safe Extraction with Fallbacks
```c
/* Pure function with explicit fallback, no branching macro */
int32_t safe_val = nur_unwrap_or_i32(r1, -1);
```

---

## 4. MISRA C:2012 / 2023 Rule Compliance Matrix

| Rule | Rule Requirement | How `nur.h` Complies in Purist Mode |
| :--- | :--- | :--- |
| **Rule 14.4** | The controlling expression of an `if` statement and the operand of `!` shall have essentially Boolean type | `nur_is_ok_*()` and `nur_is_err_*()` strictly return `bool` (`<stdbool.h>`). |
| **Rule 15.5** | A function should have a single point of exit at the end | `NUR_PURIST 1` eliminates all macros with early `return`. Developers retain full control over explicit single-exit logic. |
| **Rule 20.10** | The `#` and `##` preprocessor operators should not be used | Developers utilize predefined types (`NurResult_i32`, `NurResult_usize`, etc.) and pre-generated pure functions. No runtime or user macro expansion is performed. |
| **Rule 21.3** | Dynamic memory allocation functions shall not be used (`malloc`, `free`, `realloc`) | **Zero dynamic heap allocation**. All structures exist exclusively on the call stack or in static storage. `malloc` is never called. |
| **Rule 21.6** | Standard I/O functions shall not be used (`<stdio.h>`) | Using `nur_core.h` or `#define NUR_NO_STDIO 1` / `#define NUR_FREESTANDING 1` completely decouples the library from `<stdio.h>`. |

---

## 5. Canonical MISRA-C Safe Function Pattern

Below is an exemplar DO-178C / MISRA-C compliant function demonstrating single-point of exit and 100% macro-free error handling:

```c
#define NUR_PURIST 1
#include "nur_core.h"

NurResult_i32 calculate_sensor_velocity(int32_t raw_delta, int32_t time_delta_ms) {
    NurResult_i32 ret_val;

    if (time_delta_ms <= 0) {
        nur_error_t err = nur_internal_make_error(
            NUR_E_INVALID_ARGUMENT, 0,
            "time delta must be strictly positive",
            NULL, __FILE__, __func__, __LINE__
        );
        ret_val = nur_err_i32(err);
    } else {
        int32_t velocity = raw_delta / time_delta_ms;
        ret_val = nur_ok_i32(velocity);
    }

    return ret_val; /* Single point of exit compliant with Rule 15.5 */
}
```
