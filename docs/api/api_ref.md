# nur.h API Reference

Comprehensive, auto-generated API documentation for `nur.h` (Author: Irene Al-Caligo).

## Macros & Propagation

### `INT32_MAX`

```c
#define INT32_MAX 2147483647L
```

---

### `INT32_MIN`

```c
#define INT32_MIN (-2147483647L - 1L)
```

---

### `NUR_AND_THEN`

```c
#define NUR_AND_THEN(result, func)
```

Chain a result-returning function on a successful result.

NUR_AND_THEN(result, func) evaluates result once. If Ok, calls func with
the payload and returns its NurResult. If Err, passes the error through.

func must have signature: NurResult_<same_suffix> func(<value_type>).

GNU extension: uses statement expressions for single-evaluation.

Usage:
NurResult_i32 parse_uid(int raw);  // some transform
NurResult_i32 r2 = NUR_AND_THEN(r, parse_uid);

---

### `NUR_AND_THEN`

```c
#define NUR_AND_THEN(result, func)
```

---

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
#define NUR_AUTO __extension__ __auto_type
```

---

### `NUR_AUTO_FILE`

```c
#define NUR_AUTO_FILE __attribute__((cleanup(nur_internal_cleanup_fclose)))
```

Automatically closes FILE stream when pointer leaves scope.

Usage:
NUR_AUTO_FILE FILE *f = fopen("cfg.json", "r");

---

### `NUR_AUTO_FILE`

```c
#define NUR_AUTO_FILE
```

---

### `NUR_AUTO_FILE`

```c
#define NUR_AUTO_FILE
```

---

### `NUR_AUTO_FREE`

```c
#define NUR_AUTO_FREE __attribute__((cleanup(nur_internal_cleanup_free)))
```

Automatically frees heap memory when pointer leaves scope.

Usage:
NUR_AUTO_FREE char *buf = malloc(1024);

---

### `NUR_AUTO_FREE`

```c
#define NUR_AUTO_FREE
```

---

### `NUR_AUTO_FREE`

```c
#define NUR_AUTO_FREE
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

### `NUR_C89`

```c
#define NUR_C89 1
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

### `NUR_CHECK_AS_MSG`

```c
#define NUR_CHECK_AS_MSG(DestSuffix, cond, code, msg)
```

Like NUR_CHECK_AS but with an explicit message (typed return).
Returns typed NurResult error if condition is false.

Usage:
NUR_CHECK_AS_MSG(i32, size > 0, NUR_E_INVALID_ARGUMENT, "size must be positive");

---

### `NUR_CHECK_MSG`

```c
#define NUR_CHECK_MSG(cond, code, msg)
```

Like NUR_CHECK but with an explicit message argument for more context.
Returns NurResult_void error if condition is false.

Usage:
NUR_CHECK_MSG(port > 0 && port < 65536, NUR_E_OUT_OF_RANGE, "port must be 1-65535");

---

### `NUR_CLEANUP`

```c
#define NUR_CLEANUP(func) __attribute__((cleanup(func)))
```

Attach automatic cleanup function to a variable when it leaves scope.
Supported on GCC and Clang. On MSVC/standard C, use NUR_TRY_GOTO.

Usage:
NUR_CLEANUP(free_buf) char *buf = malloc(64);

---

### `NUR_CLEANUP`

```c
#define NUR_CLEANUP(func)
```

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
#define NUR_CUSTOM_PANIC_HANDLER_STORAGE __declspec(selectany)
```

---

### `NUR_CUSTOM_PANIC_HANDLER_STORAGE`

