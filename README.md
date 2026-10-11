# nur.h — Rust-Style Error Handling for C

A **single-header, zero-allocation** C library that brings Rust's `Result<T,E>` and
`Option<T>` patterns to ISO C, with compiler-assisted safety via `[[nodiscard]]`.

**Author**: Irene Al-Caligo | **License**: MIT | **C Standards**: C99, C11, C17, C23, C++11+

[![C Standard](https://img.shields.io/badge/C-C99%20%7C%20C11%20%7C%20C17%20%7C%20C23-blue)](src/nur.h)
[![Header Only](https://img.shields.io/badge/header--only-yes-green)](src/nur.h)
[![Zero Allocation](https://img.shields.io/badge/allocation-zero%20core-green)](src/nur.h)
[![CI Pipeline](https://img.shields.io/badge/CI-Passing-brightgreen)](.ci/ci.yml)

---

## Why nur.h?

C APIs communicate failure through at least four incompatible conventions
(`-1`, `NULL`, `errno`, out-parameters) — none of which the compiler forces
you to check. `nur.h` replaces all of them with a single, typed
`NurResult<T>` value that carries either a success payload or structured
error information, and uses `[[nodiscard]]`/`warn_unused_result` to make
ignoring the result a compiler warning.

```c
// Old C — sentinel, easy to accidentally ignore
int val = parse_port("99999");
// Did you check validity? Out of range? Error sentinel?

// nur.h — failure is part of the type, compiler enforces checking
NurResult_i32 r = parse_port("99999");
if (NUR_IS_ERR(r)) {
    nur_error_eprint(&NUR_ERROR_OF(r));   // [NUR_E_OUT_OF_RANGE] port out of range [99999] at net.c:42
    return 1;
}
int port = NUR_VALUE(r);
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
gcc --version   # gcc 11+ or later
```

### 2. Drop the header into your project

**PowerShell**:
```powershell
Copy-Item src\nur.h your_project\
```

**POSIX (Linux / macOS)**:
```bash
cp src/nur.h your_project/
# Or curl directly:
curl -fsSL https://raw.githubusercontent.com/IreneAl-Caligo/nur.h/master/src/nur.h -o your_project/nur.h
```

**CMake (FetchContent)**:
```cmake
include(FetchContent)
FetchContent_Declare(
    nur
    GIT_REPOSITORY https://github.com/IreneAl-Caligo/nur.h.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(nur)
target_link_libraries(my_app PRIVATE nur::nur)
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
        nur_error_eprint(&NUR_ERROR_OF(r));

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

## Error Propagation

### Auto-deduction (`NUR_TRY_AUTO`)
Works across all C standards (C99, C11, C17, C23) and deduces the source type automatically:

```c
NurResult_i32 compute(int a, int b, int c) {
    int step1, step2;
    NUR_TRY_AUTO(i32, step1, divide(a, b));
    NUR_TRY_AUTO(i32, step2, divide(step1, c));
    return NUR_OK(i32, step2);
}
```

### Leak-Free Resource Cleanup (`NUR_TRY_GOTO`)
Standard C cleanup pattern with zero resource leaks:

```c
NurResult_i32 read_data(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NUR_ERR(i32, NUR_ERROR_ERRNO("fopen failed"));

    int val = 0;
    nur_error_t err = {0};

    NUR_TRY_GOTO(val, parse_stream(f), err, cleanup);

cleanup:
    if (f) fclose(f);
    if (err.code != NUR_E_NONE) return nur_err_i32(err);
    return NUR_OK(i32, val);
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
char *data = (char *)NUR_UNWRAP(buf);
/* ... use data ... */
nur_free(data);
```

---

## errno Integration — `NUR_ERROR_ERRNO`

Capture `errno` immediately after a failing syscall. Automatically maps to semantic taxonomy (`ENOENT` -> `NUR_E_NOT_FOUND`, `EACCES` -> `NUR_E_PERMISSION`, etc.):

```c
FILE *f = fopen(path, "rb");
if (!f)
    return NUR_ERR(ptr, NUR_ERROR_ERRNO("fopen failed"));

// With context:
return NUR_ERR(ptr, NUR_ERROR_ERRNO_CTX("fopen failed", path));
```

The error struct carries both the semantic code in `.code` and the raw `errno` value in `.sys`.

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
NUR_RESULT_TYPE(Point, point);          /* generates NurResult_point */

Point p = {3, 4};
NurResult_point r = NUR_OK(point, p);
```

---

## Error Code Taxonomy (36 Semantic Codes)

| Range  | Category      | Count | Examples                                          |
|--------|---------------|-------|---------------------------------------------------|
| 1–9    | Validation    | 6     | `NUR_E_INVALID_ARGUMENT`, `NUR_E_NULL_POINTER`    |
| 10–19  | Data          | 6     | `NUR_E_PARSE`, `NUR_E_FORMAT`, `NUR_E_ENCODING`   |
| 20–29  | Resources     | 5     | `NUR_E_OUT_OF_MEMORY`, `NUR_E_RESOURCE_LIMIT`     |
| 30–39  | I/O           | 7     | `NUR_E_NOT_FOUND`, `NUR_E_PERMISSION`, `NUR_E_IO` |
| 40–49  | Network/Async | 6     | `NUR_E_TIMEOUT`, `NUR_E_CONNECTION`, `NUR_E_WOULD_BLOCK` |
| 50–59  | System        | 6     | `NUR_E_UNSUPPORTED`, `NUR_E_INTERNAL`, `NUR_E_BUG` |
| ≥ 1000 | User-defined  | -     | `NUR_E_USER_BASE + n`                             |

---

## Full API Reference

### Propagation & Safety
| Macro                                | Meaning                                          |
|--------------------------------------|--------------------------------------------------|
| `NUR_TRY_AUTO(D, lhs, expr)`         | Auto-typed propagate: assigns `lhs` or returns error |
| `NUR_TRY_TO(D, S, lhs, expr)`        | Explicitly typed propagate: assign or return error |
| `NUR_TRY_GOTO(lhs, expr, err, lbl)`  | Leak-free: assign `lhs` or store error and jump to `lbl` |
| `NUR_TRY_VOID_GOTO(expr, err, lbl)`  | For void results: jump to cleanup on failure     |
| `NUR_CHECK(cond, err)`               | Guard clause → returns `NurResult_void` on fail  |
| `NUR_CHECK_AS(D, cond, err)`         | Guard clause → returns `NurResult_D` on fail     |
| `NUR_ASSERT_OK(r)`                   | Test helper: asserts Ok or aborts with trace     |
| `NUR_ASSERT_ERR(r, code)`            | Test helper: asserts Err with specific code      |

### Utilities & Formatting
| Function / macro                          | Meaning                              |
|-------------------------------------------|--------------------------------------|
| `nur_error_format(err, buf, cap)`         | Single-line human readable format    |
| `nur_error_json(err, buf, cap)`           | RFC 8259 JSON format for observability |
| `nur_set_panic_handler(fn)`               | Overridable panic callback           |
| `nur_error_eprint(err)`                   | Print formatted error to `stderr`    |
| `nur_malloc(size)` → `NurResult_ptr`      | Safe allocation wrapper              |

---

## Enterprise Observability

### JSON Error Serialization
Zero-allocation formatting compliant with RFC 8259:
```c
char buf[256];
nur_error_json(&NUR_ERROR_OF(res), buf, sizeof(buf));
// {"ok":false,"code":30,"name":"NUR_E_NOT_FOUND","message":"file not found","context":"cfg.json","sys":2,"file":"main.c","func":"run","line":42}
```

### Configurable Panic Handler
By default, `NUR_UNWRAP` prints diagnostics and calls `abort()`. You can register a custom panic handler for logging, metrics, or graceful process termination:
```c
void my_panic_handler(const char *msg, const nur_error_t *e, const char *file, unsigned int line) {
    syslog(LOG_CRIT, "Panic at %s:%u - %s", file, line, msg ? msg : "");
    exit(EXIT_FAILURE);
}

nur_set_panic_handler(my_panic_handler);
```
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
