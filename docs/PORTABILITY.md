# Portability Guide for nur.h

## 1. Supported Compilers & Standards

| Compiler     | C89 | C99 | C11 | C17 | C23 | GNU Extensions |
|--------------|-----|-----|-----|-----|-----|----------------|
| GCC >= 10    | Yes | Yes | Yes | Yes | Yes | Full           |
| Clang >= 12  | Yes | Yes | Yes | Yes | Yes | Full           |
| MSVC 2022    | Yes | Yes | Yes | Yes | Partial | None       |
| MinGW-w64    | Yes | Yes | Yes | Yes | Yes | Full           |

---

## 2. GNU Extension Dependency Map

| Macro               | GNU Required | Non-GNU Behaviour              |
|---------------------|-------------|-------------------------------|
| `NUR_UNWRAP_OR`     | No          | Single-evaluation via `_Generic` (C11/MSVC) |
| `NUR_AND_THEN`      | No          | Functional chaining           |
| `NUR_MAP`           | YES         | Passthrough stub on non-GNU    |
| `NUR_MATCH`         | YES         | Use `NUR_MATCH_AS` on C99/MSVC |
| `NUR_TRY`           | YES         | Use `NUR_TRY_GOTO` on C99/MSVC |
| `NUR_TRY_GOTO`      | No          | Requires pre-assigned lvalue on non-GNU |

---

## 3. Critical: Double-Evaluation Protection on Non-GNU / C99

On older C99 compilers without statement expressions or `_Generic`, expression macros may evaluate arguments multiple times. Always pre-assign expressions with side effects to a named local variable:

```c
// RISKY on legacy C99 if f() has side effects:
int val = NUR_UNWRAP_OR(f(), 0);

// SAFE everywhere (GCC, Clang, MSVC, C99, C11, C23):
NurResult_i32 res = f();
int val = NUR_UNWRAP_OR(res, 0);
```

---

## 4. Leak-Free Error Propagation (`NUR_TRY_GOTO`)

### GNU Compiler (Statement Expression / `__auto_type`):
```c
NUR_TRY_GOTO(val, parse_config(path), err, cleanup);
```

### MSVC / Standard C Portable Pattern:
```c
NurResult_i32 res = parse_config(path);
NUR_TRY_GOTO(val, res, err, cleanup);
```

---

## 5. `NurResult_void` Layout Symmetry

For `NurResult_void`:
- In ISO C11 and MSVC, an anonymous union provides both `.error` and `.u.error` for full syntax symmetry.
- To access errors portably across all C versions, use `NUR_VOID_ERROR_OF(r)` or `r.error`.

---

## 6. Recommended Compiler Flags

### GCC / Clang (Modern C11)
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -g -Isrc main.c -o main
```

### GCC / Clang (Legacy ISO C89/C90)
```bash
gcc -std=c89 -Wall -Wextra -Wpedantic -Werror -g -Isrc main.c -o main
```

### MSVC
```bash
cl.exe /std:c11 /W4 /WX /Isrc main.c /Fe:main.exe
```

---

## 7. C-Purist & Zero-Hidden-Return Mode (`NUR_PURIST`)

For codebases with strict coding standards (such as MISRA-C or single-exit function rules) where macros with hidden `return` statements are forbidden:

1. Define `#define NUR_PURIST 1` before including `nur.h`.
2. All hidden early-return macros (`NUR_TRY*`, `NUR_CHECK*`, `NUR_RETURN_IF_ERR`) are strictly disabled at compile time.
3. Use the first-class pure-function API instead:
   - `nur_is_ok_<type>(res)` / `nur_is_err_<type>(res)`
   - `nur_value_<type>(res)` / `nur_error_<type>(res)`
   - `nur_result_is_ok(&res)` / `nur_result_is_err(&res)`
   - `NUR_TRY_GOTO(...)` for clean, single-point goto cleanups without early returns.

---

## 8. `musl` libc & Embedded Caveats

- `NUR_ENABLE_BACKTRACE` requires GNU `execinfo.h` (glibc only). Do not enable backtraces on `musl` (Alpine Linux) or bare-metal embedded systems.
- For embedded and freestanding targets, define `-DNUR_FREESTANDING` or `-DNUR_LITE` to eliminate all runtime dependencies.

---

## 9. Continuous Integration Matrix

```bash
make test-all   # Validates C89, C99, C11, C17, and C23 matrix
make test-asan  # AddressSanitizer and UndefinedBehaviorSanitizer
make analyze    # Static analysis (cppcheck)
```