```c
#define NUR_CUSTOM_PANIC_HANDLER_STORAGE __attribute__((weak))
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

### `NUR_ERROR_CAUSED_BY`

```c
#define NUR_ERROR_CAUSED_BY(err_val, new_ctx, cause_val)
```

Wrap an error with both context and root cause.

---

### `NUR_ERROR_CONTEXT_AT`

```c
#define NUR_ERROR_CONTEXT_AT(err, idx) nur_error_context_at(&(err), (size_t)(idx))
```

---

### `NUR_ERROR_CONTEXT_AT`

```c
#define NUR_ERROR_CONTEXT_AT(err, idx) nur_error_context_at(&(err), (size_t)(idx))
```

---

### `NUR_ERROR_COPY`

```c
#define NUR_ERROR_COPY(code, dynamic_str)
```

---

### `NUR_ERROR_ERRNO`

```c
#define NUR_ERROR_ERRNO(msg)
```

---

### `NUR_ERROR_FMT`

```c
#define NUR_ERROR_FMT(code, fmt, ...)
```

---

### `NUR_ERROR_INLINE_MSG_SIZE`

```c
#define NUR_ERROR_INLINE_MSG_SIZE 64
```

---

### `NUR_ERROR_IS`

```c
#define NUR_ERROR_IS(err, errcode_) ((err).code == (errcode_))
```

---

### `NUR_ERROR_MSG`

```c
#define NUR_ERROR_MSG(err) nur_error_message_of(&(err))
```

---

### `NUR_ERROR_OF`

```c
#define NUR_ERROR_OF(r) ((r).u.error)
```

---

### `NUR_ERROR_WRAP`

```c
#define NUR_ERROR_WRAP(err_val, new_ctx) NUR_WRAP_ERROR((err_val), (new_ctx))
```

Convenience alias: wrap an existing error with a new context string,
creating a new nur_error_t that preserves original code, sys, message,
file, func, line but replaces .context.

This is an ergonomic alias for NUR_WRAP_ERROR (which preserves original
source location). Use for adding caller context without losing the origin.

Usage:
nur_error_t wrapped = NUR_ERROR_WRAP(NUR_ERROR_OF(r), "while parsing config");
return NUR_ERR(i32, wrapped);

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

### `NUR_EXPECT_VOID`

```c
#define NUR_EXPECT_VOID(r, msg) nur_expect_void_internal((r), (msg), NUR_LOC_FILE, NUR_LOC_LINE)
```

---

### `NUR_E_PERMISSION_DENIED`

```c
#define NUR_E_PERMISSION_DENIED NUR_E_PERMISSION
```

Backward-compatibility and POSIX taxonomy alias

---

### `NUR_FN`

```c
#define NUR_FN(ReturnType, FuncName, ...) NUR_NODISCARD ReturnType FuncName(__VA_ARGS__)
```

---

### `NUR_GNU_EXTENSIONS`

```c
#define NUR_GNU_EXTENSIONS 1
```

---

### `NUR_H_INCLUDED`

```c
#define NUR_H_INCLUDED
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
#define NUR_INLINE static __inline__
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static __inline
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static inline
```

---

### `NUR_INLINE`

```c
#define NUR_INLINE static __inline
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

### `NUR_MAP`

```c
#define NUR_MAP(result, func)
```

Map a successful Result value using func, preserving error.

Evaluates result once. If Ok, applies func to the payload and returns
a new Ok with the result. If Err, propagates the existing error.

NOTE: NUR_MAP is restricted to same-type transformations (T -> T).
For mapping between different types (T -> U), use NUR_MAP_TO(target_suffix, result, func).

func signature: <value_type> func(<value_type>)

Usage:
static int double_it(int x) { return x * 2; }
NurResult_i32 r2 = NUR_MAP(r, double_it);

---

### `NUR_MAP_TO`

```c
#define NUR_MAP_TO(suffix, result, func)
```

Typed map converting Ok value into target type 'suffix'.
On error, constructs a result of target type 'suffix' with the original error.

---

### `NUR_MAP_TO`

```c
#define NUR_MAP_TO(suffix, result, func)
```

NUR_MAP_TO - Portable typed map for non-GNU and ANSI C compilers.
Maps Ok value using 'func' into a new Ok result of target type 'suffix'.
On error, constructs a result of target type 'suffix' with the original error.

