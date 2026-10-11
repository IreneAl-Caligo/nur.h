# nur.h API Reference

Comprehensive, auto-generated API documentation for `nur.h` (Author: Irene Al-Caligo).

## Macros & Propagation

### `NUR_ASSERT_ERR`

```c
#define NUR_ASSERT_ERR(r, expected_code)
```

Asserts that result is Err with expected error code.

---

### `NUR_ASSERT_OK`

```c
#define NUR_ASSERT_OK(r)
```

Asserts that result is Ok. Aborts with diagnostic if error.

---

### `NUR_AUTO`

```c
#define NUR_AUTO auto
```

---

### `NUR_AUTO`

```c
#define NUR_AUTO auto
```

---

### `NUR_AUTO`

```c
#define NUR_AUTO __auto_type
```

---

### `NUR_C11`

```c
#define NUR_C11 1
```

---

### `NUR_C11`

```c
#define NUR_C11 1
```

---

### `NUR_C11`

```c
#define NUR_C11 1
```

---

### `NUR_C17`

```c
#define NUR_C17 1
```

---

### `NUR_C17`

```c
#define NUR_C17 1
```

---

### `NUR_C23`

```c
#define NUR_C23 1
```

---

### `NUR_C99`

```c
#define NUR_C99 1
```

---

### `NUR_C99`

```c
#define NUR_C99 1
```

---

### `NUR_C99`

```c
#define NUR_C99 1
```

---

### `NUR_C99`

```c
#define NUR_C99 1
```

---

### `NUR_CHECK`

```c
#define NUR_CHECK(cond, err)
```

---

### `NUR_CHECK_AS`

```c
#define NUR_CHECK_AS(DestSuffix, cond, err)
```

Return typed result error if condition is false.

Usage:
NUR_CHECK_AS(i32, size > 0, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "size must be positive"));

---

### `NUR_COLD`

```c
#define NUR_COLD __attribute__((cold))
```

---

### `NUR_COLD`

```c
#define NUR_COLD
```

---

### `NUR_CUSTOM_PANIC_HANDLER_STORAGE`

```c
#define NUR_CUSTOM_PANIC_HANDLER_STORAGE static
```

---

### `NUR_ERR`

```c
#define NUR_ERR(suffix, err) nur_err_##suffix(err)
```

---

### `NUR_ERROR_ERRNO`

```c
#define NUR_ERROR_ERRNO(msg)
```

---

### `NUR_ERROR_OF`

```c
#define NUR_ERROR_OF(r) ((r).u.error)
```

---

### `NUR_ERR_VOID`

```c
#define NUR_ERR_VOID(err) nur_err_void(err)
```

---

### `NUR_EXPECT`

```c
#define NUR_EXPECT(r, msg)
```

---

### `NUR_E_PERMISSION_DENIED`

```c
#define NUR_E_PERMISSION_DENIED NUR_E_PERMISSION
```

Backward-compatibility and POSIX taxonomy alias

---

### `NUR_GNU_EXTENSIONS`

```c
#define NUR_GNU_EXTENSIONS 1
```

---

### `NUR_IF_ERR`

```c
#define NUR_IF_ERR(r, err_var, body)
```

Branch on error and bind error struct into err_var.

Usage:
NUR_IF_ERR(res, err, {
nur_error_eprint(&err);
});

---

### `NUR_IGNORE`

```c
#define NUR_IGNORE(expr)
```

Explicitly ignore a result - documents intent and suppresses compiler warnings.

Usage: NUR_IGNORE(write_log(msg));

---

### `NUR_IGNORE`

```c
#define NUR_IGNORE(expr) ((void)(expr))
```

Explicitly ignore a result - documents intent and suppresses warnings.

Usage: NUR_IGNORE(write_log(msg));

---

### `NUR_INLINE`

```c
#define NUR_INLINE static inline
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static __forceinline
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static inline
```

---

### `NUR_IS_ERR`

```c
#define NUR_IS_ERR(r) (!(r).ok)
```

---

### `NUR_IS_NONE`

```c
#define NUR_IS_NONE(o) (!(o).some)
```

