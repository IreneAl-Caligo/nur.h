# An Open Letter to C Purists: Why nur.h is Pure C, Not Rust

**Target Audience**: Veteran C developers, Linux kernel contributors, systems architects, and MISRA-C reviewers.

---

## 1. The Skepticism: "Are you trying to turn C into Rust?"

When veteran C developers see `NurResult`, `NUR_TRY`, and `NUR_OK`, a natural and healthy skepticism arises:
> *"C is simple, explicit, and close to the metal. If I wanted Rust, I would write Rust. Stop polluting C with macro black-magic!"*

We wholeheartedly agree with the philosophy of C: **no hidden control flow, no compiler magic, no hidden allocations, and absolute transparency down to the assembly level.**

This document demystifies `nur.h` and demonstrates that it is **100% traditional, idiomatic ISO C** under the hood.

---

## 2. What `nur.h` Actually Is: The Classic Tagged Union

`nur.h` does not invent a new programming model. It standardizes a pattern that C programmers have used for over 40 years: **The Tagged Union (Sum Type)**.

Consider standard C patterns you already use every day:
- **X11 / Xlib**: `XEvent` is an `int type` followed by a union of 30+ event structures.
- **POSIX**: `siginfo_t` uses `si_code` tag with a union of signal details.
- **Compilers / Interpreters**: AST nodes always use `struct ASTNode { enum NodeType type; union { ... } data; };`.

`NurResult` is identical:
```c
/* What you write */
NurResult_i32 res;

/* What the compiler actually compiles */
typedef struct {
    bool ok;          /* 1-byte discriminant tag */
    /* 7 bytes padding on 64-bit */
    union {
        int32_t     value; /* active if ok == true */
        nur_error_t error; /* active if ok == false */
    } u;
} NurResult_i32;
```

There are:
- ❌ **NO C++ exceptions** (`throw`/`catch`)
- ❌ **NO `setjmp` / `longjmp`** jumps
- ❌ **NO hidden heap allocations** (`malloc` is never called)
- ❌ **NO stack unwinding metadata**
- ❌ **NO runtime type information (RTTI)**
- ❌ **NO vtables or indirect function pointers**

---

## 3. Disassembly: Zero Overhead Down to the Bare Metal

Consider a simple function returning a `NurResult_i32`:

```c
NurResult_i32 divide(int32_t a, int32_t b) {
    if (b == 0) return nur_err_i32(NUR_ERROR(NUR_E_DIVIDE_BY_ZERO, "zero divisor"));
    return nur_ok_i32(a / b);
}
```

On x86_64 GCC `-O2`, this compiles to standard register-based returns:
```nasm
divide:
    test   esi, esi
    je     .Lerror
    mov    eax, edi
    cdq
    idiv   esi
    ; Pack into registers: ok = 1 (in rax), value = result (in rdx)
    mov    DWORD PTR [rdi+8], eax
    mov    BYTE PTR [rdi], 1
    ret
.Lerror:
    ; Zero out and set ok = 0
    ...
```

It is identical in machine code efficiency to returning any two-field C struct.

---

## 4. Don't Like Macros? Don't Use Them! (`NUR_PURIST`)

If you prefer zero macros, you can turn `nur.h` into a **pure function library** by adding one line:

```c
#define NUR_PURIST 1
#include "nur_core.h"
```

In Purist Mode:
- Every macro that injects control flow (`NUR_TRY`, `NUR_CHECK`, `NUR_RETURN_IF_ERR`) is disabled at compile time.
- You interact **exclusively with standard inline C functions**:

```c
/* 100% Macro-Free, Pure Functional C */
NurResult_i32 res = read_sensor_id();

if (nur_is_ok_i32(res)) {
    int32_t id = nur_value_i32(res);
    process_id(id);
} else {
    nur_error_t err = nur_error_i32(res);
    log_failure(nur_error_code_of(&err));
}
```

---

## 5. Why `NurResult` is Superior to Legacy C Error Patterns

| Metric | Legacy `int` return + Out-Pointer (`int parse(const char*, int*)`) | Global `errno` (`double sqrt(double)`) | `nur.h` (`NurResult_T`) |
| :--- | :---: | :---: | :---: |
| **Compiler Enforces Error Check** | ❌ No (silent ignore) | ❌ No (easily forgotten) | ✅ **Yes** (`[[nodiscard]]`) |
| **Uninitialized Value Read** | ⚠️ High (if caller forgets check) | ⚠️ High (reads garbage) | 🛡️ **Impossible** (segregated in union) |
| **Thread Safety** | ✅ Yes | ⚠️ Requires TLS / thread-local | ✅ **100% Reentrant / Pure** |
| **Heap Overhead** | 0 bytes | 0 bytes | **0 bytes** |
| **Source Context (File/Line/Code)**| ❌ None | ❌ None | ✅ **Complete Origin Trace** |

---

## 6. Summary

`nur.h` is not an attempt to force Rust onto C. It is an **evolution of idiomatic C**, providing compile-time safety and self-documenting APIs using the language's native tagged union primitives.
