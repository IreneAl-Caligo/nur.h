/* SPDX-License-Identifier: MIT */
/**
 * @file    nur.h
 * @brief   Typed outcomes for C: Result, Option, Status and structured errors.
 * @version 0.1.0
 *
 * Copyright (c) 2026 The nur.h authors.  Licensed under the MIT license.
 *
 * OVERVIEW
 * --------
 * nur.h makes failure explicit.  A fallible function returns a value that is
 * either a payload or a structured error.  It never reports failure through an
 * undocumented sentinel (-1, NULL) or through ambient state (errno).
 *
 *   Result<T>   NUR_DEFINE_RESULT(T, Name)   Ok(T)   | Err(nur_error_t)
 *   Status      nur_status                   Ok      | Err(nur_error_t)
 *   Option<T>   NUR_DEFINE_OPTION(T, Name)   Some(T) | None
 *
 * It is a single header: `#include <nur.h>` is all a consumer needs.  There is
 * nothing to link, nothing to initialise and nothing to free.
 *
 * QUICK START
 * -----------
 *     #include <nur.h>
 *     #include <stdio.h>
 *
 *     NUR_DEFINE_RESULT(int, int_result);
 *
 *     NUR_NODISCARD static int_result divide(int a, int b)
 *     {
 *         if (b == 0) {
 *             return NUR_FAIL(int_result, NUR_E_INVALID_ARGUMENT, "division by zero");
 *         }
 *         return NUR_OK(int_result, a / b);
 *     }
 *
 *     NUR_NODISCARD static int_result average(int sum, int count)
 *     {
 *         int q;
 *         NUR_TRY_ASSIGN(int_result, q, divide(sum, count));  // propagate on error
 *         return NUR_OK(int_result, q);
 *     }
 *
 *     int main(void)
 *     {
 *         int_result r = average(10, 0);
 *         if (NUR_IS_ERR(r)) {
 *             nur_error_print(stderr, &NUR_ERROR(r));
 *             return 1;
 *         }
 *         printf("%d\n", NUR_VALUE(r));
 *         return 0;
 *     }
 *
 * DESIGN RULES
 * ------------
 *   1. Failure must be explicit.
 *   2. Errors are machine-readable: a stable numeric code plus the native OS code.
 *   3. Messages are human-readable and never used for control flow.
 *   4. The core never allocates.
 *   5. The core never logs implicitly (printing is an explicit, opt-out helper).
 *   6. The core has no hidden global state and no OS dependency.
 *   7. C11 is the baseline.  C23 and compiler extensions may enhance, never gate.
 *   8. No macro evaluates an argument more than once (NUR_ASSERT does not evaluate
 *      its condition at all when assertions are disabled).
 *
 * CONTRACT
 * --------
 * Result invariant
 *   state == NUR_RESULT_OK  =>  data.value is the active member.
 *   state == NUR_RESULT_ERR =>  data.error is the active member.
 *   Reading the inactive member is a contract violation.  Check with NUR_IS_OK
 *   first, or use NUR_UNWRAP / NUR_EXPECT, which check for you.  A zero-
 *   initialised result is an *error* (fail-closed), never a bogus success.
 *   Padding and inactive-union bytes are unspecified: never compare results
 *   with memcmp().
 *
 * Ownership
 *   Every `const char *` inside nur_error_t is BORROWED and must outlive the
 *   error: use string literals, __FILE__, __func__ or other static storage.
 *   The library never copies, allocates or frees them.  Build dynamic text in a
 *   caller-owned buffer and keep that buffer alive alongside the error.
 *
 * Threading
 *   The header is stateless.  Every function is reentrant and thread-safe.
 *
 * Linkage and ABI
 *   Everything is `static inline` (see NUR_API).  No symbols are exported, so
 *   there is no ABI to break.  The layout of nur_error_t is identical under
 *   every configuration, so translation units may use different NUR_CONFIG_*
 *   settings safely.
 *
 * Expected failure vs. programmer error
 *   Expected, recoverable failures travel in a Result/Status.  Violated
 *   invariants (NUR_ASSERT, NUR_PANIC, NUR_UNWRAP on an Err) terminate.
 *
 * Compilers
 *   Standard C11 (or C++11) is the baseline.  GCC- and Clang-compatible compilers
 *   and MSVC (/std:c11) are supported; every compiler-specific construct lives in
 *   section 4 behind a feature test and degrades to plain standard C.
 *
 * What nur.h does NOT guarantee
 *   - NUR_NODISCARD is a compiler-assisted warning, not a must-use type system.
 *   - Result does not make C memory safe and does not remove undefined behaviour.
 *   - Macros cannot be fully generic in C: propagation names its result types.
 *
 * CONFIGURATION (all optional; define before including nur.h)
 * -----------------------------------------------------------
 *   NUR_CONFIG_MINIMAL            Implies NO_STDIO and SOURCE_LOCATION=0.
 *   NUR_CONFIG_NO_STDIO           Do not include <stdio.h>.  Removes nur_error_print()
 *                                 and the default panic message.
 *   NUR_CONFIG_NO_ERRNO           Do not include <errno.h>.  Removes NUR_ERROR_FROM_ERRNO.
 *   NUR_CONFIG_SOURCE_LOCATION    0 none | 1 file:line | 2 file:line:function.
 *                                 Default: 2, or 0 when NDEBUG is defined.
 *   NUR_CONFIG_ASSERTS            1 enables NUR_ASSERT.  Default: 1, or 0 under NDEBUG.
 *   NUR_CONFIG_NO_NODISCARD       Disable all nodiscard annotations.
 *   NUR_CONFIG_NO_EXTENSIONS      Do not use compiler extensions (typeof).
 *   NUR_CONFIG_NO_BUILTIN_TYPES   Do not predefine nur_result_int and friends.
 *   NUR_CONFIG_PRINT_BUFFER_SIZE  Stack buffer used by nur_error_print().  Default 1024.
 *   NUR_PANIC_HANDLER(msg, err, file, func, line)
 *                                 Replaces the default panic reporter.  Must not return;
 *                                 if it does, the program is aborted.
 *   NUR_API                       Linkage of every function.  Default: static inline.
 *
 * Names beginning with NUR_PRIV_ or nur_priv_ are internal and may change.
 */

#ifndef NUR_H
#define NUR_H

/* ==========================================================================
 * 1. Version
 * ========================================================================== */

#define NUR_VERSION_MAJOR  0
#define NUR_VERSION_MINOR  1
#define NUR_VERSION_PATCH  0
#define NUR_VERSION_STRING "0.1.0"

#define NUR_VERSION_NUMBER \
    ((NUR_VERSION_MAJOR * 10000) + (NUR_VERSION_MINOR * 100) + NUR_VERSION_PATCH)

/** True when this header is at least version major.minor.patch. */
#define NUR_VERSION_AT_LEAST(major_, minor_, patch_) \
    (NUR_VERSION_NUMBER >= (((major_) * 10000) + ((minor_) * 100) + (patch_)))

/* ==========================================================================
 * 2. Language requirements and configuration
 * ========================================================================== */