---

### `NUR_IS_OK`

```c
#define NUR_IS_OK(r) ((r).ok)
```

Status checking

---

### `NUR_IS_SOME`

```c
#define NUR_IS_SOME(o) ((o).some)
```

---

### `NUR_LIKELY`

```c
#define NUR_LIKELY(x) __builtin_expect(!!(x), 1)
```

---

### `NUR_LIKELY`

```c
#define NUR_LIKELY(x) (x)
```

---

### `NUR_MATCH`

```c
#define NUR_MATCH(r, val_var, ok_body, err_var, err_body)
```

Type-inferring pattern match construct (GNU/C23).

Usage:
NUR_MATCH(res, val, { printf("Val: %d\n", val); }, err, { nur_error_eprint(&err); });

---

### `NUR_MATCH_AS`

```c
#define NUR_MATCH_AS(Type, r, val_var, ok_body, err_var, err_body)
```

Pattern match style construct for typed results with explicit type (C99 compatible).

Usage:
NUR_MATCH_AS(int, res, val, { printf("Val: %d\n", val); }, err, { nur_error_eprint(&err); });

---

### `NUR_MSVC`

```c
#define NUR_MSVC 1
```

---

### `NUR_NODISCARD`

```c
#define NUR_NODISCARD [[nodiscard]]
```

---

### `NUR_NODISCARD`

```c
#define NUR_NODISCARD __attribute__((warn_unused_result))
```

---

### `NUR_NODISCARD`

```c
#define NUR_NODISCARD _Check_return_
```

---

### `NUR_NODISCARD`

```c
#define NUR_NODISCARD
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg) __attribute__((warn_unused_result))
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg) _Check_return_
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg)
```

---

### `NUR_NODISCARD_TYPE`

```c
#define NUR_NODISCARD_TYPE [[nodiscard]]
```

---

### `NUR_NODISCARD_TYPE`

```c
#define NUR_NODISCARD_TYPE
```

---

### `NUR_NODISCARD_TYPE`

```c
#define NUR_NODISCARD_TYPE
```

---

### `NUR_NODISCARD_TYPE`

```c
#define NUR_NODISCARD_TYPE
```

---

### `NUR_NOINLINE`

```c
#define NUR_NOINLINE __attribute__((noinline))
```

---

### `NUR_NOINLINE`

```c
#define NUR_NOINLINE __declspec(noinline)
```

---

### `NUR_NOINLINE`

```c
#define NUR_NOINLINE
```

---

### `NUR_NONE`

```c
#define NUR_NONE(suffix) nur_none_##suffix()
```

---

### `NUR_NORETURN`

```c
#define NUR_NORETURN _Noreturn
```

---

### `NUR_NORETURN`

```c
#define NUR_NORETURN __attribute__((noreturn))
```

---

### `NUR_NORETURN`

```c
#define NUR_NORETURN __declspec(noreturn)
```

---

### `NUR_NORETURN`

```c
#define NUR_NORETURN
```

---

### `NUR_OK`

```c
#define NUR_OK(suffix, val) nur_ok_##suffix(val)
```

---

### `NUR_OK_VOID`

```c
#define NUR_OK_VOID() nur_ok_void()
```

---

### `NUR_OPTION_TYPE`

```c
#define NUR_OPTION_TYPE(T, suffix)
```

---

### `NUR_OPTION_UNWRAP_OR`

```c
#define NUR_OPTION_UNWRAP_OR(o, fallback)
```

Get option value or fallback if None (evaluates o exactly once).

Usage: int val = NUR_OPTION_UNWRAP_OR(opt, 0);

---

### `NUR_OPTION_UNWRAP_OR`

```c
#define NUR_OPTION_UNWRAP_OR(o, fallback)
```

Get option value or fallback if None.

Usage: int val = NUR_OPTION_UNWRAP_OR(opt, 0);

---

### `NUR_OPTION_VALUE`

```c
#define NUR_OPTION_VALUE(o) ((o).value)
```

---

### `NUR_OR_ELSE`

