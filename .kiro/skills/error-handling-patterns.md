---
name: error-handling-patterns
description: Specialised knowledge for implementing and reviewing Result-based error handling in C. Covers the full nur.h API, common usage patterns, and migration from legacy C error conventions.
---

# Skill: Error Handling Patterns with nur.h

## Quick Reference — Full API

### Type Generation
```c
NUR_RESULT_TYPE(MyStruct, my_struct);   // → NurResult_my_struct
NUR_OPTION_TYPE(int, i32);              // → NurOption_i32 (already predefined)
```

### Constructing Results
```c
return NUR_OK(i32, 42);
return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "value must be positive"));
return NUR_OK_VOID();
return NUR_ERR_VOID(NUR_ERROR_ERRNO("fwrite failed"));
```

### Inspecting Results
```c
if (NUR_IS_OK(result))  { /* use result.u.value  */ }
if (NUR_IS_ERR(result)) { /* use result.u.error  */ }
```

### Value Extraction
```c
// Panic on error (use only in main / test harnesses)
int v = NUR_UNWRAP(result);
int v = NUR_EXPECT(result, "context message");

// Fallback (never panics)
int v = NUR_UNWRAP_OR(result, -1);
```

### Error Propagation
```c
// GCC/Clang — idiomatic
int x = NUR_TRY(parse_int(str));

// Portable
int x;
NUR_TRY_TO(i32, i32, x, parse_int(str));

// Guard clause
NUR_CHECK_AS(i32, ptr != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "ptr"));
```

## The 8 Cookbook Patterns

1. **Check-then-use** — `if (NUR_IS_OK(r)) { use(r.u.value); }`
2. **Guard-clause preconditions** — `NUR_CHECK_AS` at the top of every function
3. **`?` composition** — chain with `NUR_TRY` / `NUR_TRY_TO`
4. **Cross-type error mapping** — `NUR_TRY_AS(DestType, expr)` to re-wrap into a different result type
5. **Per-item loop recovery** — check `NUR_IS_ERR` inside a loop, log and continue
6. **Void results** — `NurResult_void` for side-effect operations (write, send, flush)
7. **Cleanup discipline** — goto-free: use `NurResult_void` with early return + RAII-style cleanup
8. **errno-shim adapter** — capture `errno` immediately, wrap in `NUR_ERROR_ERRNO`

## Migration Guide: Legacy C → nur.h

| Legacy pattern              | nur.h replacement                        |
|-----------------------------|------------------------------------------|
| `int rc = fn(); if (rc < 0)` | `NurResult_i32 r = fn(); if (NUR_IS_ERR(r))` |
| `ptr = malloc(n); if (!ptr)` | `NurResult_ptr r = nur_malloc(n); if (NUR_IS_ERR(r))` |
| `errno` after syscall       | `NUR_ERROR_ERRNO("msg")` immediately after call |
| `NULL` return from getter   | `NurOption_ptr` with `NUR_NONE(ptr)`     |
| Out-parameter `int *result` | Return `NurResult_i32` directly          |
| `goto cleanup` chains       | Early `return NUR_ERR(...)` + defer-style cleanup |

## Thread Safety Notes

- All `NurResult_*` objects are stack-allocated value types — inherently thread-safe.
- `NUR_ERROR_ERRNO` must be called on the **same thread** that made the system call, before any other syscall.
- Never store `nur_error_t` with a `message` pointer to a stack-allocated string that could go out of scope — use string literals.