#if defined(__cplusplus)
#  if defined(_MSVC_LANG)
#    define NUR_PRIV_CPLUSPLUS _MSVC_LANG
#  else
#    define NUR_PRIV_CPLUSPLUS __cplusplus
#  endif
#  if NUR_PRIV_CPLUSPLUS < 201103L
#    error "nur.h: C++ translation units require C++11 or newer"
#  endif
#elif !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)
#  error "nur.h: C translation units require C11 or newer (-std=c11 or /std:c11)"
#endif
#if !defined(NUR_PRIV_CPLUSPLUS)
#  define NUR_PRIV_CPLUSPLUS 0L
#endif

/* C23 (or a C2x draft that already has [[]] attributes).  __has_c_attribute()
 * alone is not enough: some compilers answer "yes" in older modes, where using
 * [[]] is a pedantic error. */
#if !defined(__cplusplus) && defined(__STDC_VERSION__) && (__STDC_VERSION__ > 201710L)
#  define NUR_C23 1
#else
#  define NUR_C23 0
#endif

#define NUR_SOURCE_LOCATION_NONE  0
#define NUR_SOURCE_LOCATION_BASIC 1
#define NUR_SOURCE_LOCATION_FULL  2

#if defined(NUR_CONFIG_MINIMAL)
#  if !defined(NUR_CONFIG_NO_STDIO)
#    define NUR_CONFIG_NO_STDIO
#  endif
#  if !defined(NUR_CONFIG_SOURCE_LOCATION)
#    define NUR_CONFIG_SOURCE_LOCATION NUR_SOURCE_LOCATION_NONE
#  endif
#endif

#if !defined(NUR_CONFIG_SOURCE_LOCATION)
#  if defined(NDEBUG)
#    define NUR_CONFIG_SOURCE_LOCATION NUR_SOURCE_LOCATION_NONE
#  else
#    define NUR_CONFIG_SOURCE_LOCATION NUR_SOURCE_LOCATION_FULL
#  endif
#endif

#if (NUR_CONFIG_SOURCE_LOCATION != NUR_SOURCE_LOCATION_NONE) && \
    (NUR_CONFIG_SOURCE_LOCATION != NUR_SOURCE_LOCATION_BASIC) && \
    (NUR_CONFIG_SOURCE_LOCATION != NUR_SOURCE_LOCATION_FULL)
#  error "nur.h: NUR_CONFIG_SOURCE_LOCATION must be 0 (none), 1 (file:line) or 2 (+function)"
#endif

#if !defined(NUR_CONFIG_ASSERTS)
#  if defined(NDEBUG)
#    define NUR_CONFIG_ASSERTS 0
#  else
#    define NUR_CONFIG_ASSERTS 1
#  endif
#endif

#if !defined(NUR_CONFIG_PRINT_BUFFER_SIZE)
#  define NUR_CONFIG_PRINT_BUFFER_SIZE 1024
#endif
#if NUR_CONFIG_PRINT_BUFFER_SIZE < 64
#  error "nur.h: NUR_CONFIG_PRINT_BUFFER_SIZE must be at least 64"
#endif

#if !defined(NUR_API)
#  define NUR_API static inline
#endif

/* ==========================================================================
 * 3. Platform detection and standard headers
 * ========================================================================== */

#if defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0)
#  define NUR_HOSTED 0
#else
#  define NUR_HOSTED 1
#endif

#if NUR_HOSTED && !defined(NUR_CONFIG_NO_STDIO)
#  define NUR_HAS_STDIO 1
#else
#  define NUR_HAS_STDIO 0
#endif

#if NUR_HOSTED && !defined(NUR_CONFIG_NO_ERRNO)
#  define NUR_HAS_ERRNO 1
#else
#  define NUR_HAS_ERRNO 0
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if NUR_HOSTED
#  include <stdlib.h>
#endif
#if NUR_HAS_STDIO
#  include <stdio.h>
#endif
#if NUR_HAS_ERRNO
#  include <errno.h>
#endif

/* ==========================================================================
 * 4. Compiler abstraction
 * ========================================================================== */

#if defined(__has_attribute)
#  define NUR_HAS_ATTRIBUTE(x_) __has_attribute(x_)
#else
#  define NUR_HAS_ATTRIBUTE(x_) 0
#endif

#if defined(__has_c_attribute)
#  define NUR_HAS_C_ATTRIBUTE(x_) __has_c_attribute(x_)
#else
#  define NUR_HAS_C_ATTRIBUTE(x_) 0
#endif

#if defined(__has_cpp_attribute)
#  define NUR_HAS_CPP_ATTRIBUTE(x_) __has_cpp_attribute(x_)
#else
#  define NUR_HAS_CPP_ATTRIBUTE(x_) 0
#endif

#define NUR_PRIV_CAT2(a_, b_) a_##b_
/** Token pasting that expands its arguments first. */
#define NUR_CAT(a_, b_) NUR_PRIV_CAT2(a_, b_)
#define NUR_PRIV_STR(x_) #x_
/** Stringification that expands its argument first. */
#define NUR_STRINGIFY(x_) NUR_PRIV_STR(x_)

/** A fresh identifier per expansion: NUR_UNIQUE(tmp_) -> tmp_0, tmp_1, ... */
#if defined(__COUNTER__)
#  define NUR_UNIQUE(prefix_) NUR_CAT(prefix_, __COUNTER__)
#else
#  define NUR_UNIQUE(prefix_) NUR_CAT(prefix_, __LINE__)
#endif

#if defined(__cplusplus)
#  define NUR_STATIC_ASSERT(cond_, msg_) static_assert(cond_, msg_)
#else
#  define NUR_STATIC_ASSERT(cond_, msg_) _Static_assert(cond_, msg_)
#endif

#define NUR_FUNC __func__

#if defined(__GNUC__) || defined(__clang__)
#  define NUR_LIKELY(x_)   __builtin_expect(!!(x_), 1)
#  define NUR_UNLIKELY(x_) __builtin_expect(!!(x_), 0)
#else
#  define NUR_LIKELY(x_)   (x_)
#  define NUR_UNLIKELY(x_) (x_)
#endif

/* Function that never returns. */
#if defined(__cplusplus)
#  define NUR_NORETURN [[noreturn]]
#elif NUR_C23 && NUR_HAS_C_ATTRIBUTE(noreturn)
#  define NUR_NORETURN [[noreturn]]
#elif defined(_MSC_VER)
#  define NUR_NORETURN __declspec(noreturn)
#else
#  define NUR_NORETURN _Noreturn
#endif

/*
 * NUR_NODISCARD       - place before a function declaration (before `static`):
 *                           NUR_NODISCARD static my_result f(void);
 * NUR_NODISCARD_TYPE  - placed after `struct` in a type definition.  Types
 *                       defined by NUR_DEFINE_RESULT/OPTION carry it, so
 *                       discarding any such return value is diagnosed where the
 *                       compiler supports it (C23, C++17, Clang).
 *
 * Capability ladder: C23/C++17 [[nodiscard]] -> GNU warn_unused_result
 * -> MSVC _Check_return_ -> nothing.  Unsupported compilers degrade silently.
 * GCC before C23 cannot annotate types, only functions.
 */