```c
#define NUR_OR_ELSE(r, fallback) NUR_UNWRAP_OR((r), (fallback))
```

Semantic alias for NUR_UNWRAP_OR

---

### `NUR_PRINTF_LIKE`

```c
#define NUR_PRINTF_LIKE(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
```

---

### `NUR_PRINTF_LIKE`

```c
#define NUR_PRINTF_LIKE(fmt_idx, va_idx)
```

---

### `NUR_PRINT_BUFFER_SIZE`

```c
#define NUR_PRINT_BUFFER_SIZE 1024
```

---

### `NUR_REQUIRE_OK`

```c
#define NUR_REQUIRE_OK(r)
```

Asserts that result is Ok, printing full error details to stderr before aborting.

Usage:
NUR_REQUIRE_OK(res);

---

### `NUR_REQUIRE_OK_MSG`

```c
#define NUR_REQUIRE_OK_MSG(r, msg)
```

Asserts that result is Ok with custom failure message, printing full error details before aborting.

Usage:
NUR_REQUIRE_OK_MSG(res, "Configuration loading failed");

---

### `NUR_RESULT_TYPE`

```c
#define NUR_RESULT_TYPE(T, suffix)
```

---

### `NUR_SOME`

```c
#define NUR_SOME(suffix, val) nur_some_##suffix(val)
```

Option constructors

---

### `NUR_TRY`

```c
#define NUR_TRY(expr)
```

Statement expression version of NUR_TRY (GNU extension).
Evaluates to the unwrapped value on success.
Enclosing function must return NurResult_void.

Usage:
int value = NUR_TRY(parse_number(str));
// value contains the number, or function returned error

---

### `NUR_TRY_AS`

```c
#define NUR_TRY_AS(DestSuffix, expr)
```

Propagate into a specific return type (GNU extension).

Usage:
int value = NUR_TRY_AS(i32, parse_number(str));

---

### `NUR_TRY_AUTO`

```c
#define NUR_TRY_AUTO(DestSuffix, lhs, expr)
```

---

### `NUR_TRY_AUTO`

```c
#define NUR_TRY_AUTO(DestSuffix, lhs, expr)
```

---

### `NUR_TRY_GOTO`

```c
#define NUR_TRY_GOTO(lhs, expr, errvar, label)
```

---

### `NUR_TRY_GOTO`

```c
#define NUR_TRY_GOTO(lhs, expr, errvar, label)
```

Non-GNU: expr is an lvalue, evaluated once via assignment.

---

### `NUR_TRY_TO`

```c
#define NUR_TRY_TO(DestSuffix, SrcSuffix, lhs, expr)
```

---

### `NUR_TRY_VOID`

```c
#define NUR_TRY_VOID(SrcSuffix, expr)
```

Like NUR_TRY_TO but for functions returning NurResult_void.

Usage:
NUR_TRY_VOID(i32, open_file(path));

---

### `NUR_TRY_VOID_AUTO`

```c
#define NUR_TRY_VOID_AUTO(expr)
```

---

### `NUR_TRY_VOID_AUTO`

```c
#define NUR_TRY_VOID_AUTO(expr)
```

---

### `NUR_TRY_VOID_FROM_VOID`

```c
#define NUR_TRY_VOID_FROM_VOID(expr)
```

---

### `NUR_TRY_VOID_GOTO`

```c
#define NUR_TRY_VOID_GOTO(expr, errvar, label)
```

---

### `NUR_TRY_VOID_GOTO`

```c
#define NUR_TRY_VOID_GOTO(expr, errvar, label)
```

---

### `NUR_TYPEOF`

```c
#define NUR_TYPEOF(expr) decltype(expr)
```

---

### `NUR_TYPEOF`

```c
#define NUR_TYPEOF(expr) typeof(expr)
```

---

### `NUR_TYPEOF`

```c
#define NUR_TYPEOF(expr) __typeof__(expr)
```

---

### `NUR_UNLIKELY`

```c
#define NUR_UNLIKELY(x) __builtin_expect(!!(x), 0)
```

---

### `NUR_UNLIKELY`

