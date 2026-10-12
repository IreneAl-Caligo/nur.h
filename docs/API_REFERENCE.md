# API Reference — `nur.h`

Complete technical specification and quick-lookup reference for all macros, types, functions, and error codes in `nur.h`.

---

## 1. Result Types & Constructors

### `NUR_RESULT_TYPE(Type, suffix)`
Declares a new `NurResult_<suffix>` struct and companion constructors at file scope:
- `NurResult_<suffix>`: Struct containing `bool ok` and `union { Type value; nur_error_t error; } u`.
- `nur_ok_<suffix>(val)`: Constructs an Ok result.
- `nur_err_<suffix>(err)`: Constructs an Err result.
- `nur_unwrap_or_<suffix>(r, fallback)`: Inline fallback extractor (guaranteed single-evaluation).

### Predefined Result Types
| Type Name | Value Type | Constructor (Ok) | Constructor (Err) |
|---|---|---|---|
| `NurResult_i32` | `int32_t` | `NUR_OK(i32, val)` | `NUR_ERR(i32, err)` |
| `NurResult_i64` | `int64_t` | `NUR_OK(i64, val)` | `NUR_ERR(i64, err)` |
| `NurResult_usize`| `size_t` | `NUR_OK(usize, val)` | `NUR_ERR(usize, err)` |
| `NurResult_f32` | `float` | `NUR_OK(f32, val)` | `NUR_ERR(f32, err)` |
| `NurResult_f64` | `double` | `NUR_OK(f64, val)` | `NUR_ERR(f64, err)` |
| `NurResult_cstr` | `char *` | `NUR_OK(cstr, str)` | `NUR_ERR(cstr, err)` |
| `NurResult_ptr` | `void *` | `NUR_OK(ptr, ptr)` | `NUR_ERR(ptr, err)` |
| `NurResult_void` | `void` | `nur_ok_void()` | `nur_err_void(err)` |

---

## 2. Result Inspection & Value Extraction

| Macro | Description | Return / Action |
|---|---|---|
| `NUR_IS_OK(r)` | Returns true if result is Ok. | `bool` |
| `NUR_IS_ERR(r)` | Returns true if result is Err. | `bool` |
| `NUR_VALUE(r)` | Reads value payload. **MUST only be called if `NUR_IS_OK(r)` is true.** | Value |
| `NUR_ERROR_OF(r)` | Reads `nur_error_t` payload. | `nur_error_t` |
| `NUR_VOID_ERROR_OF(r)`| Reads error from `NurResult_void`. | `nur_error_t` |
| `NUR_OR_ELSE(r, fb)` | Returns value if Ok, otherwise fallback `fb`. | Value |
| `NUR_UNWRAP_OR(r, fb)`| Alias for `NUR_OR_ELSE`. Uses `_Generic` on C11/MSVC. | Value |
| `NUR_EXPECT(r, msg)` | Unwraps value; if Err, prints `msg` and diagnostic to stderr and aborts. | Value |
| `NUR_UNWRAP(r)` | Unwraps value; aborts with diagnostic on Err. (Use in tests/CLI only). | Value |
| `NUR_IGNORE(expr)` | Evaluates `expr` and explicitly silences unused result warnings. | `void` |

---

## 3. Result Combinators & Transforms

### `NUR_MAP(result, func)`
Transforms the value payload using `func(val)` if Ok; passes through Err unchanged.
```c
static int double_it(int v) { return v * 2; }
NurResult_i32 r2 = NUR_MAP(r, double_it);
```

### `NUR_AND_THEN(result, func)`
Chains execution: if Ok, calls `func(val)` which returns a new `NurResult_<same_suffix>`. If Err, passes error through.
```c
NurResult_i32 r2 = NUR_AND_THEN(r, parse_record);
```

---

## 4. Error Construction & Propagation

### Constructors
- `NUR_ERROR(code, message)`: Constructs `nur_error_t` capturing file, function, and line.
- `NUR_ERROR_CTX(code, message, context)`: Adds operation context string.
- `NUR_ERROR_ERRNO(message)`: Maps current POSIX `errno` to semantic code and captures `errno`.
- `NUR_ERROR_ERRNO_CTX(message, context)`: Maps `errno` with operation context.
- `NUR_ERROR_FROM_WIN32(win32_code, message)`: Maps Win32/Winsock error to semantic code.