#if defined(NUR_CONFIG_NO_NODISCARD)
#  define NUR_NODISCARD
#  define NUR_NODISCARD_TYPE
#elif defined(__cplusplus) && (NUR_PRIV_CPLUSPLUS >= 201703L) && NUR_HAS_CPP_ATTRIBUTE(nodiscard)
#  define NUR_NODISCARD      [[nodiscard]]
#  define NUR_NODISCARD_TYPE [[nodiscard]]
#elif NUR_C23 && NUR_HAS_C_ATTRIBUTE(nodiscard)
#  define NUR_NODISCARD      [[nodiscard]]
#  define NUR_NODISCARD_TYPE [[nodiscard]]
#elif NUR_HAS_ATTRIBUTE(warn_unused_result)
#  define NUR_NODISCARD __attribute__((warn_unused_result))
#  if defined(__clang__)
#    define NUR_NODISCARD_TYPE __attribute__((warn_unused_result))
#  else
#    define NUR_NODISCARD_TYPE
#  endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#  include <sal.h>
#  define NUR_NODISCARD _Check_return_
#  define NUR_NODISCARD_TYPE
#else
#  define NUR_NODISCARD
#  define NUR_NODISCARD_TYPE
#endif

/* typeof, where a compiler provides it.  Used only by NUR_IGNORE. */
#if defined(NUR_CONFIG_NO_EXTENSIONS)
#  define NUR_HAS_TYPEOF 0
#elif defined(__cplusplus)
#  define NUR_HAS_TYPEOF 1
#  define NUR_TYPEOF(x_) decltype(x_)
#elif defined(__GNUC__) || defined(__clang__)
#  define NUR_HAS_TYPEOF 1
#  define NUR_TYPEOF(x_) __typeof__(x_)
#else
#  define NUR_HAS_TYPEOF 0
#endif

/* Source location captured at the call site, per NUR_CONFIG_SOURCE_LOCATION.
 * Expands to three arguments: file, function, line.  Internal. */
#if NUR_CONFIG_SOURCE_LOCATION == NUR_SOURCE_LOCATION_FULL
#  define NUR_PRIV_LOC __FILE__, NUR_FUNC, (unsigned)__LINE__
#elif NUR_CONFIG_SOURCE_LOCATION == NUR_SOURCE_LOCATION_BASIC
#  define NUR_PRIV_LOC __FILE__, NULL, (unsigned)__LINE__
#else
#  define NUR_PRIV_LOC NULL, NULL, 0u
#endif

/* Terminate the program without relying on any library when freestanding. */
#if NUR_HOSTED
#  define NUR_PRIV_ABORT() abort()
#elif defined(__GNUC__) || defined(__clang__)
#  define NUR_PRIV_ABORT() __builtin_trap()
#else
#  define NUR_PRIV_ABORT() for (;;) { }
#endif

/* ==========================================================================
 * 5. Error codes
 * ========================================================================== */

/**
 * Single source of truth for error codes: X(name, numeric value, description).
 *
 * The numeric values are part of the stable API: they may be logged, stored or
 * sent over a wire.  Append new codes at the end; never renumber or reuse.
 * Names are stable too; descriptions may be reworded.
 */
#define NUR_ERROR_CODE_LIST(X)                                                       \
    X(NUR_E_NONE,             0,  "no error")                                        \
    /* input and contract */                                                         \
    X(NUR_E_INVALID_ARGUMENT, 1,  "invalid argument")                                \
    X(NUR_E_NULL_ARGUMENT,    2,  "unexpected null argument")                        \
    X(NUR_E_OUT_OF_RANGE,     3,  "value out of range")                              \
    X(NUR_E_INVALID_STATE,    4,  "operation not valid in the current state")        \
    /* data */                                                                       \
    X(NUR_E_PARSE,            5,  "parse error")                                     \
    X(NUR_E_FORMAT,           6,  "malformed or unexpected format")                  \
    X(NUR_E_ENCODING,         7,  "invalid text encoding")                           \
    X(NUR_E_CORRUPTED,        8,  "data is corrupted")                               \
    /* filesystem and operating system */                                            \
    X(NUR_E_NOT_FOUND,        9,  "not found")                                       \
    X(NUR_E_ALREADY_EXISTS,   10, "already exists")                                  \
    X(NUR_E_PERMISSION,       11, "permission denied")                               \
    X(NUR_E_IO,               12, "input/output error")                              \
    X(NUR_E_SYSTEM,           13, "operating system error")                          \
    /* resources */                                                                  \
    X(NUR_E_OUT_OF_MEMORY,    14, "out of memory")                                   \
    X(NUR_E_RESOURCE_LIMIT,   15, "resource limit reached")                          \
    X(NUR_E_BUSY,             16, "resource busy")                                   \
    /* network */                                                                    \
    X(NUR_E_TIMEOUT,          17, "operation timed out")                             \
    X(NUR_E_CONNECTION,       18, "connection failed")                               \
    X(NUR_E_PROTOCOL,         19, "protocol violation")                              \
    X(NUR_E_DISCONNECTED,     20, "peer disconnected")                               \
    /* control flow */                                                               \
    X(NUR_E_CANCELLED,        21, "operation cancelled")                             \
    X(NUR_E_INTERRUPTED,      22, "operation interrupted")                           \
    /* application */                                                                \
    X(NUR_E_UNSUPPORTED,      23, "operation not supported")                         \
    X(NUR_E_INTERNAL,         24, "internal error")                                  \
    X(NUR_E_BUG,              25, "programming error (contract violation)")

/** Semantic, machine-readable error code.  Never a message, never an errno. */
typedef enum nur_error_code {
#define NUR_PRIV_X_ENUM(name_, value_, text_) name_ = value_,
    NUR_ERROR_CODE_LIST(NUR_PRIV_X_ENUM)
#undef NUR_PRIV_X_ENUM
} nur_error_code_t;

NUR_STATIC_ASSERT(NUR_E_NONE == 0, "a zero-initialised nur_error_t must mean 'no error'");

/**
 * Stable identifier of @p code, e.g. "NUR_E_NOT_FOUND".  Takes any integer, so a
 * code read back from a log or the wire is safe to look up; values that are not
 * in NUR_ERROR_CODE_LIST yield "NUR_E_UNKNOWN".
 */
NUR_API const char *nur_error_code_name(int code)
{
    switch (code) {
#define NUR_PRIV_X_NAME(name_, value_, text_) case name_: return #name_;
        NUR_ERROR_CODE_LIST(NUR_PRIV_X_NAME)
#undef NUR_PRIV_X_NAME
    default:
        break;
    }
    return "NUR_E_UNKNOWN";
}

/**
 * Default human-readable description of @p code (wording is not stable; never
 * parse it).  Takes any integer; unknown values yield "unknown error".
 */
NUR_API const char *nur_error_code_text(int code)
{
    switch (code) {
#define NUR_PRIV_X_TEXT(name_, value_, text_) case name_: return text_;
        NUR_ERROR_CODE_LIST(NUR_PRIV_X_TEXT)
#undef NUR_PRIV_X_TEXT
    default:
        break;
    }
    return "unknown error";
}

/* ==========================================================================
 * 6. Error object
 * ========================================================================== */

/**
 * A structured error.  Plain data: copy it freely, return it by value.
 *
 * All pointers are borrowed (see "Ownership" above) and may be NULL.
 * The layout does not depend on any configuration macro.
 */