```c
#define NUR_UNLIKELY(x) (x)
```

---

### `NUR_UNUSED`

```c
#define NUR_UNUSED __attribute__((unused))
```

---

### `NUR_UNUSED`

```c
#define NUR_UNUSED
```

---

### `NUR_UNWRAP`

```c
#define NUR_UNWRAP(r)
```

---

### `NUR_UNWRAP`

```c
#define NUR_UNWRAP(r)
```

Non-GNU: takes (r).u.error by value (not &(r).u.error), avoiding 'lvalue required'
on rvalue function calls like NUR_UNWRAP(f()).
Note: evaluate 'r' to a temporary variable if 'r' has non-trivial side effects.

---

### `NUR_UNWRAP_OR`

```c
#define NUR_UNWRAP_OR(r, fallback)
```

---

### `NUR_UNWRAP_OR`

```c
#define NUR_UNWRAP_OR(r, fallback)
```

Get value or fallback if error.

Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);

---

### `NUR_VALUE`

```c
#define NUR_VALUE(r) ((r).u.value)
```

Value access - use only after checking status

---

### `NUR_VERSION_MAJOR`

```c
#define NUR_VERSION_MAJOR 1
```

---

### `NUR_VERSION_MINOR`

```c
#define NUR_VERSION_MINOR 1
```

---

### `NUR_VERSION_NUMBER`

```c
#define NUR_VERSION_NUMBER 10100
```

---

### `NUR_VERSION_PATCH`

```c
#define NUR_VERSION_PATCH 0
```

---

### `NUR_VERSION_STRING`

```c
#define NUR_VERSION_STRING "1.1.0"
```

---

### `NUR_VOID_ERROR_OF`

```c
#define NUR_VOID_ERROR_OF(r) ((r).error)
```

---

### `NUR_VOID_IF_ERR`

```c
#define NUR_VOID_IF_ERR(r, err_var, body)
```

Branch on error for void result and bind error struct into err_var.

---

### `NUR_WRAP_ERROR`

```c
#define NUR_WRAP_ERROR(err_val, new_ctx)
```

---

### `nur_error_eprint`

```c
#define nur_error_eprint(err) nur_error_print((err), stderr)
```

Convenience macro to print error to stderr.

Usage:
if (NUR_IS_ERR(result)) {
nur_error_eprint(&NUR_ERROR_OF(result));
}

---

## Functions & Utilities

### `nur_err_DestSuffix`

```c
return nur_err_DestSuffix(error)
```

---

### `nur_err_ptr`

```c
return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "malloc: size cannot be zero")
```

---

### `nur_err_ptr`

```c
return nur_err_ptr(NUR_ERROR(NUR_E_OUT_OF_MEMORY, "malloc failed")
```

---

### `nur_err_ptr`

```c
return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "realloc: size cannot be zero")
```

---

### `nur_err_ptr`

```c
return nur_err_ptr(NUR_ERROR(NUR_E_OUT_OF_MEMORY, "realloc failed")
```

---

### `nur_err_void`

```c
NurResult_void nur_err_void(nur_error_t err_)
```

Create a failed void result with the given error.

---

### `nur_err_void`

```c
return nur_err_void(NUR_ERROR_OF(_nur_tmp_)
```

---

### `nur_err_void`

```c
return nur_err_void(NUR_ERROR_OF(_nur_tmp_)
```

---

### `nur_err_void`

```c
return nur_err_void(NUR_ERROR_OF(_nur_tmp_)
```

---

### `nur_err_void`

```c
return nur_err_void(_nur_tmp_.error)
```

---

### `nur_err_void`

```c
return nur_err_void(_nur_r_.u.error)
```

---

### `nur_err_void`

```c
return nur_err_void(err)
```

---

### `nur_error_code_from_errno`

```c
nur_error_code_t nur_error_code_from_errno(int e)
```

Maps errno to a semantic nur_error_code_t.
Fix: NUR_ERROR_ERRNO previously always returned NUR_E_IO;
now ENOENT->NOT_FOUND, EACCES->PERMISSION, ENOMEM->OUT_OF_MEMORY, etc.