---

### `NUR_MATCH`

```c
#define NUR_MATCH(r, val_var, ok_body, err_var, err_body)
```

Type-inferring pattern match construct (GNU/C23/C++).

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

### `NUR_MAX_CONTEXT_DEPTH`

```c
#define NUR_MAX_CONTEXT_DEPTH 4
```

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
#define NUR_NODISCARD [[nodiscard]]
```

---

### `NUR_NODISCARD`

```c
#define NUR_NODISCARD
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
#define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg)
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg) [[nodiscard]]
```

---

### `NUR_NODISCARD_MSG`

```c
#define NUR_NODISCARD_MSG(msg) __attribute__((warn_unused_result))
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
#define NUR_NODISCARD_TYPE [[nodiscard]]
```

---

### `NUR_NODISCARD_TYPE`

```c
#define NUR_NODISCARD_TYPE __attribute__((warn_unused_result))
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

### `NUR_NO_CTYPE`

```c
#define NUR_NO_CTYPE 1
```

---

### `NUR_NO_ERRNO`

```c
#define NUR_NO_ERRNO 1
```

---

### `NUR_NO_EXTENSIONS`

```c
#define NUR_NO_EXTENSIONS 1
```

---

### `NUR_NO_SECURITY_SHIELD`

```c
#define NUR_NO_SECURITY_SHIELD 1
```

---

### `NUR_NO_STDIO`

```c
#define NUR_NO_STDIO 1
```

---

### `NUR_NO_STDLIB`

```c
#define NUR_NO_STDLIB 1
```

---

### `NUR_NO_STRING`

```c
#define NUR_NO_STRING 1
```

---

### `NUR_OK`

```c
#define NUR_OK(...) NUR_INTERNAL_GET_MACRO_3(__VA_ARGS__, NUR_INTERNAL_OK_2, NUR_INTERNAL_OK_1, nur_internal_unused)(__VA_ARGS__)
```

---

### `NUR_OK`

```c
#define NUR_OK(...) NUR_INTERNAL_GET_MACRO_3(__VA_ARGS__, NUR_INTERNAL_OK_2, NUR_INTERNAL_OK_1, nur_internal_unused)(__VA_ARGS__)
```

---

### `NUR_OK`

```c
#define NUR_OK(suffix, val) nur_ok_##suffix(val)
```

C99 legacy 2-argument constructor

---

### `NUR_OK_OR`

```c
#define NUR_OK_OR(suffix, opt, err)
```

Convert Option<T> to Result<T, nur_error_t>.
If opt has value, returns Ok(value); otherwise returns Err(err).

Usage:
NurResult_i32 res = NUR_OK_OR(i32, opt, NUR_ERROR(NUR_E_NOT_FOUND, "no value"));

---

### `NUR_OK_VOID`

```c
#define NUR_OK_VOID() nur_ok_void()
```

---

### `NUR_OPTION_TRY`

```c
#define NUR_OPTION_TRY(lhs, expr)
```

---

### `NUR_OPTION_TRY_TO`