typedef struct nur_error {
    nur_error_code_t code;     /**< Semantic code; branch on this.                  */
    int              system_code; /**< Native errno / Win32 value; 0 if none.       */
    const char      *message;  /**< Human-readable text; NULL selects the default.   */
    const char      *context;  /**< Short operation detail (e.g. a path); or NULL.   */
    const char      *file;     /**< Source file of the origin; NULL if disabled.     */
    const char      *function; /**< Source function of the origin; NULL if disabled. */
    unsigned         line;     /**< Source line of the origin; 0 if disabled.        */
} nur_error_t;

/** Fully explicit constructor.  Prefer the NUR_MAKE_ERROR* macros. */
NUR_API nur_error_t nur_error_make(nur_error_code_t code, int system_code,
                                   const char *message, const char *context,
                                   const char *file, const char *function,
                                   unsigned line)
{
    nur_error_t error;
    error.code        = code;
    error.system_code = system_code;
    error.message     = message;
    error.context     = context;
    error.file        = file;
    error.function    = function;
    error.line        = line;
    return error;
}

/** Error with source location captured at the call site. */
#define NUR_MAKE_ERROR(code_, message_) \
    nur_error_make((code_), 0, (message_), NULL, NUR_PRIV_LOC)

/** As NUR_MAKE_ERROR, plus a short operation context (e.g. the path involved). */
#define NUR_MAKE_ERROR_CTX(code_, message_, context_) \
    nur_error_make((code_), 0, (message_), (context_), NUR_PRIV_LOC)

/** As NUR_MAKE_ERROR, plus the native OS error value. */
#define NUR_MAKE_ERROR_SYS(code_, system_code_, message_) \
    nur_error_make((code_), (system_code_), (message_), NULL, NUR_PRIV_LOC)

/** Returns @p error with its context replaced (errors are immutable-style values). */
NUR_API nur_error_t nur_error_with_context(nur_error_t error, const char *context)
{
    error.context = context;
    return error;
}

/** Message of @p error, or the default description of its code if none was given. */
NUR_API const char *nur_error_message(const nur_error_t *error)
{
    if (error == NULL) {
        return nur_error_code_text(NUR_E_NONE);
    }
    return (error->message != NULL) ? error->message : nur_error_code_text(error->code);
}

/* ==========================================================================
 * 7. System error adapters (errno, Win32)
 *
 * Capture the native value immediately at the failing call, before anything
 * else can overwrite it.  The native value is preserved in system_code; the
 * semantic code is what callers should branch on.
 * ========================================================================== */

/**
 * Maps an errno value to a semantic code.  0 maps to NUR_E_NONE; values with no
 * mapping (or whose macro is unavailable on this platform) map to NUR_E_SYSTEM.
 * Each constant is probed with #ifdef, so the function is portable.
 */
NUR_API nur_error_code_t nur_error_code_from_errno(int errnum)
{
    if (errnum == 0) { return NUR_E_NONE; }

    /* invalid argument / state / range / encoding */
#ifdef EINVAL
    if (errnum == EINVAL) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef EBADF
    if (errnum == EBADF) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef EFAULT
    if (errnum == EFAULT) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef ENAMETOOLONG
    if (errnum == ENAMETOOLONG) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef ENOTDIR
    if (errnum == ENOTDIR) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef EISDIR
    if (errnum == EISDIR) { return NUR_E_INVALID_ARGUMENT; }
#endif
#ifdef ERANGE
    if (errnum == ERANGE) { return NUR_E_OUT_OF_RANGE; }
#endif
#ifdef EDOM
    if (errnum == EDOM) { return NUR_E_OUT_OF_RANGE; }
#endif
#ifdef EOVERFLOW
    if (errnum == EOVERFLOW) { return NUR_E_OUT_OF_RANGE; }
#endif
#ifdef ENOTEMPTY
    if (errnum == ENOTEMPTY) { return NUR_E_INVALID_STATE; }
#endif
#ifdef EILSEQ
    if (errnum == EILSEQ) { return NUR_E_ENCODING; }
#endif

    /* filesystem and operating system */
#ifdef ENOENT
    if (errnum == ENOENT) { return NUR_E_NOT_FOUND; }
#endif
#ifdef ENODEV
    if (errnum == ENODEV) { return NUR_E_NOT_FOUND; }
#endif
#ifdef EEXIST
    if (errnum == EEXIST) { return NUR_E_ALREADY_EXISTS; }
#endif
#ifdef EACCES
    if (errnum == EACCES) { return NUR_E_PERMISSION; }
#endif
#ifdef EPERM
    if (errnum == EPERM) { return NUR_E_PERMISSION; }
#endif
#ifdef EROFS
    if (errnum == EROFS) { return NUR_E_PERMISSION; }
#endif
#ifdef EIO
    if (errnum == EIO) { return NUR_E_IO; }
#endif

    /* resources */
#ifdef ENOMEM
    if (errnum == ENOMEM) { return NUR_E_OUT_OF_MEMORY; }
#endif
#ifdef EMFILE
    if (errnum == EMFILE) { return NUR_E_RESOURCE_LIMIT; }
#endif
#ifdef ENFILE
    if (errnum == ENFILE) { return NUR_E_RESOURCE_LIMIT; }
#endif
#ifdef ENOSPC
    if (errnum == ENOSPC) { return NUR_E_RESOURCE_LIMIT; }
#endif
#ifdef EDQUOT
    if (errnum == EDQUOT) { return NUR_E_RESOURCE_LIMIT; }
#endif
#ifdef EBUSY
    if (errnum == EBUSY) { return NUR_E_BUSY; }
#endif
#ifdef EAGAIN
    if (errnum == EAGAIN) { return NUR_E_BUSY; }
#endif
#ifdef EWOULDBLOCK
    if (errnum == EWOULDBLOCK) { return NUR_E_BUSY; }
#endif
#ifdef ETXTBSY
    if (errnum == ETXTBSY) { return NUR_E_BUSY; }
#endif
#ifdef EINPROGRESS
    if (errnum == EINPROGRESS) { return NUR_E_BUSY; }
#endif

    /* network */
#ifdef ETIMEDOUT
    if (errnum == ETIMEDOUT) { return NUR_E_TIMEOUT; }
#endif
#ifdef ECONNREFUSED
    if (errnum == ECONNREFUSED) { return NUR_E_CONNECTION; }
#endif
#ifdef ECONNABORTED
    if (errnum == ECONNABORTED) { return NUR_E_CONNECTION; }
#endif
#ifdef ENETUNREACH
    if (errnum == ENETUNREACH) { return NUR_E_CONNECTION; }
#endif
#ifdef EHOSTUNREACH
    if (errnum == EHOSTUNREACH) { return NUR_E_CONNECTION; }
#endif
#ifdef ECONNRESET
    if (errnum == ECONNRESET) { return NUR_E_DISCONNECTED; }
#endif
#ifdef ENOTCONN
    if (errnum == ENOTCONN) { return NUR_E_DISCONNECTED; }
#endif
#ifdef EPIPE
    if (errnum == EPIPE) { return NUR_E_DISCONNECTED; }
#endif
#ifdef EPROTO
    if (errnum == EPROTO) { return NUR_E_PROTOCOL; }
#endif
#ifdef EPROTONOSUPPORT
    if (errnum == EPROTONOSUPPORT) { return NUR_E_PROTOCOL; }
#endif

    /* control flow */
#ifdef ECANCELED
    if (errnum == ECANCELED) { return NUR_E_CANCELLED; }
#endif
#ifdef EINTR
    if (errnum == EINTR) { return NUR_E_INTERRUPTED; }
#endif

    /* application */
#ifdef ENOSYS
    if (errnum == ENOSYS) { return NUR_E_UNSUPPORTED; }
#endif
#ifdef ENOTSUP
    if (errnum == ENOTSUP) { return NUR_E_UNSUPPORTED; }
#endif
#ifdef EOPNOTSUPP
    if (errnum == EOPNOTSUPP) { return NUR_E_UNSUPPORTED; }
#endif

    return NUR_E_SYSTEM;
}