---

### `nur_error_code_from_win32`

```c
now uses nur_error_code_from_win32()
```

---

### `nur_error_code_from_win32`

```c
nur_error_code_t nur_error_code_from_win32(unsigned long error)
```

Maps a Win32/Winsock error number to a semantic nur_error_code_t.

Fixes:
- 10035 (WSAEWOULDBLOCK) was NUR_E_BUSY, now NUR_E_WOULD_BLOCK
- 38 (ERROR_HANDLE_EOF) was NUR_E_IO, now NUR_E_EOF
- 10053 (WSAECONNABORTED) was NUR_E_REFUSED, now NUR_E_CONNECTION

---

### `nur_error_format`

```c
size_t nur_error_format(const nur_error_t *error, char *buffer, size_t capacity)
```

Renders error into buffer as a single line.
Returns required length excluding NUL. If >= capacity, output was truncated.

Fix: if error is NULL, writes NUL to buffer[0] so buf is always valid.

---

### `nur_error_json`

```c
size_t nur_error_json(const nur_error_t *error, char *buffer, size_t capacity)
```

Renders error as a valid RFC 8259 JSON string into buffer.
Returns required length excluding NUL. If >= capacity, output was truncated.
Thread-safe, allocation-free.

Example output:
{"ok":false,"code":30,"name":"NUR_E_NOT_FOUND","message":"file missing","context":"cfg.json","sys":2,"file":"main.c","func":"run","line":42}

---

### `nur_error_print`

```c
void nur_error_print(const nur_error_t *err, FILE *f)
```

Print error information to a FILE stream.
Format: [CODE_NAME] message (context: ...) (sys: N) at file:line in func()

Usage:
if (NUR_IS_ERR(result)) {
nur_error_print(&NUR_ERROR_OF(result), stderr);
}

---

### `nur_error_with_context`

```c
nur_error_t nur_error_with_context(nur_error_t e, const char *ctx)
```

Wrap an existing error with new context, PRESERVING original code, sys,
message, file, func, line, and backtrace. Only .context is changed.
This is NOT a full error chain -- the original source location is retained.

Fix: previously called nur_internal_make_error with new source location,
discarding the original trace. Now uses nur_error_with_context().

Usage: NUR_WRAP_ERROR(NUR_ERROR_OF(res), "failed to parse config")

---

### `nur_free`

```c
void nur_free(void *ptr)
```

Free wrapper for API symmetry.

Usage:
nur_free(ptr);

---

### `nur_malloc`

```c
NurResult_ptr nur_malloc(size_t size)
```

malloc wrapper returning NurResult_ptr.
Never returns NULL inside the result - failures become error results.

Ownership: caller must free the returned pointer on success.

Usage:
NurResult_ptr result = nur_malloc(1024);
if (NUR_IS_OK(result)) {
void *ptr = NUR_VALUE(result);
// ... use ptr ...
free(ptr);
}

---

### `nur_ok_ptr`

```c
return nur_ok_ptr(ptr)
```

---

### `nur_ok_ptr`

```c
return nur_ok_ptr(new_ptr)
```

---

### `nur_ok_void`

```c
NurResult_void nur_ok_void(void)
```

Create a successful void result. (Freestanding-safe: no memset needed)

---

### `nur_realloc`

```c
NurResult_ptr nur_realloc(void *ptr, size_t size)
```

realloc wrapper returning NurResult_ptr.

Ownership:
- On success: caller must free the returned pointer
- On failure: original ptr remains valid and unchanged

Usage:
NurResult_ptr result = nur_realloc(old_ptr, new_size);
if (NUR_IS_OK(result)) {
void *new_ptr = NUR_VALUE(result);
// old_ptr is now invalid, use new_ptr
} else {
// old_ptr is still valid
}

---

### `nur_set_panic_handler`

```c
void nur_set_panic_handler(nur_panic_handler_t handler)
```

Set custom panic handler. Set NULL to restore default stderr+abort behaviour.

---

## Error Codes

### `NUR_E_AGAIN`