```c
#define NUR_OPTION_TRY_TO(suffix, lhs, expr)
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

### `NUR_PANIC_ABORT`

```c
#define NUR_PANIC_ABORT() abort()
```

---

### `NUR_PANIC_ABORT`

```c
#define NUR_PANIC_ABORT() __builtin_trap()
```

---

### `NUR_PANIC_ABORT`

```c
#define NUR_PANIC_ABORT() do { volatile int *p_ = 0; (void)*p_; for(;;) {} } while(0)
```

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

### `NUR_PURIST_MODE`

```c
#define NUR_PURIST_MODE 1
```

---

### `NUR_REQUIRE_OK`

```c
#define NUR_REQUIRE_OK(r)
```

Asserts that result is Ok, printing full error details before aborting.

Usage:
NUR_REQUIRE_OK(res);

---

### `NUR_REQUIRE_OK_MSG`

```c
#define NUR_REQUIRE_OK_MSG(r, msg)
```

Asserts that result is Ok with custom failure message, printing details before aborting.

Usage:
NUR_REQUIRE_OK_MSG(res, "Configuration loading failed");

---

### `NUR_RESULT_TYPE`

```c
#define NUR_RESULT_TYPE(T, suffix)
```

---

### `NUR_RESULT_TYPE_E`

```c
#define NUR_RESULT_TYPE_E(T, E, suffix)
```

Generate a Result<T, E> type with arbitrary error type E.

Creates:
- NurResult_<suffix>        The result type
- nur_ok_<suffix>(val)      Success constructor
- nur_err_<suffix>(err)     Failure constructor
- nur_is_ok_<suffix>(r)     Success predicate
- nur_is_err_<suffix>(r)    Failure predicate
- nur_value_<suffix>(r)     Direct value accessor
- nur_error_<suffix>(r)     Direct error accessor
- nur_unwrap_or_<suffix>    Value extractor with fallback

---

### `NUR_RETURN_IF_ERR`

```c
#define NUR_RETURN_IF_ERR(expr)
```

Return an error result from the enclosing function if expr is Err.
The enclosing function's return type must be NurResult_void.

This is a portable (no GNU extensions) early-return helper.
It evaluates expr exactly once via a named temporary.

Usage:
NUR_RETURN_IF_ERR(validate_input(buf));

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

Statement expression version of NUR_TRY (GNU extension / C++).
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

Propagate into a specific return type (GNU extension / C++).

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
#define NUR_TRY_GOTO(lhs, expr_lval, errvar, label)
```

Non-GNU & C89 portable: user must pre-assign result to a named variable before
passing to NUR_TRY_GOTO to avoid double-evaluation of expr.
Example:
NurResult_i32 _r = parse_int(str);
NUR_TRY_GOTO(val, _r, err, cleanup);
This is documented in docs/PORTABILITY.md.

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
#define NUR_TYPEOF(expr) __extension__ __typeof__(expr)
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

Universal single/portable expression fallback supporting all standard and custom Result types

---

### `NUR_UNWRAP_OPTION`

```c
#define NUR_UNWRAP_OPTION(o)
```

---

### `NUR_UNWRAP_OR`

```c
#define NUR_UNWRAP_OR(r, fallback)
```

Get value or fallback if error (evaluates r exactly once).

Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);

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

---

### `NUR_UNWRAP_VOID`

```c
#define NUR_UNWRAP_VOID(r) nur_unwrap_void_internal((r), NUR_LOC_FILE, NUR_LOC_LINE)
```

---

### `NUR_VALUE`

```c
#define NUR_VALUE(r) ((r).u.value)
```

Value access - use only after checking status

---

### `NUR_VALUE_OR`

```c
#define NUR_VALUE_OR(r, fallback) NUR_UNWRAP_OR((r), (fallback))
```

Portable value-or alias (same semantics, more explicit name)

---

### `NUR_VERSION_MAJOR`

```c
#define NUR_VERSION_MAJOR 1
```

---

### `NUR_VERSION_MINOR`

```c
#define NUR_VERSION_MINOR 4
```

---

### `NUR_VERSION_NUMBER`

```c
#define NUR_VERSION_NUMBER 10400
```

---

### `NUR_VERSION_PATCH`

```c
#define NUR_VERSION_PATCH 0
```

---

### `NUR_VERSION_STRING`

```c
#define NUR_VERSION_STRING "1.4.0"
```

---

### `NUR_VOID_ERROR_OF`

