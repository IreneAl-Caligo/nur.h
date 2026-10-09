# nur.h — Rust-Style Error Handling for C

A **single-header, zero-allocation** C library that brings Rust's `Result<T,E>` and
`Option<T>` patterns to ISO C, with compiler-assisted safety via `[[nodiscard]]`.

[![C Standard](https://img.shields.io/badge/C-C99%20%7C%20C11%20%7C%20C17%20%7C%20C23-blue)](src/nur.h)
[![Header Only](https://img.shields.io/badge/header--only-yes-green)](src/nur.h)
[![Zero Allocation](https://img.shields.io/badge/allocation-zero%20core-green)](src/nur.h)

---

## Why nur.h?

C APIs communicate failure through at least four incompatible conventions
(`-1`, `NULL`, `errno`, out-parameters) — none of which the compiler forces
you to check. `nur.h` replaces all of them with a single, typed
`NurResult<T>` value that carries either a success payload or structured
error information, and uses `[[nodiscard]]`/`warn_unused_result` to make
ignoring the result a compiler warning.

```c
// Old C — errno + sentinel, easy to ignore
int fd = open("config.toml", O_RDONLY);
// Did you check fd == -1? errno? Both?

// nur.h — failure is part of the type
NurResult_i32 fd = nur_open("config.toml");
if (NUR_IS_ERR(fd)) {
    nur_error_print(&fd.u.error, stderr);   // [NUR_E_NOT_FOUND] config.toml at open.c:12
    return 1;
}
```

---

## Quick Start

### 1. Install GCC (Windows / MSYS2)

```powershell
# Open an MSYS2 UCRT64 shell, then:
pacman -S mingw-w64-ucrt-x86_64-gcc
```

Add `C:\msys64\ucrt64\bin` to your `PATH`. Verify:

```powershell
gcc --version   # gcc (Rev5, Built by MSYS2 project) 13.x or later
```

### 2. Drop the header into your project

```powershell
Copy-Item src\nur.h  your_project\
```

### 3. Include and use

```c
#include "nur.h"

NurResult_i32 divide(int32_t a, int32_t b) {
    if (b == 0)
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "division by zero"));
    return NUR_OK(i32, a / b);
}

int main(void) {
    NurResult_i32 r = divide(10, 2);

    if (NUR_IS_OK(r))
        printf("result: %d\n", NUR_VALUE(r));       // result: 5
    else
        nur_error_print(&r.u.error, stderr);

    /* Fallback value — no branching needed */
    int32_t safe = NUR_UNWRAP_OR(divide(10, 0), -1); // -1
    return 0;
}
```

Compile:

```bash
gcc -std=c11 -Wall -Wextra -I. main.c -o main
```

---

## Error Propagation — `NUR_TRY_TO`

Rust's `?` operator in C:

```c
NurResult_i32 pipeline(int a, int b, int c) {
    int step1, step2;
    NUR_TRY_TO(i32, i32, step1, divide(a, b));   /* return early on error */
    NUR_TRY_TO(i32, i32, step2, divide(step1, c));
    return NUR_OK(i32, step2);
}
```

On GCC/Clang, `NUR_TRY_AS` works as an expression:

```c
NurResult_i32 pipeline(int a, int b, int c) {
    int s1 = NUR_TRY_AS(i32, divide(a, b));
    int s2 = NUR_TRY_AS(i32, divide(s1, c));
    return NUR_OK(i32, s2);
}
```

---

## Guard Clauses — `NUR_CHECK`

```c
NurResult_void validate_port(int port) {
    NUR_CHECK(port > 0 && port <= 65535,
              NUR_ERROR(NUR_E_OUT_OF_RANGE, "port out of range"));
    return NUR_OK_VOID();
}

NurResult_i32 parse_positive(int v) {
    NUR_CHECK_AS(i32, v > 0,
                 NUR_ERROR(NUR_E_INVALID_ARGUMENT, "must be positive"));
    return NUR_OK(i32, v);
}
```

---

## Safe Allocation — `nur_malloc`

```c
NurResult_ptr buf = nur_malloc(1024);
if (NUR_IS_ERR(buf)) {
    /* NUR_E_OUT_OF_MEMORY, never a silent NULL */
    return 1;
}
char *data = NUR_UNWRAP(buf);
/* ... use data ... */
free(data);
```

---

## errno Integration — `NUR_ERROR_ERRNO`

Capture `errno` immediately after a failing syscall:

```c
FILE *f = fopen(path, "rb");
if (!f)
    return NUR_ERR(ptr, NUR_ERROR_ERRNO("fopen failed"));

// With context:
return NUR_ERR(ptr, NUR_ERROR_ERRNO_CTX("fopen failed", path));
```

The error struct carries both `NUR_E_IO` (semantic code) and the raw
`errno` value in `.sys` for programmatic inspection.

---

## Predefined Result Types

| Type              | C type     | Constructor              |
|-------------------|------------|--------------------------|
| `NurResult_i32`   | `int32_t`  | `NUR_OK(i32, 42)`        |
| `NurResult_i64`   | `int64_t`  | `NUR_OK(i64, 1000000LL)` |
| `NurResult_usize` | `size_t`   | `NUR_OK(usize, n)`       |
| `NurResult_f32`   | `float`    | `NUR_OK(f32, 1.5f)`      |
| `NurResult_f64`   | `double`   | `NUR_OK(f64, 3.14)`      |
| `NurResult_cstr`  | `char *`   | `NUR_OK(cstr, "hello")`  |
| `NurResult_ptr`   | `void *`   | `NUR_OK(ptr, p)`         |
| `NurResult_void`  | (no value) | `NUR_OK_VOID()`          |

### Custom types

```c
typedef struct { int x, y; } Point;
NUR_RESULT_TYPE(Point, point)           /* generates NurResult_point */

Point p = {3, 4};
NurResult_point r = NUR_OK(point, p);
```

---

## Error Code Taxonomy

| Range  | Category      | Examples                                          |
|--------|---------------|---------------------------------------------------|
| 1–9    | Input         | `NUR_E_INVALID_ARGUMENT`, `NUR_E_NULL_POINTER`    |
| 10–19  | Data          | `NUR_E_PARSE`, `NUR_E_FORMAT`, `NUR_E_ENCODING`   |
| 20–29  | Resources     | `NUR_E_OUT_OF_MEMORY`, `NUR_E_RESOURCE_LIMIT`     |
| 30–39  | I/O           | `NUR_E_NOT_FOUND`, `NUR_E_PERMISSION`, `NUR_E_IO` |
| 40–49  | Network/Async | `NUR_E_TIMEOUT`, `NUR_E_CANCELLED`                |
| 50–59  | System        | `NUR_E_UNSUPPORTED`, `NUR_E_INTERNAL`             |
| ≥ 1000 | User-defined  | `NUR_E_USER_BASE + n`                             |

---

## Option Types

```c
NurOption_i32 find_index(const int *arr, size_t n, int target) {
    for (size_t i = 0; i < n; i++)
        if (arr[i] == target)
            return NUR_SOME(i32, (int32_t)i);
    return NUR_NONE(i32);
}

int arr[] = {10, 20, 30};
NurOption_i32 idx = find_index(arr, 3, 20);
int32_t pos = NUR_OPTION_UNWRAP_OR(idx, -1);   // 1
```

---

## Full API Reference

### Constructors
| Macro / function                      | Returns          | Notes                          |
|---------------------------------------|------------------|--------------------------------|
| `NUR_OK(suffix, val)`                 | `NurResult_T`    | Success with value             |
| `NUR_ERR(suffix, err)`                | `NurResult_T`    | Failure with `nur_error_t`     |
| `NUR_OK_VOID()`                       | `NurResult_void` | Success, no payload            |
| `NUR_ERR_VOID(err)`                   | `NurResult_void` | Failure, no payload            |
| `NUR_ERROR(code, msg)`                | `nur_error_t`    | Error with source location     |
| `NUR_ERROR_CTX(code, msg, ctx)`       | `nur_error_t`    | Error + context string         |
| `NUR_WRAP_ERROR(err, ctx)`            | `nur_error_t`    | Wrap/chain error with context  |
| `NUR_ERROR_ERRNO(msg)`                | `nur_error_t`    | Error from current `errno`     |
| `NUR_ERROR_ERRNO_CTX(msg, ctx)`       | `nur_error_t`    | Same + context string          |

### Inspection
| Macro                        | Meaning                                    |
|------------------------------|--------------------------------------------|
| `NUR_IS_OK(r)`               | `true` when result holds a value           |
| `NUR_IS_ERR(r)`              | `true` when result holds an error          |
| `NUR_VALUE(r)`               | Raw value access (check `NUR_IS_OK` first) |
| `NUR_ERROR_OF(r)`            | Raw error access                           |
| `NUR_UNWRAP(r)`              | Value or `abort()` with diagnostics        |
| `NUR_EXPECT(r, msg)`         | Value or `abort()` with custom message     |
| `NUR_UNWRAP_OR(r, fallback)` | Value or compile-time fallback             |

### Propagation
| Macro                              | Meaning                                          |
|------------------------------------|--------------------------------------------------|
| `NUR_TRY_TO(D, S, lhs, expr)`      | Portable: assign or return error as `D`          |
| `NUR_TRY_AS(D, expr)` *(GNU/Clang)*| Expression: unwrap or return error as `D`        |
| `NUR_TRY(expr)` *(GNU/Clang)*      | Expression: unwrap or return `NurResult_void`    |
| `NUR_TRY_VOID(S, expr)`            | Void return: propagate error from `S` result     |
| `NUR_CHECK(cond, err)`             | Guard clause → returns `NurResult_void` on fail  |
| `NUR_CHECK_AS(D, cond, err)`       | Guard clause → returns `NurResult_D` on fail     |
| `NUR_IGNORE(expr)`                 | Explicitly discard a result (no warning)         |

### Utilities
| Function / macro                          | Meaning                              |
|-------------------------------------------|--------------------------------------|
| `nur_error_code_name(code)`               | Static string name for error code    |
| `nur_error_print(err, file)`              | Print formatted error to `FILE *`    |
| `nur_error_eprint(err)`                   | Print to `stderr`                    |
| `nur_malloc(size)` → `NurResult_ptr`      | `malloc` wrapper, never silent NULL  |
| `nur_realloc(ptr, size)` → `NurResult_ptr`| `realloc` wrapper                    |

---

## Advanced Enterprise Features

### 1. Error Chaining & Context Wrapping
Use `NUR_WRAP_ERROR` to append new context as an error bubbles up the call stack, without losing the original code or source location.
```c
NurResult_i32 load_config() {
    NurResult_ptr file = nur_open("config.toml");
    if (NUR_IS_ERR(file)) {
        return NUR_ERR(i32, NUR_WRAP_ERROR(NUR_ERROR_OF(file), "failed to load config"));
    }
    // ...
}
```

### 2. Stacktraces (Opt-in)
Define `#define NUR_ENABLE_BACKTRACE` before including `nur.h` (on Linux or Windows) to automatically capture the native call stack whenever an error is instantiated. When `nur_error_print` is called, it will dump the full backtrace.

---

## IDE / LSP Support (Developer Experience)

C macros can sometimes confuse Language Servers (like `clangd`). 

1. **Auto-completion**: To ensure `clangd` perfectly understands `NurResult_T` types, generate a `compile_commands.json` for your project (e.g., using `bear -- make`). 
2. **VSCode Snippets**: This repository includes a `.vscode/nur.code-snippets` file. Just type `try` or `unwrap` in VSCode to instantly generate `NUR_TRY_AUTO(...)` or `NUR_UNWRAP(...)` guard clauses.

---

## Building & Testing

```powershell
# Run main test suite (C11)
gcc -std=c11 -Wall -Wextra -I. -Isrc test/test_nur.c -o test_nur.exe
.\test_nur.exe          # 16/16 PASS

# Run C23 feature tests
gcc -std=c23 -Wall -Wextra -I. -Isrc test/test_c23_features.c -o test_c23.exe
.\test_c23.exe          # 7/7 PASS

# Compile a basic example
gcc -std=c11 -Wall -I. -Isrc examples/basic/01_hello_result.c -o hello.exe
.\hello.exe
```

Via Makefile (Linux/macOS/MSYS2):

```bash
make test        # C11 suite
make test-all    # C99 / C11 / C17 / C23
make analyze     # cppcheck static analysis
```

---

## Design Principles

1. **Single header** — everything lives in `src/nur.h`; no `.c` file, no link step
2. **Zero allocation in core** — `nur.h` itself never calls `malloc`; allocation wrappers (`nur_malloc`) are opt-in
3. **No global mutable state** — all Result objects are value types on the stack
4. **Multi-standard** — compiles cleanly under `-std=c99` through `-std=c23`
5. **`[[nodiscard]]`** — every function returning `NurResult_*` carries the attribute; silently dropped results are a compiler warning

---

## Integration

Copy `src/nur.h` into your project tree and include it:

```c
#include "nur.h"
```

No build system changes. No linker flags. No dependencies beyond the C
standard library.

---

## License

MIT — see [LICENSE](LICENSE).

---

*"Errors should never pass silently."*
