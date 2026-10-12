# Rust to nur.h Rosetta Stone & Cheat Sheet

For developers coming from **Rust** (and LLMs trained heavily on Rust Result/Option semantics), this reference provides a 1-to-1 mapping between Rust paradigms and `nur.h` idioms in C99/C11/C23.

---

## 1. Core Idiom Mapping Table

| Rust Pattern | `nur.h` C Equivalent | Notes / Safety Semantics |
|---|---|---|
| `Result<i32, E>` | `NurResult_i32` | Zero heap allocation, packed value or error |
| `Option<T>` | `NurOption_i32`, `NurOption_ptr` | Explicit presence check, no `NULL` sentinel ambiguity |
| `Ok(42)` | `NUR_OK(i32, 42)` or `NUR_H(42)` | Universal generic constructor in C11+ |
| `Err(e)` | `NUR_ERR(i32, err)` | Typed error structure with file, line, code |
| `Some(x)` | `NUR_SOME(i32, x)` | Returns present option |
| `None` | `NUR_NONE(i32)` | Returns empty option |
| `r.is_ok()` | `NUR_IS_OK(r)` | Returns true if Ok |
| `r.is_err()` | `NUR_IS_ERR(r)` | Returns true if Err |
| `r.unwrap()` | `NUR_UNWRAP(r)` | Aborts with file, line & diagnostic message if Err |
| `r.expect("msg")` | `NUR_EXPECT(r, "msg")` | Aborts with custom message & context |
| `r.unwrap_or(def)` | `NUR_UNWRAP_OR(r, def)` | Safe fallback; evaluates arguments cleanly |
| `r?` (standard) | `NUR_TRY_AUTO(T, val, expr)` | Early return on Err from enclosing function |
| `r?` (with cleanup) | `NUR_TRY_GOTO(val, expr, err, lbl)` | Jumps to cleanup label (avoids memory leaks) |
| `r.map(fn)` | `NUR_MAP(r, fn)` | Applies transform function to Ok payload |
| `r.and_then(fn)` | `NUR_AND_THEN(r, fn)` | Monadic bind (chaining fallible operations) |
| `#[must_use]` | `NUR_NODISCARD` / `NUR_FN` | Triggers compiler warning/error if ignored |
| `?` with context | `NUR_WRAP_ERROR(r, "layer")` | Breadcrumb context chaining (up to 4 depths) |

---

## 2. Side-by-Side Code Examples

### A. Fallible Function Definition

#### Rust
```rust
fn divide(a: i32, b: i32) -> Result<i32, &'static str> {
    if b == 0 {
        return Err("division by zero");
    }
    Ok(a / b)
}
```

#### `nur.h` (C)
```c
NUR_NODISCARD NurResult_i32 divide(int a, int b) {
    if (b == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "division by zero"));
    }
    return NUR_OK(i32, a / b);
}
```

---

### B. Error Propagation (`?` Operator)

#### Rust
```rust
fn process() -> Result<i32, Error> {
    let fd = open_socket()?;
    let data = read_data(fd)?;
    Ok(data)
}
```

#### `nur.h` (C) — With Resource Cleanup Guard
```c
NurResult_i32 process(void) {
    int fd;
    nur_error_t err = {0};

    NUR_TRY_GOTO(fd, open_socket(), err, cleanup);
    
    int data;
    NUR_TRY_GOTO(data, read_data(fd), err, cleanup);

    close_socket(fd);
    return NUR_OK(i32, data);

cleanup:
    if (fd >= 0) close_socket(fd);
    if (err.code != NUR_E_NONE) return nur_err_i32(err);
    return NUR_OK(i32, data);
}
```

---

### C. Functional Transformation (`map` and `unwrap_or`)

#### Rust
```rust
let val = parse_int("42").map(|x| x * 2).unwrap_or(0);
```

#### `nur.h` (C)
```c
static int double_val(int x) { return x * 2; }

NurResult_i32 r = parse_int("42");
NurResult_i32 doubled = NUR_MAP(r, double_val);
int val = NUR_UNWRAP_OR(doubled, 0);
```

---

## 3. Key Differences to Remember in C

1. **Borrow Lifetime**: `nur.h` does not allocate memory on the heap. Error strings (`message`, `context`) must be static string literals or owned by the caller.
2. **Deterministic Cleanup**: C has no RAII drop glue. When holding allocated resources, always use `NUR_TRY_GOTO` instead of plain `NUR_TRY_TO` / `NUR_TRY_AUTO`.
3. **Types are Statically Declared**: Predefined types exist for `i32`, `i64`, `usize`, `f32`, `f64`, `cstr`, `ptr`, `bool`, and `void`. For custom structs:
   ```c
   NUR_RESULT_TYPE(MyStruct, mystruct);
   ```
