---
description: Core design principles, architecture decisions, and invariants for the nur.h library. Always included so every agent understands the library's non-negotiable constraints.
inclusion: always
---

# nur.h Library Design Principles

## What nur.h Is

`nur.h` is a **single-file, header-only, zero-allocation** C library that brings Rust-style safe error handling to ISO C. Its flagship feature is `NurResult<T>`: a tagged-union type that forces callers to acknowledge failure before accessing a value.

## Non-Negotiable Invariants

1. **Single header.** Everything lives in `src/nur.h`. No `.c` file, no link step.
2. **Zero allocation in the core.** `nur.h` itself never calls `malloc`. Optional adapter functions (e.g., `nur_malloc`) wrap allocation and return a `NurResult`.
3. **No global mutable state.** All result objects are value types on the stack.
4. **Multi-standard.** Compile cleanly with `-std=c99`, `-std=c11`, `-std=c17`, and `-std=c23` without warnings.
5. **`[[nodiscard]]` / `__attribute__((warn_unused_result))`.** Every function returning `NurResult_*` must carry this attribute so the compiler warns when results are silently dropped.

## Architecture

```
nur.h
├── VERSION & CONFIG        (NUR_VERSION_*, feature detection macros)
├── ATTRIBUTES              (NUR_NODISCARD, NUR_INLINE, NUR_LIKELY/UNLIKELY)
├── ERROR SYSTEM            (nur_error_code_t enum, nur_error_t struct)
├── ERROR CONSTRUCTORS      (NUR_ERROR, NUR_ERROR_CTX, NUR_ERROR_ERRNO)
├── RESULT TYPE GENERATOR   (NUR_RESULT_TYPE macro + predefined types)
├── OPTION TYPE GENERATOR   (NUR_OPTION_TYPE macro + predefined types)
├── CONVENIENCE MACROS      (NUR_OK, NUR_ERR, NUR_IS_OK, NUR_UNWRAP, etc.)
├── ERROR PROPAGATION       (NUR_TRY, NUR_TRY_AS, NUR_TRY_TO, NUR_CHECK)
└── UTILITIES               (nur_error_code_name, nur_error_print)
```

## Predefined Result Types

| Type suffix | C type    |
|-------------|-----------|
| `i32`       | `int`     |
| `i64`       | `long`    |
| `usize`     | `size_t`  |
| `f32`       | `float`   |
| `f64`       | `double`  |
| `cstr`      | `char *`  |
| `ptr`       | `void *`  |
| `void`      | (no value)|

User-defined types: `NUR_RESULT_TYPE(MyStruct, my_struct)` at file scope.

## Error Code Taxonomy

- **1–9**: Input/validation (`NUR_E_INVALID_ARGUMENT`, `NUR_E_NULL_POINTER`, …)
- **10–19**: Data (`NUR_E_PARSE`, `NUR_E_FORMAT`, …)
- **20–29**: Resources (`NUR_E_OUT_OF_MEMORY`, …)
- **30–39**: I/O (`NUR_E_NOT_FOUND`, `NUR_E_IO`, …)
- **40–49**: Network/async (`NUR_E_TIMEOUT`, `NUR_E_CANCELLED`, …)
- **50–59**: System (`NUR_E_UNSUPPORTED`, `NUR_E_INTERNAL`, …)
- **≥1000**: User-defined (`NUR_E_USER_BASE + n`)

## What Agents Must Never Do

- Add a `.c` source file to the library itself.
- Introduce heap allocation inside `nur.h` without an `NUR_NO_ALLOC` guard.
- Change the `NurResult_*` struct layout (breaks ABI).
- Remove the `NUR_NODISCARD` attribute from any result-returning function.
- Use `goto` for error handling — that is precisely what `NUR_TRY` replaces.
- Rename `NUR_OK` / `NUR_ERR` / `NUR_TRY` — these are the stable public API.