/**
 * Maps a Win32 / Winsock error number (GetLastError(), WSAGetLastError()) to a
 * semantic code.  Pure arithmetic: it needs no Windows headers and is therefore
 * testable on every platform.  Unmapped values yield NUR_E_SYSTEM.
 */
NUR_API nur_error_code_t nur_error_code_from_win32(unsigned long error)
{
    switch (error) {
    case 0UL:                                   /* ERROR_SUCCESS */
        return NUR_E_NONE;
    case 2UL: case 3UL: case 15UL: case 126UL: case 127UL: case 1168UL:
        /* FILE_NOT_FOUND, PATH_NOT_FOUND, INVALID_DRIVE, MOD_NOT_FOUND,
         * PROC_NOT_FOUND, NOT_FOUND */
        return NUR_E_NOT_FOUND;
    case 5UL: case 19UL: case 1314UL:
        /* ACCESS_DENIED, WRITE_PROTECT, PRIVILEGE_NOT_HELD */
        return NUR_E_PERMISSION;
    case 6UL: case 87UL: case 123UL: case 161UL: case 267UL:
        /* INVALID_HANDLE, INVALID_PARAMETER, INVALID_NAME, BAD_PATHNAME, DIRECTORY */
        return NUR_E_INVALID_ARGUMENT;
    case 8UL: case 14UL:
        /* NOT_ENOUGH_MEMORY, OUTOFMEMORY */
        return NUR_E_OUT_OF_MEMORY;
    case 4UL: case 39UL: case 112UL: case 122UL:
        /* TOO_MANY_OPEN_FILES, HANDLE_DISK_FULL, DISK_FULL, INSUFFICIENT_BUFFER */
        return NUR_E_RESOURCE_LIMIT;
    case 21UL: case 32UL: case 33UL: case 170UL: case 231UL: case 10035UL:
        /* NOT_READY, SHARING_VIOLATION, LOCK_VIOLATION, BUSY, PIPE_BUSY, WSAEWOULDBLOCK */
        return NUR_E_BUSY;
    case 23UL:                                  /* CRC */
        return NUR_E_CORRUPTED;
    case 38UL: case 1117UL:                     /* HANDLE_EOF, IO_DEVICE */
        return NUR_E_IO;
    case 50UL: case 120UL:                      /* NOT_SUPPORTED, CALL_NOT_IMPLEMENTED */
        return NUR_E_UNSUPPORTED;
    case 80UL: case 183UL:                      /* FILE_EXISTS, ALREADY_EXISTS */
        return NUR_E_ALREADY_EXISTS;
    case 145UL:                                 /* DIR_NOT_EMPTY */
        return NUR_E_INVALID_STATE;
    case 109UL: case 232UL: case 10054UL: case 10057UL:
        /* BROKEN_PIPE, NO_DATA, WSAECONNRESET, WSAENOTCONN */
        return NUR_E_DISCONNECTED;
    case 121UL: case 258UL: case 1460UL: case 10060UL:
        /* SEM_TIMEOUT, WAIT_TIMEOUT, TIMEOUT, WSAETIMEDOUT */
        return NUR_E_TIMEOUT;
    case 995UL: case 1223UL:                    /* OPERATION_ABORTED, CANCELLED */
        return NUR_E_CANCELLED;
    case 1225UL: case 1236UL: case 10051UL: case 10053UL: case 10061UL: case 10065UL:
        /* CONNECTION_REFUSED, CONNECTION_ABORTED, WSAENETUNREACH, WSAECONNABORTED,
         * WSAECONNREFUSED, WSAEHOSTUNREACH */
        return NUR_E_CONNECTION;
    case 10004UL:                               /* WSAEINTR */
        return NUR_E_INTERRUPTED;
    default:
        break;
    }
    return NUR_E_SYSTEM;
}

/** Error from an errno value.  The result never carries NUR_E_NONE. */
NUR_API nur_error_t nur_error_from_errno(int errnum, const char *message,
                                         const char *file, const char *function,
                                         unsigned line)
{
    nur_error_code_t code = nur_error_code_from_errno(errnum);
    if (code == NUR_E_NONE) {
        code = NUR_E_SYSTEM;
    }
    return nur_error_make(code, errnum, message, NULL, file, function, line);
}

/** Error from a Win32 error number.  The result never carries NUR_E_NONE. */
NUR_API nur_error_t nur_error_from_win32(unsigned long error, const char *message,
                                         const char *file, const char *function,
                                         unsigned line)
{
    nur_error_code_t code = nur_error_code_from_win32(error);
    if (code == NUR_E_NONE) {
        code = NUR_E_SYSTEM;
    }
    return nur_error_make(code, (int)error, message, NULL, file, function, line);
}

/**
 * Error from an explicit errno value, with call-site source location:
 *     int saved = errno;  cleanup();  return NUR_ERR(R, NUR_ERROR_FROM_ERRNO_VALUE(saved, "..."));
 */
#define NUR_ERROR_FROM_ERRNO_VALUE(errnum_, message_) \
    nur_error_from_errno((errnum_), (message_), NUR_PRIV_LOC)

#if NUR_HAS_ERRNO
/**
 * Error from the current errno.  Use it immediately after the failing call and
 * pass a side-effect-free message (argument evaluation order is unspecified).
 */
#  define NUR_ERROR_FROM_ERRNO(message_) NUR_ERROR_FROM_ERRNO_VALUE(errno, (message_))
#endif

/** Error from a Win32 error number, e.g. NUR_ERROR_FROM_WIN32(GetLastError(), "..."). */
#define NUR_ERROR_FROM_WIN32(error_, message_) \
    nur_error_from_win32((unsigned long)(error_), (message_), NUR_PRIV_LOC)

/* ==========================================================================
 * 8. Error formatting (allocation-free, no stdio dependency)
 * ========================================================================== */

typedef struct nur_priv_sink {
    char  *buffer;
    size_t capacity;
    size_t length; /* logical length: keeps counting past the capacity */
} nur_priv_sink;

NUR_API void nur_priv_sink_putc(nur_priv_sink *sink, char c)
{
    if (sink->buffer != NULL && sink->capacity != 0u && sink->length < sink->capacity - 1u) {
        sink->buffer[sink->length] = c;
    }
    ++sink->length;
}

NUR_API void nur_priv_sink_puts(nur_priv_sink *sink, const char *text)
{
    while (*text != '\0') {
        nur_priv_sink_putc(sink, *text);
        ++text;
    }
}

NUR_API void nur_priv_sink_putu(nur_priv_sink *sink, unsigned long long value)
{
    char digits[24];
    size_t count = 0u;
    do {
        digits[count++] = (char)('0' + (int)(value % 10u));
        value /= 10u;
    } while (value != 0u);
    while (count > 0u) {
        nur_priv_sink_putc(sink, digits[--count]);
    }
}