```c
NUR_E_AGAIN = 44
```

---

### `NUR_E_ALREADY_EXISTS`

```c
NUR_E_ALREADY_EXISTS = 32
```

---

### `NUR_E_BUG`

```c
NUR_E_BUG = 52
```

---

### `NUR_E_BUSY`

```c
NUR_E_BUSY = 22
```

---

### `NUR_E_CANCELLED`

```c
NUR_E_CANCELLED = 41
```

---

### `NUR_E_CHECKSUM`

```c
NUR_E_CHECKSUM = 15
```

---

### `NUR_E_CLOSED`

```c
NUR_E_CLOSED = 35
```

---

### `NUR_E_CONNECTION`

```c
NUR_E_CONNECTION = 42
```

---

### `NUR_E_CORRUPTED`

```c
NUR_E_CORRUPTED = 13
```

---

### `NUR_E_DEADLOCK`

```c
NUR_E_DEADLOCK = 24
```

---

### `NUR_E_ENCODING`

```c
NUR_E_ENCODING = 12
```

---

### `NUR_E_EOF`

```c
NUR_E_EOF = 34
```

---

### `NUR_E_FORMAT`

```c
NUR_E_FORMAT = 11
```

---

### `NUR_E_INTERNAL`

```c
NUR_E_INTERNAL = 51
```

---

### `NUR_E_INVALID_ARGUMENT`

```c
NUR_E_INVALID_ARGUMENT = 1
```

Input/Validation Errors (1-9)

---

### `NUR_E_INVALID_STATE`

```c
NUR_E_INVALID_STATE = 4
```

---

### `NUR_E_IO`

```c
NUR_E_IO = 33
```

---

### `NUR_E_NONE`

```c
NUR_E_NONE = 0
```

---

### `NUR_E_NOT_FOUND`

```c
NUR_E_NOT_FOUND = 30
```

I/O Errors (30-39)

---

### `NUR_E_NULL_POINTER`

```c
NUR_E_NULL_POINTER = 3
```

---

### `NUR_E_OUT_OF_MEMORY`

```c
NUR_E_OUT_OF_MEMORY = 20
```

Resource Errors (20-29)

---

### `NUR_E_OUT_OF_RANGE`

```c
NUR_E_OUT_OF_RANGE = 2
```

---

### `NUR_E_OVERFLOW`

```c
NUR_E_OVERFLOW = 5
```

---

### `NUR_E_PARSE`

```c
NUR_E_PARSE = 10
```

Data Processing Errors (10-19)

---

### `NUR_E_PERMISSION`

```c
NUR_E_PERMISSION = 31
```

---

### `NUR_E_PROTOCOL`

```c
NUR_E_PROTOCOL = 43
```

---

### `NUR_E_QUOTA`

```c
NUR_E_QUOTA = 23
```

---

### `NUR_E_REFUSED`

```c
NUR_E_REFUSED = 45
```

---

### `NUR_E_RESOURCE_LIMIT`

```c
NUR_E_RESOURCE_LIMIT = 21
```

---

### `NUR_E_SYSTEM`

```c
NUR_E_SYSTEM = 55
```

---

### `NUR_E_TIMEOUT`

```c
NUR_E_TIMEOUT = 40
```

Network/Async Errors (40-49)

---

### `NUR_E_TRUNCATED`

```c
NUR_E_TRUNCATED = 14
```

---

### `NUR_E_UNDERFLOW`

```c
NUR_E_UNDERFLOW = 6
```

---

### `NUR_E_UNINITIALIZED`

```c
NUR_E_UNINITIALIZED = 54
```

---

### `NUR_E_UNSUPPORTED`

```c
NUR_E_UNSUPPORTED = 50
```

System Errors (50-59)

---

### `NUR_E_USER_BASE`

```c
NUR_E_USER_BASE = 1000
```

User-defined error codes start here

---

### `NUR_E_VERSION`

```c
NUR_E_VERSION = 53
```

---

### `NUR_E_WOULD_BLOCK`

```c
NUR_E_WOULD_BLOCK = 36
```

---
