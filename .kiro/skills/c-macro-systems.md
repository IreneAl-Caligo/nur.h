---
name: c-macro-systems
description: Advanced C preprocessor and macro system expertise for nur.h development. Covers token pasting, variadic macros, compound literals, and safe multi-statement macro patterns.
---

# Skill: Advanced C Macro Systems

## Core Techniques

### Token Pasting (`##`)
Used in `NUR_RESULT_TYPE` and `DEFINE_NUR_RESULT` to generate unique type names:
```c
#define DEFINE_NUR_RESULT(type, name) \
    typedef struct { ... } NurResult_##name;
// NurResult_##i32 → NurResult_i32
```

### Compound Literals (C99+)
Used in `NUR_OK` / `NUR_ERR` for zero-overhead construction:
```c
#define NUR_OK(Type, val) (Type){ .is_ok = true, .data.value = (val) }
// Produces a temporary struct on the stack — no heap involved.
```

### Statement Expressions (`__extension__ ({ ... })`)
GCC/Clang extension used in `NUR_TRY` to return a value from a block:
```c
#define NUR_TRY(expr) __extension__ ({ \
    __auto_type _r = (expr); \
    if (!_r.ok) return nur_err_void(_r.u.error); \
    _r.u.value; \
})
```
Always prefix with `__extension__` to suppress `-Wpedantic` warnings.

### `do { ... } while (0)` Wrapper
Mandatory for multi-statement macros used as statements:
```c
#define NUR_TRY_TO(DestType, SrcType, lhs, expr) \
    do { \
        NurResult_##SrcType _r = (expr); \
        if (!_r.ok) return nur_err_##DestType(_r.u.error); \
        (lhs) = _r.u.value; \
    } while (0)
```

### `__auto_type` vs Explicit Type
- `__auto_type` (GCC/Clang): infers the type of the result, enabling `NUR_TRY` to work with any `NurResult_*` without specifying the type.
- Portable fallback: require the caller to name the type explicitly (`NUR_TRY_TO`).

## Common Pitfalls

| Pitfall | Example | Fix |
|---------|---------|-----|
| Double evaluation | `#define MAX(a,b) (a)>(b)?(a):(b)` with side effects | Use `__auto_type` temp |
| Missing parentheses | `#define SQ(x) x*x` | `#define SQ(x) ((x)*(x))` |
| Parameter name clash | param named `type` shadows C keyword in body | Use `T_`, `Name_` etc. |
| Missing `do/while(0)` | Multi-statement macro breaks `if` without braces | Always wrap |
| Stringification in token paste | `#x` inside `##` context | Separate into two macros |

## Variadic Macros (C99+)
```c
#define NUR_LOG(fmt, ...) fprintf(stderr, "[NUR] " fmt "\n", ##__VA_ARGS__)
```
Use `##__VA_ARGS__` (GCC extension) to handle zero extra arguments; portable C99 requires at least one.