NUR_API void nur_priv_sink_puti(nur_priv_sink *sink, long long value)
{
    unsigned long long magnitude;
    if (value < 0) {
        nur_priv_sink_putc(sink, '-');
        magnitude = 0ull - (unsigned long long)value;
    } else {
        magnitude = (unsigned long long)value;
    }
    nur_priv_sink_putu(sink, magnitude);
}

/**
 * Renders @p error as one line into @p buffer:
 *
 *     NUR_E_NOT_FOUND: could not open file [config.ini] (system code 2) at main.c:42 in load()
 *
 * Context, system code and location are omitted when absent.  At most
 * capacity-1 characters are written and the result is always NUL-terminated
 * when capacity > 0.  @p buffer may be NULL when capacity is 0.
 *
 * @return the length the full text needs, excluding the NUL (snprintf semantics);
 *         a value >= capacity means the output was truncated.
 */
NUR_API size_t nur_error_format(const nur_error_t *error, char *buffer, size_t capacity)
{
    nur_priv_sink sink;
    nur_error_t none;

    sink.buffer   = buffer;
    sink.capacity = capacity;
    sink.length   = 0u;

    if (error == NULL) {
        none  = nur_error_make(NUR_E_NONE, 0, NULL, NULL, NULL, NULL, 0u);
        error = &none;
    }

    nur_priv_sink_puts(&sink, nur_error_code_name(error->code));
    nur_priv_sink_puts(&sink, ": ");
    nur_priv_sink_puts(&sink, nur_error_message(error));

    if (error->context != NULL && error->context[0] != '\0') {
        nur_priv_sink_puts(&sink, " [");
        nur_priv_sink_puts(&sink, error->context);
        nur_priv_sink_putc(&sink, ']');
    }
    if (error->system_code != 0) {
        nur_priv_sink_puts(&sink, " (system code ");
        nur_priv_sink_puti(&sink, (long long)error->system_code);
        nur_priv_sink_putc(&sink, ')');
    }
    if (error->file != NULL) {
        nur_priv_sink_puts(&sink, " at ");
        nur_priv_sink_puts(&sink, error->file);
        nur_priv_sink_putc(&sink, ':');
        nur_priv_sink_putu(&sink, (unsigned long long)error->line);
        if (error->function != NULL) {
            nur_priv_sink_puts(&sink, " in ");
            nur_priv_sink_puts(&sink, error->function);
            nur_priv_sink_puts(&sink, "()");
        }
    }

    if (buffer != NULL && capacity != 0u) {
        buffer[(sink.length < capacity) ? sink.length : (capacity - 1u)] = '\0';
    }
    return sink.length;
}

#if NUR_HAS_STDIO
/**
 * Writes the formatted error plus a newline to @p stream (stderr if NULL).
 * Output beyond NUR_CONFIG_PRINT_BUFFER_SIZE is truncated.  Printing is always
 * explicit: nur.h never logs on its own.
 */
NUR_API void nur_error_print(FILE *stream, const nur_error_t *error)
{
    char line[NUR_CONFIG_PRINT_BUFFER_SIZE];
    const size_t capacity = sizeof line - 1u;       /* reserve one byte for '\n' */
    const size_t needed = nur_error_format(error, line, capacity);
    const size_t written = (needed < capacity) ? needed : (capacity - 1u);

    line[written]      = '\n';
    line[written + 1u] = '\0';
    (void)fputs(line, (stream != NULL) ? stream : stderr);
}
#endif

/* ==========================================================================
 * 9. Panic and assertions
 *
 * Panics are for violated invariants, never for expected failures.
 * ========================================================================== */

/**
 * Reports a fatal programming error and terminates.  Never returns.
 *
 * Default behaviour: print to stderr (when stdio is available), then abort().
 * Override it by defining NUR_PANIC_HANDLER; a handler that returns is aborted
 * anyway.  @p error, @p file and @p function may be NULL.
 */
NUR_NORETURN NUR_API void nur_panic_at(const char *message, const nur_error_t *error,
                                       const char *file, const char *function,
                                       unsigned line)
{
#if defined(NUR_PANIC_HANDLER)
    NUR_PANIC_HANDLER(message, error, file, function, line);
#elif NUR_HAS_STDIO
    (void)fprintf(stderr, "\n[nur] panic: %s\n", (message != NULL) ? message : "(no message)");
    if (error != NULL) {
        (void)fputs("[nur]   cause: ", stderr);
        nur_error_print(stderr, error);
    }
    if (file != NULL) {
        (void)fprintf(stderr, "[nur]   at %s:%u", file, line);
        if (function != NULL) {
            (void)fprintf(stderr, " in %s()", function);
        }
        (void)fputc('\n', stderr);
    }
    (void)fflush(stderr);
#else
    (void)message;
    (void)error;
    (void)file;
    (void)function;
    (void)line;
#endif
    NUR_PRIV_ABORT();
}

/** Terminates with @p message (a C string).  Use for unrecoverable states. */
#define NUR_PANIC(message_) nur_panic_at((message_), NULL, NUR_PRIV_LOC)

/** Marks code that must never execute (e.g. an impossible switch default). */
#define NUR_UNREACHABLE() NUR_PANIC("unreachable code was executed")

#if NUR_CONFIG_ASSERTS
/**
 * Contract check.  Terminates if @p cond_ is false.  When assertions are
 * disabled (NDEBUG), @p cond_ is NOT evaluated but still has to compile.
 */