```c
#define NUR_VOID_ERROR_OF(r) ((r).u.error)
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

### `NUR_WRAP_ERROR`

```c
#define NUR_WRAP_ERROR(err_val, new_ctx)
```

---

### `false`

```c
#define false 0
```

---

### `gets`

```c
#define gets(buf) _Static_assert(0, "[nur.h Security Shield] 'gets()' is inherently unsafe and banned due to buffer overflow! Use 'fgets()' instead.")
```

---

### `gets`

```c
#define gets(buf)
```

---

### `nur_error_elog_json`

```c
#define nur_error_elog_json(err) nur_error_log_json((err), stderr)
```

Convenience macro: Stream structured JSON error directly to stderr.

Usage:
nur_error_elog_json(&NUR_ERROR_OF(res));

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

### `prinf`

```c
#define prinf(...) _Static_assert(0, "[nur.h Typo Guard] Typo detected: 'prinf' is not a valid C function. Did you mean 'printf'?")
```

---

### `prinf`

```c
#define prinf(...)
```

---

### `true`

```c
#define true 1
```

---

## Functions & Utilities

### `nur_add_i32`

```c
NurResult_i32 nur_add_i32(int32_t a, int32_t b)
```

---

### `nur_cpp_make_result`

```c
type nur_cpp_make_result(T val)
```

---

### `nur_div_i32`

```c
NurResult_i32 nur_div_i32(int32_t a, int32_t b)
```

---

### `nur_err_DestSuffix`

```c
return nur_err_DestSuffix(error)
```

---

### `nur_err_cstr`

```c
return nur_err_cstr(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0, "Invalid argument passed to nur_file_read_text", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_cstr`

```c
return nur_err_cstr(nur_internal_make_error( nur_error_code_from_errno(errno)
```

---

### `nur_err_cstr`

```c
return nur_err_cstr(nur_internal_make_error( NUR_E_IO, errno, "I/O error while reading file", path, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_cstr`

```c
return nur_err_cstr(nur_internal_make_error( NUR_E_TRUNCATED, 0, "Buffer capacity insufficient: file truncated", path, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_cstr`

```c
return nur_err_cstr(nur_internal_make_error( NUR_E_IO, errno, "I/O error while reading file", path, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_f64`

```c
return nur_err_f64(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0, "Square root of negative number is undefined in real domain", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow in 32-bit addition", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow in 32-bit addition", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow/underflow in 32-bit subtraction", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow/underflow in 32-bit subtraction", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow in 32-bit multiplication", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow in 32-bit multiplication", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0, "Division by zero", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
```

---

### `nur_err_i32`

```c
return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0, "Integer overflow in division (INT32_MIN / -1)
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

### `nur_err_usize`

```c
return nur_err_usize(nur_internal_make_error(NUR_E_NULL_POINTER, 0, "Cannot calculate length of NULL string pointer", NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)
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
return nur_err_void(NUR_ERROR((code)
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

### `nur_error_code_of`

```c
nur_error_code_t nur_error_code_of(const nur_error_t *err)
```

---

### `nur_error_copy_msg`

```c
NUR_INLINE nur_error_t nur_error_copy_msg(nur_error_t e, const char *src_str)
```

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

### `nur_error_log_json`

```c
void nur_error_log_json(const nur_error_t *err, FILE *f)
```

Stream structured JSON error directly to a FILE stream (e.g. stdout or file).
Thread-safe, zero dynamic heap allocation. Appends a newline and flushes.

Usage:
nur_error_log_json(&NUR_ERROR_OF(res), stderr);

---

### `nur_error_message_of`

```c
return nur_error_message_of(err)
```

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

### `nur_error_void_val`

```c
nur_error_t nur_error_void_val(NurResult_void r_)
```

---

### `nur_error_with_cause`

```c
nur_error_t nur_error_with_cause(nur_error_t e, const char *ctx, const char *cause_str)
```

---

### `nur_error_with_cause`

```c
nur_error_t nur_error_with_cause(nur_error_t e, const char *ctx, const char *cause_str)
```

---

### `nur_error_with_context`

```c
nur_error_t nur_error_with_context(nur_error_t e, const char *ctx)
```

---

### `nur_error_with_context`

```c
nur_error_t nur_error_with_context(nur_error_t e, const char *ctx)
```

---

### `nur_expect_void_internal`

```c
void nur_expect_void_internal(NurResult_void r_, const char *msg_, const char *file_, unsigned int line_)
```

Assert that a NurResult_void is successful with custom message, panicking on error.
Evaluates 'r' exactly once across all C standards.

---

### `nur_file_read_text`

```c
NurResult_cstr nur_file_read_text(const char *path, char *buf, size_t buf_size)
```

---

### `nur_free`

```c
void nur_free(void *ptr)
```

Free wrapper for API symmetry.

Usage:
nur_free(ptr);

---

### `nur_is_err_void`

```c
bool nur_is_err_void(NurResult_void r_)
```

---

### `nur_is_ok_void`

```c
bool nur_is_ok_void(NurResult_void r_)
```

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

### `nur_mul_i32`

```c
NurResult_i32 nur_mul_i32(int32_t a, int32_t b)
```

---

### `nur_ok_bool`

```c
return nur_ok_bool(v)
```

---

### `nur_ok_cstr`

```c
return nur_ok_cstr(v)
```

---

### `nur_ok_cstr`

```c
return nur_ok_cstr(buf)
```

---

### `nur_ok_cstr_const`

```c
return nur_ok_cstr_const(v)
```

---

### `nur_ok_f32`

```c
return nur_ok_f32(v)
```

---

### `nur_ok_f64`

```c
return nur_ok_f64(v)
```

---

### `nur_ok_f64`

```c
return nur_ok_f64(0.0)
```

---

### `nur_ok_f64`

```c
return nur_ok_f64(guess)
```

---

### `nur_ok_i16`

```c
return nur_ok_i16(v)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(v)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(res)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(a + b)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(res)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(a - b)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(res)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32((int32_t)
```

---

### `nur_ok_i32`

```c
return nur_ok_i32(a / b)
```

---

### `nur_ok_i64`

```c
return nur_ok_i64(v)
```

---

### `nur_ok_i64`

```c
return nur_ok_i64(v)
```

---

### `nur_ok_i8`

```c
return nur_ok_i8(v)
```

---

### `nur_ok_ptr`

```c
return nur_ok_ptr(v)
```

---

### `nur_ok_ptr`

```c
return nur_ok_ptr((void*)
```

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

### `nur_ok_u16`

```c
return nur_ok_u16(v)
```

---

### `nur_ok_u64`

```c
return nur_ok_u64(v)
```

---

### `nur_ok_u8`

```c
return nur_ok_u8(v)
```

---

### `nur_ok_usize`

```c
return nur_ok_usize(v)
```

---

### `nur_ok_usize`

```c
return nur_ok_usize(v)
```

---

### `nur_ok_usize`

```c
return nur_ok_usize(v)
```

---

### `nur_ok_usize`

```c
return nur_ok_usize(strlen(str)
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

### `nur_result_is_err`

```c
bool nur_result_is_err(const void *result_ptr)
```

---

### `nur_result_is_ok`

```c
bool nur_result_is_ok(const void *result_ptr)
```

Universal pointer predicates for pure function checks (100% macro-free).
Works across all NurResult_<T> types since .ok is at byte offset 0.

---

### `nur_set_panic_handler`

```c
void nur_set_panic_handler(nur_panic_handler_t handler)
```

Set custom panic handler. Synchronized across all translation units.
Set NULL to restore default stderr+abort behaviour.

---

### `nur_sqrt`

```c
NurResult_f64 nur_sqrt(double x)
```

---

### `nur_strlen`

```c
NurResult_usize nur_strlen(const char *str)
```

---

### `nur_sub_i32`

```c
NurResult_i32 nur_sub_i32(int32_t a, int32_t b)
```

---

### `nur_unwrap_void_internal`

```c
NUR_INLINE void nur_unwrap_void_internal(NurResult_void r_, const char *file_, unsigned int line_)
```

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