### Error Chaining & Inspection
- `NUR_ERROR_WRAP(err, new_context)`: Adds new caller context while automatically retaining the previous context in `.cause`.
- `NUR_ERROR_CAUSE(err)`: Accesses the root cause string (`const char *`).
- `NUR_ERROR_CAUSED_BY(err, context, cause)`: Explicitly sets context and cause.
- `NUR_ERROR_IS(err, code)`: Tests if error matches code (`(err).code == code`).

### Control Flow & Guards
- `NUR_CHECK(cond, err)`: Returns `err` from enclosing function if `cond` is false.
- `NUR_CHECK_AS(DestSuffix, cond, err)`: Returns `nur_err_<DestSuffix>(err)` if `cond` is false.
- `NUR_CHECK_MSG(cond, code, msg)`: Precondition guard with explicit code and message.
- `NUR_RETURN_IF_ERR(expr)`: Early returns from `NurResult_void` function if `expr` failed.
- `NUR_TRY_GOTO(dest, expr, err_var, label)`: Leak-free resource propagation. Evaluates `expr`; if Err, binds `err_var` and jumps to `label`.

---

## 5. Error Serialization & Output (Zero Allocation)

- `nur_error_format(const nur_error_t *e, char *buf, size_t cap)`: Formats single-line human-readable summary. Returns required length (snprintf-style).
- `nur_error_json(const nur_error_t *e, char *buf, size_t cap)`: Serializes RFC 8259 JSON format into buffer.
- `nur_error_print(const nur_error_t *e, FILE *f)`: Prints formatted diagnostic to stream.
- `nur_error_eprint(const nur_error_t *e)`: Prints diagnostic to `stderr`.
- `nur_set_panic_handler(handler)`: Overrides panic handler for testing or logging integration.

---

## 6. Universal Type Engine (v1.2.0+)

| Macro | Description | Compatibility |
|---|---|---|
| `NUR_H(val)` | Single-argument universal constructor using C11 `_Generic`. Infers primitive type and returns matching `NurResult_<T>`. | C11, C17, C23, C++11 |
| `NUR_OK(val)` | 1-argument overload of `NUR_OK`, delegating to `NUR_H(val)`. | C11, C17, C23, C++11 |
| `NUR_OK(suffix, val)` | Legacy explicit 2-argument constructor (`nur_ok_##suffix(val)`). | C99, C11, C17, C23 |

---

## 7. Batteries-Included Extensions & Security Shield

### Safe Math
- `nur_add_i32(int32_t a, int32_t b)`: Safe 32-bit addition; returns `NurResult_i32` with `NUR_E_OVERFLOW` on overflow.
- `nur_sub_i32(int32_t a, int32_t b)`: Safe 32-bit subtraction; returns `NurResult_i32` with `NUR_E_OVERFLOW` on overflow/underflow.
- `nur_mul_i32(int32_t a, int32_t b)`: Safe 32-bit multiplication; returns `NurResult_i32` with `NUR_E_OVERFLOW` on overflow.
- `nur_div_i32(int32_t a, int32_t b)`: Safe 32-bit division; returns `NurResult_i32` with `NUR_E_INVALID_ARGUMENT` on divide-by-zero.

### Safe File I/O
- `nur_file_read_text(const char *path, char *buf, size_t buf_size)`: Zero-heap text file reader. Automatically closes file handle, null-terminates buffer, and returns `NurResult_cstr` with mapped system errors.

### Security Shield & CLI
- **Compile-Time Shield**: Intercepts banned functions (`gets()`) and common typos (`prinf()`) at compile-time with actionable diagnostic notes. Opt-out via `#define NUR_NO_SECURITY_SHIELD`.
- **Developer CLI (`tools/nur-cc`)**: Pre-flight syntax and typo checker with multi-language i18n support (`--lang=en`, `--lang=bn`) and `--fix` auto-correction.