#  define NUR_ASSERT(cond_) \
    (NUR_LIKELY(cond_) ? (void)0 : NUR_PANIC("assertion failed: " #cond_))
/** As NUR_ASSERT with an explanation; @p msg_ must be a string literal. */
#  define NUR_ASSERT_MSG(cond_, msg_) \
    (NUR_LIKELY(cond_) ? (void)0 : NUR_PANIC("assertion failed: " #cond_ " (" msg_ ")"))
#else
#  define NUR_ASSERT(cond_)           ((void)sizeof(cond_))
#  define NUR_ASSERT_MSG(cond_, msg_) ((void)sizeof(cond_))
#endif

/* ==========================================================================
 * 10. Result and Status
 * ========================================================================== */

/** Discriminant of a Result.  ERR is zero so that zero-initialisation fails closed. */
typedef enum nur_result_state {
    NUR_RESULT_ERR = 0,
    NUR_RESULT_OK  = 1
} nur_result_state_t;

NUR_STATIC_ASSERT((int)NUR_RESULT_ERR == 0, "a zero-initialised result must be an error");

/**
 * Defines the result type @p Name carrying a @p Type payload, plus its helpers.
 * Use at file scope, once per type, followed by a semicolon:
 *
 *     NUR_DEFINE_RESULT(FILE *, file_result);
 *
 * @p Type must be a single type name: typedef function pointers and anything
 * else containing top-level commas first.  The generated members are:
 *
 *     state                 NUR_RESULT_OK or NUR_RESULT_ERR
 *     data.value            valid iff state == NUR_RESULT_OK
 *     data.error            valid iff state == NUR_RESULT_ERR
 *
 * and the helpers Name_ok, Name_err, Name_unwrap_at, Name_unwrap_or (use them
 * through NUR_OK, NUR_ERR, NUR_UNWRAP, NUR_EXPECT, NUR_UNWRAP_OR).
 */
#define NUR_DEFINE_RESULT(Type_, Name_)                                                \
    typedef struct NUR_NODISCARD_TYPE Name_ {                                          \
        nur_result_state_t state;                                                      \
        union {                                                                        \
            Type_       value;                                                         \
            nur_error_t error;                                                         \
        } data;                                                                        \
    } Name_;                                                                           \
    NUR_API Name_ NUR_CAT(Name_, _ok)(Type_ value_)                                    \
    {                                                                                  \
        Name_ result_;                                                                 \
        result_.state      = NUR_RESULT_OK;                                            \
        result_.data.value = value_;                                                   \
        return result_;                                                                \
    }                                                                                  \
    NUR_API Name_ NUR_CAT(Name_, _err)(nur_error_t error_)                             \
    {                                                                                  \
        Name_ result_;                                                                 \
        result_.state      = NUR_RESULT_ERR;                                           \
        result_.data.error = error_;                                                   \
        return result_;                                                                \
    }                                                                                  \
    NUR_API Type_ NUR_CAT(Name_, _unwrap_at)(Name_ result_, const char *message_,      \
                                             const char *file_, const char *function_, \
                                             unsigned line_)                           \
    {                                                                                  \
        if (NUR_UNLIKELY(result_.state != NUR_RESULT_OK)) {                            \
            nur_panic_at((message_ != NULL) ? message_ : "NUR_UNWRAP on an Err result",\
                         &result_.data.error, file_, function_, line_);                \
        }                                                                              \
        return result_.data.value;                                                     \
    }                                                                                  \
    NUR_API Type_ NUR_CAT(Name_, _unwrap_or)(Name_ result_, Type_ fallback_)           \
    {                                                                                  \
        return (result_.state == NUR_RESULT_OK) ? result_.data.value : fallback_;      \
    }                                                                                  \
    NUR_STATIC_ASSERT(sizeof(Name_) >= sizeof(nur_error_t),                            \
                      "NUR_DEFINE_RESULT: a result must be able to carry a nur_error_t")

/** Builds a successful result.  @p value_ is evaluated once and converted to the payload
 *  type exactly as for an ordinary function argument. */
#define NUR_OK(Name_, value_) NUR_CAT(Name_, _ok)(value_)

/** Builds a failed result from a nur_error_t.  @p error_ is evaluated once. */
#define NUR_ERR(Name_, error_) NUR_CAT(Name_, _err)(error_)

/** Builds a failed result with a code and message; location is captured here. */
#define NUR_FAIL(Name_, code_, message_) \
    NUR_CAT(Name_, _err)(NUR_MAKE_ERROR((code_), (message_)))

/** True if @p r_ holds a value.  Anything not exactly OK counts as an error. */
#define NUR_IS_OK(r_)  ((r_).state == NUR_RESULT_OK)
/** True if @p r_ holds an error. */
#define NUR_IS_ERR(r_) ((r_).state != NUR_RESULT_OK)
/** Raw access to the payload.  Valid only if NUR_IS_OK.  Does no checking. */
#define NUR_VALUE(r_)  ((r_).data.value)
/** Raw access to the error.  Valid only if NUR_IS_ERR.  Does no checking. */
#define NUR_ERROR(r_)  ((r_).data.error)

/**
 * Payload of the result expression @p expr_, or terminate with the error as the
 * cause.  A deliberate "this cannot fail here" assertion, not error handling.
 * Evaluates @p expr_ once; a result of any other type is a compile error.
 */
#define NUR_UNWRAP(Name_, expr_) \
    NUR_CAT(Name_, _unwrap_at)((expr_), NULL, NUR_PRIV_LOC)

/** As NUR_UNWRAP with a message explaining why failure is impossible. */
#define NUR_EXPECT(Name_, expr_, message_) \
    NUR_CAT(Name_, _unwrap_at)((expr_), (message_), NUR_PRIV_LOC)

/** Payload of @p expr_, or @p fallback_ on error.  Both arguments are evaluated. */
#define NUR_UNWRAP_OR(Name_, expr_, fallback_) \
    NUR_CAT(Name_, _unwrap_or)((expr_), (fallback_))

/** Payload type of Status: carries no information. */
typedef struct nur_unit {
    char reserved_;
} nur_unit_t;

/** The only value of nur_unit_t.  (Not named nur_unit: in C++ that would hide the tag.) */
NUR_API nur_unit_t nur_unit_value(void)
{
    nur_unit_t unit;
    unit.reserved_ = 0;
    return unit;
}

/**
 * Status: the outcome of an operation that returns no value (Result<void>).
 * It is an ordinary result, so NUR_IS_OK, NUR_ERROR, NUR_TRY, ... all apply, and
 * failures are built the usual way: NUR_FAIL(nur_status, NUR_E_BUSY, "device busy").
 */
NUR_DEFINE_RESULT(nur_unit_t, nur_status);
typedef nur_status nur_status_t;

/** Successful status. */
#define NUR_STATUS_OK() nur_status_ok(nur_unit_value())
/** Failed status from a nur_error_t. */
#define NUR_STATUS_ERR(error_) nur_status_err(error_)

/* ==========================================================================
 * 11. Option
 * ========================================================================== */

NUR_API void nur_priv_zero(void *destination, size_t size)
{
    unsigned char *bytes = (unsigned char *)destination;
    while (size > 0u) {
        *bytes++ = 0u;
        --size;
    }
}

/**
 * Defines the option type @p Name carrying a @p Type payload.  Option models
 * absence that is NOT a failure (e.g. "no such key"); use Result when the
 * reason matters.  Use at file scope, once per type, followed by a semicolon.
 *
 *     is_some   true iff value is meaningful (None is all-zero)
 *     value     the payload
 */
#define NUR_DEFINE_OPTION(Type_, Name_)                                                \
    typedef struct NUR_NODISCARD_TYPE Name_ {                                          \
        bool  is_some;                                                                 \
        Type_ value;                                                                   \
    } Name_;                                                                           \
    NUR_API Name_ NUR_CAT(Name_, _some)(Type_ value_)                                  \
    {                                                                                  \
        Name_ option_;                                                                 \
        option_.is_some = true;                                                        \
        option_.value   = value_;                                                      \
        return option_;                                                                \
    }                                                                                  \
    NUR_API Name_ NUR_CAT(Name_, _none)(void)                                          \
    {                                                                                  \
        Name_ option_;                                                                 \
        nur_priv_zero(&option_, sizeof option_);                                       \
        return option_;                                                                \
    }                                                                                  \
    NUR_API Type_ NUR_CAT(Name_, _unwrap_at)(Name_ option_, const char *file_,         \
                                             const char *function_, unsigned line_)    \
    {                                                                                  \
        if (NUR_UNLIKELY(!option_.is_some)) {                                          \
            nur_panic_at("NUR_OPTION_UNWRAP on a None option", NULL,                   \
                         file_, function_, line_);                                     \
        }                                                                              \
        return option_.value;                                                          \
    }                                                                                  \
    NUR_API Type_ NUR_CAT(Name_, _unwrap_or)(Name_ option_, Type_ fallback_)           \
    {                                                                                  \
        return option_.is_some ? option_.value : fallback_;                            \
    }                                                                                  \
    NUR_STATIC_ASSERT(sizeof(Name_) >= sizeof(Type_),                                  \
                      "NUR_DEFINE_OPTION: an option must be able to carry its payload")

/** Builds Some(value).  @p value_ is evaluated once and converted like a function argument. */
#define NUR_SOME(Name_, value_) NUR_CAT(Name_, _some)(value_)
/** Builds None. */
#define NUR_NONE(Name_) NUR_CAT(Name_, _none)()

/** True if @p o_ holds a value. */
#define NUR_IS_SOME(o_) ((o_).is_some)
/** True if @p o_ is empty. */
#define NUR_IS_NONE(o_) (!(o_).is_some)
/** Raw payload access.  Valid only if NUR_IS_SOME.  Does no checking. */
#define NUR_OPTION_VALUE(o_) ((o_).value)

/** Payload of @p expr_, or terminate if it is None.  Evaluates @p expr_ once. */
#define NUR_OPTION_UNWRAP(Name_, expr_) \
    NUR_CAT(Name_, _unwrap_at)((expr_), NUR_PRIV_LOC)

/** Payload of @p expr_, or @p fallback_ if None.  Both arguments are evaluated. */
#define NUR_OPTION_UNWRAP_OR(Name_, expr_, fallback_) \
    NUR_CAT(Name_, _unwrap_or)((expr_), (fallback_))

/* ==========================================================================
 * 12. Error propagation
 *
 * C has no `?` operator, so propagation names the result types involved.  That
 * keeps it portable, type-checked and free of compiler extensions.
 *
 *   NUR_TRY(R, expr)                  same type, discard the value
 *   NUR_TRY_ASSIGN(R, out, expr)      same type, store the value in `out`
 *   NUR_TRY_AS(Ret, R, expr)          convert the error into result type Ret
 *   NUR_TRY_ASSIGN_AS(Ret, R, out, expr)
 *
 * R is the type of `expr`; Ret is the return type of the enclosing function.
 * Each macro is one statement (safe in an unbraced `if`), evaluates `expr` and
 * `out` once, and uses a unique temporary, so it nests and never shadows.
 * Because they `return`, use them only where an early return is correct, i.e.
 * where nothing needs cleaning up (acquire resources after the last TRY, or
 * route through a single cleanup label by hand).
 * ========================================================================== */

#define NUR_PRIV_TRY(tmp_, R_, expr_)                                                  \
    do {                                                                               \
        R_ tmp_ = (expr_);                                                             \
        if (NUR_UNLIKELY(!NUR_IS_OK(tmp_))) {                                          \
            return tmp_;                                                               \
        }                                                                              \
    } while (0)

#define NUR_PRIV_TRY_ASSIGN(tmp_, R_, out_, expr_)                                     \
    do {                                                                               \
        R_ tmp_ = (expr_);                                                             \
        if (NUR_UNLIKELY(!NUR_IS_OK(tmp_))) {                                          \
            return tmp_;                                                               \
        }                                                                              \
        (out_) = tmp_.data.value;                                                      \
    } while (0)

#define NUR_PRIV_TRY_AS(tmp_, Ret_, R_, expr_)                                         \
    do {                                                                               \
        R_ tmp_ = (expr_);                                                             \
        if (NUR_UNLIKELY(!NUR_IS_OK(tmp_))) {                                          \
            return NUR_ERR(Ret_, tmp_.data.error);                                     \
        }                                                                              \
    } while (0)

#define NUR_PRIV_TRY_ASSIGN_AS(tmp_, Ret_, R_, out_, expr_)                            \
    do {                                                                               \
        R_ tmp_ = (expr_);                                                             \
        if (NUR_UNLIKELY(!NUR_IS_OK(tmp_))) {                                          \
            return NUR_ERR(Ret_, tmp_.data.error);                                     \
        }                                                                              \
        (out_) = tmp_.data.value;                                                      \
    } while (0)

/** Evaluate @p expr_ (of type R_); on error return it unchanged.  The enclosing
 *  function must return R_. */
#define NUR_TRY(R_, expr_) \
    NUR_PRIV_TRY(NUR_UNIQUE(nur_try_), R_, expr_)

/** As NUR_TRY, and on success assign the payload to the lvalue @p out_. */
#define NUR_TRY_ASSIGN(R_, out_, expr_) \
    NUR_PRIV_TRY_ASSIGN(NUR_UNIQUE(nur_try_), R_, out_, expr_)

/** As NUR_TRY, but the enclosing function returns Ret_: the error is carried over. */
#define NUR_TRY_AS(Ret_, R_, expr_) \
    NUR_PRIV_TRY_AS(NUR_UNIQUE(nur_try_), Ret_, R_, expr_)

/** As NUR_TRY_ASSIGN, but the enclosing function returns Ret_. */
#define NUR_TRY_ASSIGN_AS(Ret_, R_, out_, expr_) \
    NUR_PRIV_TRY_ASSIGN_AS(NUR_UNIQUE(nur_try_), Ret_, R_, out_, expr_)

/* ==========================================================================
 * 13. Explicit discard
 * ========================================================================== */

#if NUR_HAS_TYPEOF
#  define NUR_PRIV_IGNORE(tmp_, expr_)                                                 \
    do {                                                                               \
        NUR_TYPEOF(expr_) tmp_ = (expr_);                                              \
        (void)tmp_;                                                                    \
    } while (0)
/**
 * Deliberately discards a result.  Silences nodiscard diagnostics, including
 * GCC's warn_unused_result, which a plain `(void)` cast does not.  Use it where
 * ignoring failure is intentional and reviewed.  Without typeof support (or with
 * NUR_CONFIG_NO_EXTENSIONS) it degrades to a `(void)` cast, which is enough for
 * C23, C++17, Clang and MSVC but not for GCC's GNU-attribute path.
 */
#  define NUR_IGNORE(expr_) NUR_PRIV_IGNORE(NUR_UNIQUE(nur_ignored_), expr_)
#else
#  define NUR_IGNORE(expr_) ((void)(expr_))
#endif

/* ==========================================================================
 * 14. Predefined instantiations
 * ========================================================================== */

#if !defined(NUR_CONFIG_NO_BUILTIN_TYPES)
NUR_DEFINE_RESULT(bool,         nur_result_bool);
NUR_DEFINE_RESULT(int,          nur_result_int);
NUR_DEFINE_RESULT(long,         nur_result_long);
NUR_DEFINE_RESULT(unsigned,     nur_result_uint);
NUR_DEFINE_RESULT(size_t,       nur_result_size);
NUR_DEFINE_RESULT(int64_t,      nur_result_i64);
NUR_DEFINE_RESULT(uint64_t,     nur_result_u64);
NUR_DEFINE_RESULT(double,       nur_result_double);
NUR_DEFINE_RESULT(const char *, nur_result_cstr);
NUR_DEFINE_RESULT(void *,       nur_result_ptr);

NUR_DEFINE_OPTION(int,          nur_option_int);
NUR_DEFINE_OPTION(size_t,       nur_option_size);
NUR_DEFINE_OPTION(double,       nur_option_double);
NUR_DEFINE_OPTION(const char *, nur_option_cstr);
NUR_DEFINE_OPTION(void *,       nur_option_ptr);
#endif

#endif /* NUR_H */
