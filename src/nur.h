/*
 * nur.h - Rust-style Result<T,E> error handling for ISO C
 * 
 * A single-file, header-only, zero-allocation C library that brings
 * safe error handling to C through tagged union types.
 *
 * Key Features:
 * - NurResult<T>: tagged union forcing error acknowledgment 
 * - Zero heap allocation in core library
 * - Multi-standard: C99/C11/C17/C23 compatible
 * - Header-only: all functions are static inline
 * - Cross-platform: GCC, Clang, MSVC support
 * - [[nodiscard]] attributes for compile-time safety
 * - 36 error codes with systematic taxonomy (+ NONE, USER_BASE)
 * - Source location tracking (file, function, line)
 * - Error chaining via NUR_WRAP_ERROR
 * - Optional native stacktraces via NUR_ENABLE_BACKTRACE
 * - C23 typeof type deduction via NUR_TRY_AUTO
 * - errno integration for system errors
 *
 * Copyright (c) 2024-2026 Irene Al-Caligo
 * Author: Irene Al-Caligo
 * SPDX-License-Identifier: MIT
 */

#ifndef NUR_H_INCLUDED
#define NUR_H_INCLUDED

#if defined(_MSC_VER)
#  pragma warning(disable: 4996)
#  pragma warning(disable: 4714)
#  pragma warning(disable: 4702)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wgnu-auto-type"
#  pragma clang diagnostic ignored "-Wgnu-statement-expression"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * VERSION INFORMATION
 *===========================================================================*/
#define NUR_VERSION_MAJOR 1
#define NUR_VERSION_MINOR 4
#define NUR_VERSION_PATCH 0
#define NUR_VERSION_STRING "1.4.0"
#define NUR_VERSION_NUMBER 10400

#ifdef NUR_LITE
    #define NUR_NO_EXTENSIONS 1
    #define NUR_NO_SECURITY_SHIELD 1
#endif

/*============================================================================
 * STANDARD DETECTION
 *===========================================================================*/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define NUR_C23 1
    #define NUR_C17 1
    #define NUR_C11 1  
    #define NUR_C99 1
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    #define NUR_C17 1
    #define NUR_C11 1
    #define NUR_C99 1
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define NUR_C11 1
    #define NUR_C99 1
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    #define NUR_C99 1
#else
    #define NUR_C89 1
#endif

/*============================================================================
 * COMPILER DETECTION
 *===========================================================================*/
#if defined(__GNUC__) || defined(__clang__)
    #define NUR_GNU_EXTENSIONS 1
#endif

#ifdef _MSC_VER
    #define NUR_MSVC 1
#endif

/* Type deduction macros */
#ifdef __cplusplus
    #define NUR_TYPEOF(expr) decltype(expr)
    #define NUR_AUTO auto
#elif defined(NUR_C23)
    #define NUR_TYPEOF(expr) typeof(expr)
    #define NUR_AUTO auto
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_TYPEOF(expr) __extension__ __typeof__(expr)
    #define NUR_AUTO __extension__ __auto_type
#endif

/*============================================================================
 * FREESTANDING & REQUIRED HEADERS
 *===========================================================================*/
#ifdef NUR_FREESTANDING
    #ifndef NUR_NO_ERRNO
        #define NUR_NO_ERRNO 1
    #endif
    #ifndef NUR_NO_STRING
        #define NUR_NO_STRING 1
    #endif
    #ifndef NUR_NO_STDIO
        #define NUR_NO_STDIO 1
    #endif
    #ifndef NUR_NO_STDLIB
        #define NUR_NO_STDLIB 1
    #endif
    #ifndef NUR_NO_CTYPE
        #define NUR_NO_CTYPE 1
    #endif
#endif

#if defined(NUR_C89) && !defined(__cplusplus)
    #ifndef __bool_true_false_are_defined
        typedef int bool;
        #define true 1
        #define false 0
        #define __bool_true_false_are_defined 1
    #endif
    #include <stddef.h>
    #if defined(NUR_NO_STDINT)
        typedef signed char        int8_t;
        typedef short              int16_t;
        typedef long               int32_t;
        typedef unsigned char      uint8_t;
        typedef unsigned short     uint16_t;
        typedef unsigned long      uint32_t;
        #if defined(__GNUC__) || defined(_MSC_VER)
            typedef long long          int64_t;
            typedef unsigned long long uint64_t;
        #endif
    #else
        #include <stdint.h>
    #endif
#else
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
#endif

#ifndef INT32_MAX
    #define INT32_MAX 2147483647L
#endif
#ifndef INT32_MIN
    #define INT32_MIN (-2147483647L - 1L)
#endif

#ifndef NUR_NO_ERRNO
    #include <errno.h>
#endif

#ifndef NUR_NO_STRING
    #include <string.h>
#endif

#ifndef NUR_NO_STDIO
    #include <stdio.h>
    #include <stdarg.h>
#endif

#ifndef NUR_NO_STDLIB
    #include <stdlib.h>
#endif

#ifndef NUR_NO_LIMITS
    #include <limits.h>
#endif

#ifndef NUR_NO_CTYPE
    #include <ctype.h>
#endif

/*============================================================================
 * ATTRIBUTE MACROS
 *===========================================================================*/

/* [[nodiscard]] - warns when result is ignored */
#if defined(__cplusplus) && __cplusplus >= 201703L
    #define NUR_NODISCARD [[nodiscard]]
    #define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
    #define NUR_NODISCARD_TYPE [[nodiscard]]
#elif defined(NUR_C23)
    #if defined(__has_c_attribute)
        #if __has_c_attribute(nodiscard)
            #define NUR_NODISCARD [[nodiscard]]
            #define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
            #define NUR_NODISCARD_TYPE [[nodiscard]]
        #else
            #define NUR_NODISCARD
            #define NUR_NODISCARD_MSG(msg)
            #define NUR_NODISCARD_TYPE
        #endif
    #else
        #define NUR_NODISCARD [[nodiscard]]
        #define NUR_NODISCARD_MSG(msg) [[nodiscard]]
        #define NUR_NODISCARD_TYPE [[nodiscard]]
    #endif
#elif defined(__clang__)
    #define NUR_NODISCARD __attribute__((warn_unused_result))
    #define NUR_NODISCARD_MSG(msg) __attribute__((warn_unused_result))
    #define NUR_NODISCARD_TYPE __attribute__((warn_unused_result))
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_NODISCARD __attribute__((warn_unused_result))
    #define NUR_NODISCARD_MSG(msg) __attribute__((warn_unused_result))
    #define NUR_NODISCARD_TYPE
#elif defined(NUR_MSVC)
    #define NUR_NODISCARD _Check_return_
    #define NUR_NODISCARD_MSG(msg) _Check_return_
    #define NUR_NODISCARD_TYPE
#else
    #define NUR_NODISCARD
    #define NUR_NODISCARD_MSG(msg)
    #define NUR_NODISCARD_TYPE
#endif

/* Function declaration helper enforcing nodiscard across all compilers */
#ifdef NUR_C89
    /* ISO C89 does not support variadic macros in pedantic mode */
#else
    #define NUR_FN(ReturnType, FuncName, ...) NUR_NODISCARD ReturnType FuncName(__VA_ARGS__)
#endif

/* Unused attribute to prevent -Wunused-function in headers */
#ifdef NUR_GNU_EXTENSIONS
    #define NUR_UNUSED __attribute__((unused))
#else
    #define NUR_UNUSED
#endif

/* Force inlining
 * NUR_INLINE does NOT use always_inline -- panic helpers are noinline+cold.
 * Removing always_inline avoids inlining the 1 KiB panic buffer at every
 * NUR_UNWRAP call site (~11x code size reduction). */
#ifdef NUR_C89
    #if defined(__GNUC__) || defined(__clang__)
        #define NUR_INLINE  static __inline__
        #define NUR_NOINLINE __attribute__((noinline))
    #elif defined(_MSC_VER)
        #define NUR_INLINE   static __inline
        #define NUR_NOINLINE __declspec(noinline)
    #else
        #define NUR_INLINE   static
        #define NUR_NOINLINE
    #endif
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_INLINE  static inline
    #define NUR_NOINLINE __attribute__((noinline))
#elif defined(NUR_MSVC)
    #define NUR_INLINE   static __inline
    #define NUR_NOINLINE __declspec(noinline)
#else
    #define NUR_INLINE   static inline
    #define NUR_NOINLINE
#endif

/* noreturn */
#ifdef NUR_C11
    #define NUR_NORETURN _Noreturn
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_NORETURN __attribute__((noreturn))
#elif defined(NUR_MSVC)
    #define NUR_NORETURN __declspec(noreturn)
#else
    #define NUR_NORETURN
#endif

/* cold - optimizer hint: path is rarely taken */
#ifdef NUR_GNU_EXTENSIONS
    #define NUR_COLD __attribute__((cold))
#else
    #define NUR_COLD
#endif

/* Source location tracking configuration */
#ifdef NUR_NO_SOURCE_LOCATION
    #define NUR_LOC_FILE ((const char *)0)
    #define NUR_LOC_FUNC ((const char *)0)
    #define NUR_LOC_LINE 0U
#elif defined(NUR_C89)
    #define NUR_LOC_FILE __FILE__
    #if (defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)) && !defined(__STRICT_ANSI__)
        #define NUR_LOC_FUNC __FUNCTION__
    #else
        #define NUR_LOC_FUNC "unknown"
    #endif
    #define NUR_LOC_LINE ((unsigned int)__LINE__)
#else
    #define NUR_LOC_FILE __FILE__
    #define NUR_LOC_FUNC __func__
    #define NUR_LOC_LINE ((unsigned int)__LINE__)
#endif

/* Branch prediction hints */
#ifdef NUR_GNU_EXTENSIONS
    #define NUR_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define NUR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define NUR_LIKELY(x)   (x)
    #define NUR_UNLIKELY(x) (x)
#endif

/* Printf-style format checking */
#ifdef NUR_GNU_EXTENSIONS
    #define NUR_PRINTF_LIKE(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
#else
    #define NUR_PRINTF_LIKE(fmt_idx, va_idx)
#endif

/*============================================================================
 * ERROR CODE TAXONOMY
 *===========================================================================*/

/**
 * Error codes following a systematic taxonomy.
 * 
 * Ranges:
 *   1-9:    Input/validation errors
 *   10-19:  Data processing errors
 *   20-29:  Resource errors
 *   30-39:  I/O errors
 *   40-49:  Network/async errors
 *   50-59:  System errors
 *   >=1000: User-defined errors
 */
#ifdef NUR_GNU_EXTENSIONS
__extension__
#endif
typedef enum nur_error_code_t {
    NUR_E_NONE = 0,
    
    /* Input/Validation Errors (1-9) */
    NUR_E_INVALID_ARGUMENT = 1,
    NUR_E_OUT_OF_RANGE = 2,
    NUR_E_NULL_POINTER = 3,
    NUR_E_INVALID_STATE = 4,
    NUR_E_OVERFLOW = 5,
    NUR_E_UNDERFLOW = 6,
    
    /* Data Processing Errors (10-19) */
    NUR_E_PARSE = 10,
    NUR_E_FORMAT = 11,
    NUR_E_ENCODING = 12,
    NUR_E_CORRUPTED = 13,
    NUR_E_TRUNCATED = 14,
    NUR_E_CHECKSUM = 15,
    
    /* Resource Errors (20-29) */
    NUR_E_OUT_OF_MEMORY = 20,
    NUR_E_RESOURCE_LIMIT = 21,
    NUR_E_BUSY = 22,
    NUR_E_QUOTA = 23,
    NUR_E_DEADLOCK = 24,
    
    /* I/O Errors (30-39) */
    NUR_E_NOT_FOUND = 30,
    NUR_E_PERMISSION = 31,
    NUR_E_ALREADY_EXISTS = 32,
    NUR_E_IO = 33,
    NUR_E_EOF = 34,
    NUR_E_CLOSED = 35,
    NUR_E_WOULD_BLOCK = 36,
    
    /* Network/Async Errors (40-49) */
    NUR_E_TIMEOUT = 40,
    NUR_E_CANCELLED = 41,
    NUR_E_CONNECTION = 42,
    NUR_E_PROTOCOL = 43,
    NUR_E_AGAIN = 44,
    NUR_E_REFUSED = 45,
    
    /* System Errors (50-59) */
    NUR_E_UNSUPPORTED = 50,
    NUR_E_INTERNAL = 51,
    NUR_E_BUG = 52,
    NUR_E_VERSION = 53,
    NUR_E_UNINITIALIZED = 54,
    NUR_E_SYSTEM = 55,
    
    /* User-defined error codes start here */
    NUR_E_USER_BASE = 1000,
} nur_error_code_t;

/* Backward-compatibility and POSIX taxonomy alias */
#define NUR_E_PERMISSION_DENIED NUR_E_PERMISSION

/*============================================================================
 * ERROR STRUCTURE
 *===========================================================================*/

/**
 * Complete error information with source location tracking.
 * 
 * All fields are always present. Source location is captured automatically
 * by error constructor macros.
 *
 * MEMORY LIFETIME CONTRACT:
 * - .message, .context, .file, .func are BORROWED string pointers (const char*).
 * - They MUST either be string literals (static duration) or pointers that remain
 *   valid for the entire lifetime of the error consumer.
 * - The core library does NOT allocate heap memory and does NOT copy string contents.
 * - Passing ephemeral stack buffers (e.g. from sprintf into a local array) into
 *   .message or .context will cause use-after-free if the error escapes the stack frame.
 * 
 * Fields:
 *   code:    Machine-readable error code (enum)
 *   sys:     System error code (errno/Win32), 0 if none
 *   message: Human-readable description (static string, never NULL)
 *   context: Optional operation context (static string, may be NULL)
 *   cause:   Optional root cause or previous context (static string, may be NULL)
 *   file:    Source file where error was raised
 *   func:    Function where error was raised
 *   line:    Line number where error was raised
 */
#ifndef NUR_MAX_CONTEXT_DEPTH
#define NUR_MAX_CONTEXT_DEPTH 4
#endif

#ifndef NUR_ERROR_INLINE_MSG_SIZE
#define NUR_ERROR_INLINE_MSG_SIZE 64
#endif

#ifdef NUR_COMPACT_ERRORS
typedef struct {
    nur_error_code_t code;
    unsigned int     line;
    const char      *message;
} nur_error_t;
#else
typedef struct {
    nur_error_code_t code;     
    int              sys;      
    const char      *message;  
    const char      *context;  
    const char      *cause;    
    const char      *file;     
    const char      *func;     
    unsigned int     line;     
    unsigned char    context_count;
    const char      *context_chain[NUR_MAX_CONTEXT_DEPTH];
    char             inline_msg[NUR_ERROR_INLINE_MSG_SIZE];
#ifdef NUR_ENABLE_BACKTRACE
    void *trace[16];
    int trace_count;
#endif
} nur_error_t;
#endif

/*============================================================================
 * ERROR CONSTRUCTORS
 *===========================================================================*/

/**
 * Internal error constructor. Do not use directly - use the macros below.
 */
#if defined(NUR_ENABLE_BACKTRACE) && defined(_WIN32)
__declspec(dllimport) unsigned short __stdcall RtlCaptureStackBackTrace(unsigned long, unsigned long, void**, unsigned long*);
#elif defined(NUR_ENABLE_BACKTRACE) && (defined(__linux__) || defined(__APPLE__))
#include <execinfo.h>
#endif

/* NOLINTNEXTLINE(bugprone-easily-swappable-parameters) */
NUR_INLINE nur_error_t nur_internal_make_error(
    nur_error_code_t code, int sys,
    const char *message, const char *context,
    const char *file, const char *func, unsigned int line
) {
#ifdef NUR_COMPACT_ERRORS
    nur_error_t err;
    (void)sys; (void)context; (void)file; (void)func;
    err.code = code;
    err.line = line;
    err.message = message ? message : (context ? context : "unknown error");
    return err;
#else
    int i_;
    nur_error_t err;
    err.code = code;
    err.sys = sys;
    err.message = message;
    err.context = context;
    err.cause = (const char *)0;
    err.file = file;
    err.func = func;
    err.line = line;
    err.context_count = 0;
    err.inline_msg[0] = '\0';
    for (i_ = 0; i_ < NUR_MAX_CONTEXT_DEPTH; ++i_) {
        err.context_chain[i_] = (const char *)0;
    }
    if (context != (const char *)0) {
        err.context_chain[0] = context;
        err.context_count = 1;
    }
#ifdef NUR_ENABLE_BACKTRACE
#if defined(_WIN32)
    err.trace_count = RtlCaptureStackBackTrace(1, 16, err.trace, NULL);
#elif defined(__linux__) || defined(__APPLE__)
    err.trace_count = backtrace(err.trace, 16);
#else
    err.trace_count = 0;
#endif
#endif
    return err;
#endif
}

/**
 * Wrap an existing error with new context, PRESERVING original code, sys,
 * message, file, func, line, and backtrace.
 *
 * ROOT CAUSE & BREADCRUMB CHAINING:
 * - Automatically preserves initial origin in .cause.
 * - Stores full chain of contexts up to NUR_MAX_CONTEXT_DEPTH in .context_chain.
 * - Leaves original callstack trace untouched.
 *
 * Usage: NUR_WRAP_ERROR(NUR_ERROR_OF(res), "failed to parse config")
 */
#ifdef NUR_COMPACT_ERRORS
NUR_INLINE nur_error_t nur_error_with_context(nur_error_t e, const char *ctx) {
    if (ctx != (const char *)0) {
        e.message = ctx;
    }
    return e;
}

NUR_INLINE nur_error_t nur_error_with_cause(nur_error_t e, const char *ctx, const char *cause_str) {
    (void)cause_str;
    if (ctx != (const char *)0) {
        e.message = ctx;
    }
    return e;
}

#define NUR_WRAP_ERROR(err_val, new_ctx) \
    nur_error_with_context((err_val), (new_ctx))

#define NUR_ERROR_CAUSE(err) ((const char *)0)
#define NUR_ERROR_CONTEXT_COUNT(err) ((unsigned char)0)

NUR_INLINE const char *nur_error_context_at(const nur_error_t *err, size_t idx) {
    (void)err; (void)idx;
    return (const char *)0;
}
#define NUR_ERROR_CONTEXT_AT(err, idx) nur_error_context_at(&(err), (size_t)(idx))
#else
NUR_INLINE nur_error_t nur_error_with_context(nur_error_t e, const char *ctx) {
    if (ctx == (const char *)0) return e;

    if (e.cause == (const char *)0 && e.context != (const char *)0) {
        e.cause = e.context;
    }

    if (e.context_count < NUR_MAX_CONTEXT_DEPTH) {
        e.context_chain[e.context_count++] = ctx;
    } else {
        int i_;
        /* Preserve origin at [0], shift intermediate entries */
        for (i_ = 1; i_ < NUR_MAX_CONTEXT_DEPTH - 1; ++i_) {
            e.context_chain[i_] = e.context_chain[i_ + 1];
        }
        e.context_chain[NUR_MAX_CONTEXT_DEPTH - 1] = ctx;
    }

    e.context = ctx;
    return e;
}

/**
 * Explicitly wrap an error with a high-level context and specific root cause.
 */
/* NOLINTNEXTLINE(bugprone-easily-swappable-parameters) */
NUR_INLINE nur_error_t nur_error_with_cause(nur_error_t e, const char *ctx, const char *cause_str) {
    int i_;
    e.context = ctx;
    e.cause = cause_str;
    e.context_count = 0;
    for (i_ = 0; i_ < NUR_MAX_CONTEXT_DEPTH; ++i_) {
        e.context_chain[i_] = (const char *)0;
    }
    if (cause_str != (const char *)0) {
        e.context_chain[e.context_count++] = cause_str;
    }
    if (ctx != (const char *)0) {
        e.context_chain[e.context_count++] = ctx;
    }
    return e;
}

#define NUR_WRAP_ERROR(err_val, new_ctx) \
    nur_error_with_context((err_val), (new_ctx))

/**
 * Read the root cause of an error (NULL if no cause attached).
 */
#define NUR_ERROR_CAUSE(err) ((err).cause)
#define NUR_ERROR_CONTEXT_COUNT(err) ((err).context_count)

NUR_INLINE const char *nur_error_context_at(const nur_error_t *err, size_t idx) {
    if (err != (const nur_error_t *)0 && idx < (size_t)err->context_count) {
        return err->context_chain[idx];
    }
    return (const char *)0;
}
#define NUR_ERROR_CONTEXT_AT(err, idx) nur_error_context_at(&(err), (size_t)(idx))
#endif

/**
 * Wrap an error with both context and root cause.
 */
#define NUR_ERROR_CAUSED_BY(err_val, new_ctx, cause_val) \
    nur_error_with_cause((err_val), (new_ctx), (cause_val))

/**
 * Create an error with code and message, capturing source location.
 * 
 * Usage: NUR_ERROR(NUR_E_INVALID_ARGUMENT, "buffer size must be positive")
 */
#define NUR_ERROR(code, msg) \
    nur_internal_make_error((code), 0, (msg), NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

/**
 * Create an error with code, message, and context, capturing source location.
 * 
 * Usage: NUR_ERROR_CTX(NUR_E_PARSE, "invalid JSON", "reading config.json")
 */
#define NUR_ERROR_CTX(code, msg, ctx) \
    nur_internal_make_error((code), 0, (msg), (ctx), NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

/**
 * Compile-time literal enforcement:
 * Guarantees that msg is a string literal (e.g. "my error"), NOT an ephemeral
 * stack buffer. Fails compilation across GCC, Clang, and MSVC if a variable/buffer is passed.
 *
 * Usage: NUR_ERROR_LIT(NUR_E_INVALID_ARGUMENT, "port out of range")
 */
#define NUR_ERROR_LIT(code, literal_msg) \
    NUR_ERROR((code), ("" literal_msg))

/**
 * Compile-time literal enforcement for both message and context:
 *
 * Usage: NUR_ERROR_LIT_CTX(NUR_E_OUT_OF_RANGE, "port out of range", "port validator")
 */
#define NUR_ERROR_LIT_CTX(code, literal_msg, literal_ctx) \
    NUR_ERROR_CTX((code), ("" literal_msg), ("" literal_ctx))

/**
 * Safe dynamic message copy into inline buffer to eliminate dangling pointers.
 */
NUR_INLINE nur_error_t nur_error_copy_msg(nur_error_t e, const char *src_str) {
#ifndef NUR_COMPACT_ERRORS
    if (src_str != (const char *)0) {
        size_t i = 0;
        while (i < (NUR_ERROR_INLINE_MSG_SIZE - 1) && src_str[i] != '\0') {
            e.inline_msg[i] = src_str[i];
            i++;
        }
        e.inline_msg[i] = '\0';
        e.message = (const char *)0;
    }
#else
    (void)src_str;
#endif
    return e;
}

#define NUR_ERROR_COPY(code, dynamic_str) \
    nur_error_copy_msg(NUR_ERROR((code), (const char *)0), (dynamic_str))

#ifndef NUR_NO_STDIO
/**
 * Safe dynamic string formatting into inline buffer without dangling pointers.
 *
 * Usage:
 *   return NUR_ERR(i32, NUR_ERROR_FMT(NUR_E_OUT_OF_RANGE, "port %d exceeds %d", p, max_p));
 */
NUR_INLINE nur_error_t nur_internal_make_error_fmt(
    nur_error_code_t code, const char *file, const char *func, unsigned int line,
    const char *fmt, ...
) {
    nur_error_t err = nur_internal_make_error(code, 0, (const char *)0, (const char *)0, file, func, line);
#ifndef NUR_COMPACT_ERRORS
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(err.inline_msg, sizeof(err.inline_msg), fmt, args);
    va_end(args);
    err.inline_msg[sizeof(err.inline_msg) - 1] = '\0';
    err.message = (const char *)0;
#else
    (void)fmt;
#endif
    return err;
}

#ifndef NUR_C89
#define NUR_ERROR_FMT(code, fmt, ...) \
    nur_internal_make_error_fmt((code), NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE, (fmt), __VA_ARGS__)
#endif
#endif

/**
 * Create an I/O error from errno, capturing source location.
 * Captures errno immediately to prevent it from being overwritten.
 * 
 * Usage: NUR_ERROR_ERRNO("failed to open file")
 */
/* Fix: previously hardcoded NUR_E_IO; now maps errno to semantic code.
 * ENOENT->NOT_FOUND, EACCES->PERMISSION, ENOMEM->OUT_OF_MEMORY, etc. */
#define NUR_ERROR_ERRNO(msg) \
    nur_internal_make_error(nur_error_code_from_errno(errno), errno, \
        (msg), NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

/**
 * Create an error from errno with context, capturing source location.
 *
 * Usage: NUR_ERROR_ERRNO_CTX("failed to open", "config.json")
 */
#define NUR_ERROR_ERRNO_CTX(msg, ctx) \
    nur_internal_make_error(nur_error_code_from_errno(errno), errno, \
        (msg), (ctx), NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

/**
 * Create an error from a Win32 error code, capturing source location.
 * Fix: now uses nur_error_code_from_win32() for semantic mapping.
 *
 * Usage: NUR_ERROR_FROM_WIN32(GetLastError(), "failed to open handle")
 */
#define NUR_ERROR_FROM_WIN32(err_code, msg) \
    nur_internal_make_error(nur_error_code_from_win32((unsigned long)(err_code)), \
        (int)(err_code), (msg), NULL, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

/*============================================================================
 * RESULT TYPE GENERATOR
 *===========================================================================*/

/**
 * Generate a Result<T> type and its constructor functions.
 * 
 * Creates:
 *   - NurResult_<suffix>        The result type
 *   - nur_ok_<suffix>(val)      Success constructor
 *   - nur_err_<suffix>(err)     Failure constructor
 *
 * Usage at file scope:
 *   NUR_RESULT_TYPE(MyStruct, my_struct);
 */
/* Forward declarations of panic handlers for unwrap helpers */
static void nur_internal_panic_result(nur_error_t e, const char *file, unsigned int line);
static void nur_internal_panic_msg(const char *msg, nur_error_t e, const char *file, unsigned int line);

/**
 * Generate a Result<T, E> type with arbitrary error type E.
 *
 * Creates:
 *   - NurResult_<suffix>        The result type
 *   - nur_ok_<suffix>(val)      Success constructor
 *   - nur_err_<suffix>(err)     Failure constructor
 *   - nur_is_ok_<suffix>(r)     Success predicate
 *   - nur_is_err_<suffix>(r)    Failure predicate
 *   - nur_value_<suffix>(r)     Direct value accessor
 *   - nur_error_<suffix>(r)     Direct error accessor
 *   - nur_unwrap_or_<suffix>    Value extractor with fallback
 */
#define NUR_RESULT_TYPE_E(T, E, suffix) \
    typedef struct NUR_NODISCARD_TYPE NurResult_##suffix { \
        bool ok; \
        union { \
            T value; \
            E error; \
        } u; \
    } NurResult_##suffix; \
    \
    NUR_NODISCARD \
    NUR_INLINE NurResult_##suffix nur_ok_##suffix(T val_) { \
        NurResult_##suffix r_; \
        r_.ok = true; \
        r_.u.value = val_; \
        return r_; \
    } \
    \
    NUR_NODISCARD \
    NUR_INLINE NurResult_##suffix nur_err_##suffix(E err_) { \
        NurResult_##suffix r_; \
        r_.ok = false; \
        r_.u.error = err_; \
        return r_; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE bool nur_is_ok_##suffix(NurResult_##suffix r_) { \
        return r_.ok; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE bool nur_is_err_##suffix(NurResult_##suffix r_) { \
        return !r_.ok; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_value_##suffix(NurResult_##suffix r_) { \
        return r_.u.value; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE E nur_error_##suffix(NurResult_##suffix r_) { \
        return r_.u.error; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_unwrap_or_##suffix(NurResult_##suffix r_, T fallback_) { \
        return r_.ok ? r_.u.value : fallback_; \
    } \
    struct nur_internal_dummy_decl_e_##suffix

/**
 * Generate a canonical Result<T, nur_error_t> type.
 *
 * Usage at file scope:
 *   NUR_RESULT_TYPE(MyStruct, my_struct);
 */
#define NUR_RESULT_TYPE(T, suffix) \
    typedef struct NUR_NODISCARD_TYPE NurResult_##suffix { \
        bool ok; \
        union { \
            T value; \
            nur_error_t error; \
        } u; \
    } NurResult_##suffix; \
    \
    NUR_NODISCARD \
    NUR_INLINE NurResult_##suffix nur_ok_##suffix(T val_) { \
        NurResult_##suffix r_; \
        r_.ok = true; \
        r_.u.value = val_; \
        return r_; \
    } \
    \
    NUR_NODISCARD \
    NUR_INLINE NurResult_##suffix nur_err_##suffix(nur_error_t err_) { \
        NurResult_##suffix r_; \
        r_.ok = false; \
        r_.u.error = err_; \
        return r_; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE bool nur_is_ok_##suffix(NurResult_##suffix r_) { \
        return r_.ok; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE bool nur_is_err_##suffix(NurResult_##suffix r_) { \
        return !r_.ok; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_value_##suffix(NurResult_##suffix r_) { \
        return r_.u.value; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE nur_error_t nur_error_##suffix(NurResult_##suffix r_) { \
        return r_.u.error; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_unwrap_or_##suffix(NurResult_##suffix r_, T fallback_) { \
        return r_.ok ? r_.u.value : fallback_; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_unwrap_##suffix(NurResult_##suffix r_) { \
        if (NUR_UNLIKELY(!r_.ok)) { \
            nur_internal_panic_result(r_.u.error, NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        return r_.u.value; \
    } \
    \
    NUR_NODISCARD NUR_UNUSED \
    NUR_INLINE T nur_expect_##suffix(NurResult_##suffix r_, const char *msg_) { \
        if (NUR_UNLIKELY(!r_.ok)) { \
            nur_internal_panic_msg(msg_, r_.u.error, NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        return r_.u.value; \
    } \
    struct nur_internal_dummy_decl_##suffix

/*============================================================================
 * PREDEFINED RESULT TYPES
 *===========================================================================*/

/* Numeric types */
NUR_RESULT_TYPE(int32_t, i32);
NUR_RESULT_TYPE(int64_t, i64);
NUR_RESULT_TYPE(size_t, usize);
NUR_RESULT_TYPE(float, f32);
NUR_RESULT_TYPE(double, f64);
NUR_RESULT_TYPE(bool, bool);
NUR_RESULT_TYPE(uint32_t, u32);
NUR_RESULT_TYPE(uint64_t, u64);
NUR_RESULT_TYPE(int8_t, i8);
NUR_RESULT_TYPE(int16_t, i16);
NUR_RESULT_TYPE(uint8_t, u8);
NUR_RESULT_TYPE(uint16_t, u16);

/* Pointer types */
NUR_RESULT_TYPE(char *, cstr);
NUR_RESULT_TYPE(const char *, cstr_const);
NUR_RESULT_TYPE(void *, ptr);

/**
 * Void result type - represents success/failure without a value.
 *
 * Layout: unified access via .u.error (or NUR_ERROR_OF(r)) across all C standards.
 * On C11/GNU/MSVC, anonymous union also provides direct .error access.
 */
typedef struct NUR_NODISCARD_TYPE NurResult_void {
    bool ok;
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11) || defined(NUR_C23) || (defined(_MSC_VER) && !defined(__GNUC__) && !defined(__clang__))
    union {
        nur_error_t error;
        struct {
            nur_error_t error;
        } u;
    };
#elif defined(NUR_GNU_EXTENSIONS)
    __extension__ union {
        nur_error_t error;
        struct {
            nur_error_t error;
        } u;
    };
#else
    struct {
        nur_error_t error;
    } u;
#endif
} NurResult_void;

#define NUR_VOID_ERROR_OF(r) ((r).u.error)

NUR_NODISCARD NUR_UNUSED
NUR_INLINE bool nur_is_ok_void(NurResult_void r_) {
    return r_.ok;
}

NUR_NODISCARD NUR_UNUSED
NUR_INLINE bool nur_is_err_void(NurResult_void r_) {
    return (bool)!r_.ok;
}

NUR_NODISCARD NUR_UNUSED
NUR_INLINE nur_error_t nur_error_void_val(NurResult_void r_) {
    return r_.u.error;
}

/**
 * Create a successful void result. (Freestanding-safe: no memset needed)
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_ok_void(void) {
#ifndef NUR_COMPACT_ERRORS
    int i_;
#endif
    NurResult_void r;
    r.ok = true;
#ifdef NUR_COMPACT_ERRORS
    r.u.error.code = NUR_E_NONE;
    r.u.error.line = 0U;
    r.u.error.message = (const char *)0;
#else
    r.u.error.code = NUR_E_NONE;
    r.u.error.sys = 0;
    r.u.error.message = (const char *)0;
    r.u.error.context = (const char *)0;
    r.u.error.cause = (const char *)0;
    r.u.error.file = (const char *)0;
    r.u.error.func = (const char *)0;
    r.u.error.line = 0U;
    r.u.error.context_count = 0;
    r.u.error.inline_msg[0] = '\0';
    for (i_ = 0; i_ < NUR_MAX_CONTEXT_DEPTH; ++i_) {
        r.u.error.context_chain[i_] = (const char *)0;
    }
#ifdef NUR_ENABLE_BACKTRACE
    r.u.error.trace_count = 0;
#endif
#endif
    return r;
}

/**
 * Create a failed void result with the given error.
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_err_void(nur_error_t err_) {
    NurResult_void r;
    r.ok = false;
    r.u.error = err_;
    return r;
}

/*============================================================================
 * OPTION TYPE GENERATOR
 *===========================================================================*/

/**
 * Generate an Option<T> type and its constructor functions.
 * 
 * Creates:
 *   - NurOption_<suffix>        The option type
 *   - nur_some_<suffix>(val)    Some constructor
 *   - nur_none_<suffix>()       None constructor
 *
 * Usage at file scope:
 *   NUR_OPTION_TYPE(MyStruct, my_struct);
 */
#define NUR_OPTION_TYPE(T, suffix) \
    typedef struct NurOption_##suffix { \
        bool some; \
        T    value; \
    } NurOption_##suffix; \
    \
    NUR_INLINE NurOption_##suffix nur_some_##suffix(T val_) { \
        NurOption_##suffix o_; \
        o_.some = true; \
        o_.value = val_; \
        return o_; \
    } \
    \
    NUR_INLINE NurOption_##suffix nur_none_##suffix(void) { \
        NurOption_##suffix o_; \
        unsigned char *p_; \
        size_t i_; \
        o_.some = false; \
        p_ = (unsigned char *)(void *)&o_.value; \
        for (i_ = 0; i_ < sizeof(T); ++i_) p_[i_] = 0; \
        return o_; \
    } \
    struct nur_internal_dummy_opt_decl_##suffix

/*============================================================================
 * PREDEFINED OPTION TYPES
 *===========================================================================*/

NUR_OPTION_TYPE(int32_t, i32);
NUR_OPTION_TYPE(int64_t, i64);  
NUR_OPTION_TYPE(size_t, usize);
NUR_OPTION_TYPE(float, f32);
NUR_OPTION_TYPE(double, f64);
NUR_OPTION_TYPE(bool, bool);
NUR_OPTION_TYPE(uint32_t, u32);
NUR_OPTION_TYPE(uint64_t, u64);
NUR_OPTION_TYPE(int8_t, i8);
NUR_OPTION_TYPE(int16_t, i16);
NUR_OPTION_TYPE(uint8_t, u8);
NUR_OPTION_TYPE(uint16_t, u16);
NUR_OPTION_TYPE(char *, cstr);
NUR_OPTION_TYPE(const char *, cstr_const);
NUR_OPTION_TYPE(void *, ptr);

/*============================================================================
 * CONVENIENCE MACROS & UNIVERSAL TYPE ENGINE
 *===========================================================================*/

#ifdef __cplusplus
} /* temporarily exit extern "C" for C++ templates */
/* C++11+ universal constructor engine using native template traits */
template <typename T>
struct nur_cpp_result_traits;

template <> struct nur_cpp_result_traits<bool> { typedef NurResult_bool type; static type make(bool v) { return nur_ok_bool(v); } };
template <> struct nur_cpp_result_traits<signed char> { typedef NurResult_i8 type; static type make(int8_t v) { return nur_ok_i8(v); } };
template <> struct nur_cpp_result_traits<unsigned char> { typedef NurResult_u8 type; static type make(uint8_t v) { return nur_ok_u8(v); } };
template <> struct nur_cpp_result_traits<short> { typedef NurResult_i16 type; static type make(int16_t v) { return nur_ok_i16(v); } };
template <> struct nur_cpp_result_traits<unsigned short> { typedef NurResult_u16 type; static type make(uint16_t v) { return nur_ok_u16(v); } };
template <> struct nur_cpp_result_traits<int> { typedef NurResult_i32 type; static type make(int32_t v) { return nur_ok_i32(v); } };
template <> struct nur_cpp_result_traits<unsigned int> { typedef NurResult_usize type; static type make(size_t v) { return nur_ok_usize(v); } };
template <> struct nur_cpp_result_traits<long> { typedef NurResult_i64 type; static type make(int64_t v) { return nur_ok_i64(v); } };
template <> struct nur_cpp_result_traits<unsigned long> { typedef NurResult_usize type; static type make(size_t v) { return nur_ok_usize(v); } };
template <> struct nur_cpp_result_traits<long long> { typedef NurResult_i64 type; static type make(int64_t v) { return nur_ok_i64(v); } };
#if (defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ < 8) || (defined(UINTPTR_MAX) && UINTPTR_MAX <= 0xFFFFFFFFU) || defined(__i386__) || defined(_M_IX86) || (defined(__arm__) && !defined(__aarch64__))
template <> struct nur_cpp_result_traits<unsigned long long> { typedef NurResult_u64 type; static type make(uint64_t v) { return nur_ok_u64(v); } };
#else
template <> struct nur_cpp_result_traits<unsigned long long> { typedef NurResult_usize type; static type make(size_t v) { return nur_ok_usize(v); } };
#endif
template <> struct nur_cpp_result_traits<float> { typedef NurResult_f32 type; static type make(float v) { return nur_ok_f32(v); } };
template <> struct nur_cpp_result_traits<double> { typedef NurResult_f64 type; static type make(double v) { return nur_ok_f64(v); } };
template <> struct nur_cpp_result_traits<char*> { typedef NurResult_cstr type; static type make(char *v) { return nur_ok_cstr(v); } };
template <> struct nur_cpp_result_traits<const char*> { typedef NurResult_cstr_const type; static type make(const char *v) { return nur_ok_cstr_const(v); } };
template <> struct nur_cpp_result_traits<void*> { typedef NurResult_ptr type; static type make(void *v) { return nur_ok_ptr(v); } };
template <> struct nur_cpp_result_traits<const void*> { typedef NurResult_ptr type; static type make(const void *v) { return nur_ok_ptr((void*)v); } };

template <typename T>
inline typename nur_cpp_result_traits<T>::type nur_cpp_make_result(T val) {
    return nur_cpp_result_traits<T>::make(val);
}

extern "C" {

#define NUR_H(val) nur_cpp_make_result(val)

#define NUR_INTERNAL_GET_MACRO_3(_1, _2, _3, ...) _3
#define NUR_INTERNAL_OK_1(val)         NUR_H(val)
#define NUR_INTERNAL_OK_2(suffix, val) nur_ok_##suffix(val)
#define NUR_OK(...) NUR_INTERNAL_GET_MACRO_3(__VA_ARGS__, NUR_INTERNAL_OK_2, NUR_INTERNAL_OK_1, nur_internal_unused)(__VA_ARGS__)

#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
/* Internal constructor helpers for C11 _Generic dispatch */
NUR_UNUSED NUR_INLINE NurResult_bool nur_internal_gen_bool(bool v) { return nur_ok_bool(v); }
NUR_UNUSED NUR_INLINE NurResult_i8 nur_internal_gen_i8(int8_t v) { return nur_ok_i8(v); }
NUR_UNUSED NUR_INLINE NurResult_u8 nur_internal_gen_u8(uint8_t v) { return nur_ok_u8(v); }
NUR_UNUSED NUR_INLINE NurResult_i16 nur_internal_gen_i16(int16_t v) { return nur_ok_i16(v); }
NUR_UNUSED NUR_INLINE NurResult_u16 nur_internal_gen_u16(uint16_t v) { return nur_ok_u16(v); }
NUR_UNUSED NUR_INLINE NurResult_i32 nur_internal_gen_i32(int32_t v) { return nur_ok_i32(v); }
NUR_UNUSED NUR_INLINE NurResult_i64 nur_internal_gen_long(long v) { return nur_ok_i64((int64_t)v); }
NUR_UNUSED NUR_INLINE NurResult_i64 nur_internal_gen_i64(int64_t v) { return nur_ok_i64(v); }
NUR_UNUSED NUR_INLINE NurResult_usize nur_internal_gen_uint(unsigned int v) { return nur_ok_usize((size_t)v); }
NUR_UNUSED NUR_INLINE NurResult_usize nur_internal_gen_ulong(unsigned long v) { return nur_ok_usize((size_t)v); }
#if (defined(__SIZEOF_SIZE_T__) && __SIZEOF_SIZE_T__ < 8) || (defined(UINTPTR_MAX) && UINTPTR_MAX <= 0xFFFFFFFFU) || defined(__i386__) || defined(_M_IX86) || (defined(__arm__) && !defined(__aarch64__))
NUR_UNUSED NUR_INLINE NurResult_u64 nur_internal_gen_ullong(unsigned long long v) { return nur_ok_u64((uint64_t)v); }
#else
NUR_UNUSED NUR_INLINE NurResult_usize nur_internal_gen_ullong(unsigned long long v) { return nur_ok_usize((size_t)v); }
#endif
NUR_UNUSED NUR_INLINE NurResult_f32 nur_internal_gen_f32(float v) { return nur_ok_f32(v); }
NUR_UNUSED NUR_INLINE NurResult_f64 nur_internal_gen_f64(double v) { return nur_ok_f64(v); }
NUR_UNUSED NUR_INLINE NurResult_cstr nur_internal_gen_cstr(char *v) { return nur_ok_cstr(v); }
NUR_UNUSED NUR_INLINE NurResult_cstr_const nur_internal_gen_const_cstr(const char *v) { return nur_ok_cstr_const(v); }
NUR_UNUSED NUR_INLINE NurResult_ptr nur_internal_gen_ptr(void *v) { return nur_ok_ptr(v); }
NUR_UNUSED NUR_INLINE NurResult_ptr nur_internal_gen_const_ptr(const void *v) { return nur_ok_ptr((void*)v); }
/**
 * Universal single-argument Result constructor using C11 _Generic.
 * Infers input type at compile-time and returns the corresponding NurResult_<T>.
 *
 * Usage:
 *   return NUR_H(42);         // returns NurResult_i32
 *   return NUR_H("success");  // returns NurResult_cstr
 *   return NUR_H(my_ptr);     // returns NurResult_ptr
 */
#define NUR_H(val) _Generic((val), \
    _Bool:              nur_internal_gen_bool, \
    signed char:        nur_internal_gen_i8, \
    unsigned char:      nur_internal_gen_u8, \
    short:              nur_internal_gen_i16, \
    unsigned short:     nur_internal_gen_u16, \
    int:                nur_internal_gen_i32, \
    long:               nur_internal_gen_long, \
    long long:          nur_internal_gen_i64, \
    unsigned int:       nur_internal_gen_uint, \
    unsigned long:      nur_internal_gen_ulong, \
    unsigned long long: nur_internal_gen_ullong, \
    float:              nur_internal_gen_f32, \
    double:             nur_internal_gen_f64, \
    char*:              nur_internal_gen_cstr, \
    const char*:        nur_internal_gen_const_cstr, \
    void*:              nur_internal_gen_ptr, \
    const void*:        nur_internal_gen_const_ptr  \
)(val)

/* Overloaded NUR_OK macro:
 * 1 Argument:  NUR_OK(val)         -> Dispatches dynamically via NUR_H(val)
 * 2 Arguments: NUR_OK(suffix, val) -> Dispatches via legacy nur_ok_##suffix(val)
 */
#define NUR_INTERNAL_GET_MACRO_3(_1, _2, _3, ...) _3
#define NUR_INTERNAL_OK_1(val)         NUR_H(val)
#define NUR_INTERNAL_OK_2(suffix, val) nur_ok_##suffix(val)
#define NUR_OK(...) NUR_INTERNAL_GET_MACRO_3(__VA_ARGS__, NUR_INTERNAL_OK_2, NUR_INTERNAL_OK_1, nur_internal_unused)(__VA_ARGS__)

#else
/* C99 legacy 2-argument constructor */
#define NUR_OK(suffix, val)   nur_ok_##suffix(val)
#define NUR_H(val)            (val)
#endif

#define NUR_ERR(suffix, err)  nur_err_##suffix(err)
#define NUR_OK_VOID()         nur_ok_void()
#define NUR_ERR_VOID(err)     nur_err_void(err)

/* Option constructors */
#define NUR_SOME(suffix, val) nur_some_##suffix(val)
#define NUR_NONE(suffix)      nur_none_##suffix()

/* Status checking */
#define NUR_IS_OK(r)      ((r).ok)
#define NUR_IS_ERR(r)     (!(r).ok)
#define NUR_IS_SOME(o)    ((o).some)
#define NUR_IS_NONE(o)    (!(o).some)

/* Value access - use only after checking status */
#define NUR_VALUE(r)          ((r).u.value)
#define NUR_ERROR_OF(r)       ((r).u.error)
#define NUR_OPTION_VALUE(o)   ((o).value)

/*============================================================================
 * ERROR INTROSPECTION
 *===========================================================================*/

/**
 * Returns a static string name for the error code.
 * Never returns NULL - unknown codes return "NUR_E_UNKNOWN".
 */
NUR_INLINE const char *nur_error_code_name(nur_error_code_t code) {
    switch (code) {
        case NUR_E_NONE: return "NUR_E_NONE";
        
        /* Input/Validation Errors */
        case NUR_E_INVALID_ARGUMENT: return "NUR_E_INVALID_ARGUMENT";
        case NUR_E_OUT_OF_RANGE: return "NUR_E_OUT_OF_RANGE";
        case NUR_E_NULL_POINTER: return "NUR_E_NULL_POINTER";
        case NUR_E_INVALID_STATE: return "NUR_E_INVALID_STATE";
        case NUR_E_OVERFLOW: return "NUR_E_OVERFLOW";
        case NUR_E_UNDERFLOW: return "NUR_E_UNDERFLOW";
        
        /* Data Processing Errors */
        case NUR_E_PARSE: return "NUR_E_PARSE";
        case NUR_E_FORMAT: return "NUR_E_FORMAT";
        case NUR_E_ENCODING: return "NUR_E_ENCODING";
        case NUR_E_CORRUPTED: return "NUR_E_CORRUPTED";
        case NUR_E_TRUNCATED: return "NUR_E_TRUNCATED";
        case NUR_E_CHECKSUM: return "NUR_E_CHECKSUM";
        
        /* Resource Errors */
        case NUR_E_OUT_OF_MEMORY: return "NUR_E_OUT_OF_MEMORY";
        case NUR_E_RESOURCE_LIMIT: return "NUR_E_RESOURCE_LIMIT";
        case NUR_E_BUSY: return "NUR_E_BUSY";
        case NUR_E_QUOTA: return "NUR_E_QUOTA";
        case NUR_E_DEADLOCK: return "NUR_E_DEADLOCK";
        
        /* I/O Errors */
        case NUR_E_NOT_FOUND: return "NUR_E_NOT_FOUND";
        case NUR_E_PERMISSION: return "NUR_E_PERMISSION";
        case NUR_E_ALREADY_EXISTS: return "NUR_E_ALREADY_EXISTS";
        case NUR_E_IO: return "NUR_E_IO";
        case NUR_E_EOF: return "NUR_E_EOF";
        case NUR_E_CLOSED: return "NUR_E_CLOSED";
        case NUR_E_WOULD_BLOCK: return "NUR_E_WOULD_BLOCK";
        
        /* Network/Async Errors */
        case NUR_E_TIMEOUT: return "NUR_E_TIMEOUT";
        case NUR_E_CANCELLED: return "NUR_E_CANCELLED";
        case NUR_E_CONNECTION: return "NUR_E_CONNECTION";
        case NUR_E_PROTOCOL: return "NUR_E_PROTOCOL";
        case NUR_E_AGAIN: return "NUR_E_AGAIN";
        case NUR_E_REFUSED: return "NUR_E_REFUSED";
        
        /* System Errors */
        case NUR_E_UNSUPPORTED: return "NUR_E_UNSUPPORTED";
        case NUR_E_INTERNAL: return "NUR_E_INTERNAL";
        case NUR_E_BUG: return "NUR_E_BUG";
        case NUR_E_VERSION: return "NUR_E_VERSION";
        case NUR_E_UNINITIALIZED: return "NUR_E_UNINITIALIZED";
        case NUR_E_SYSTEM: return "NUR_E_SYSTEM";
        
        default:
            if (code >= NUR_E_USER_BASE) {
                return "NUR_E_USER_DEFINED";
            }
            return "NUR_E_UNKNOWN";
    }
}

/**
 * Maps a Win32/Winsock error number to a semantic nur_error_code_t.
 *
 * Fixes:
 *   - 10035 (WSAEWOULDBLOCK) was NUR_E_BUSY, now NUR_E_WOULD_BLOCK
 *   - 38 (ERROR_HANDLE_EOF) was NUR_E_IO, now NUR_E_EOF
 *   - 10053 (WSAECONNABORTED) was NUR_E_REFUSED, now NUR_E_CONNECTION
 */
NUR_INLINE nur_error_code_t nur_error_code_from_win32(unsigned long error) {
    switch (error) {
        case 0UL:   return NUR_E_NONE;
        case 2UL: case 3UL: case 15UL: case 126UL: case 127UL: case 1168UL:
            return NUR_E_NOT_FOUND;
        case 5UL: case 19UL: case 1314UL:
            return NUR_E_PERMISSION;
        case 6UL: case 87UL: case 123UL: case 161UL: case 267UL:
            return NUR_E_INVALID_ARGUMENT;
        case 8UL: case 14UL:
            return NUR_E_OUT_OF_MEMORY;
        case 4UL: case 39UL: case 112UL: case 122UL:
            return NUR_E_RESOURCE_LIMIT;
        /* Busy/locked - NOT would-block */
        case 21UL: case 32UL: case 33UL: case 170UL: case 231UL:
            return NUR_E_BUSY;
        /* 10035 = WSAEWOULDBLOCK (fix: was NUR_E_BUSY) */
        case 10035UL:
            return NUR_E_WOULD_BLOCK;
        case 23UL:
            return NUR_E_CORRUPTED;
        /* 38 = ERROR_HANDLE_EOF (fix: was NUR_E_IO) */
        case 38UL:
            return NUR_E_EOF;
        case 1117UL:
            return NUR_E_IO;
        case 50UL: case 120UL:
            return NUR_E_UNSUPPORTED;
        case 80UL: case 183UL:
            return NUR_E_ALREADY_EXISTS;
        case 145UL:
            return NUR_E_INVALID_STATE;
        /* Connection reset/dropped */
        case 109UL: case 232UL: case 10053UL: case 10054UL: case 10057UL:
            return NUR_E_CONNECTION;
        case 121UL: case 258UL: case 1460UL: case 10060UL:
            return NUR_E_TIMEOUT;
        case 995UL: case 1223UL: case 10004UL:
            return NUR_E_CANCELLED;
        case 1225UL: case 1236UL: case 10051UL: case 10061UL: case 10065UL:
            return NUR_E_REFUSED;
        default: break;
    }
    return NUR_E_SYSTEM;
}

/**
 * Maps errno to a semantic nur_error_code_t.
 * Fix: NUR_ERROR_ERRNO previously always returned NUR_E_IO;
 * now ENOENT->NOT_FOUND, EACCES->PERMISSION, ENOMEM->OUT_OF_MEMORY, etc.
 */
NUR_INLINE nur_error_code_t nur_error_code_from_errno(int e) {
    switch (e) {
#ifdef ENOENT
        case ENOENT:    return NUR_E_NOT_FOUND;
#endif
#ifdef EACCES
        case EACCES:
#endif
#if defined(EPERM) && (!defined(EACCES) || EPERM != EACCES)
        case EPERM:
#endif
#if defined(EACCES) || defined(EPERM)
            return NUR_E_PERMISSION;
#endif
#ifdef ENOMEM
        case ENOMEM:    return NUR_E_OUT_OF_MEMORY;
#endif
#ifdef EEXIST
        case EEXIST:    return NUR_E_ALREADY_EXISTS;
#endif
#ifdef EAGAIN
        case EAGAIN:    return NUR_E_AGAIN;
#endif
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || EWOULDBLOCK != EAGAIN)
        case EWOULDBLOCK: return NUR_E_WOULD_BLOCK;
#endif
#ifdef ETIMEDOUT
        case ETIMEDOUT: return NUR_E_TIMEOUT;
#endif
#ifdef ENOBUFS
        case ENOBUFS:
#endif
#if defined(ENFILE) && (!defined(ENOBUFS) || ENFILE != ENOBUFS)
        case ENFILE:
#endif
#if defined(EMFILE) && (!defined(ENOBUFS) || EMFILE != ENOBUFS) && (!defined(ENFILE) || EMFILE != ENFILE)
        case EMFILE:
#endif
#if defined(ENOBUFS) || defined(ENFILE) || defined(EMFILE)
        return NUR_E_RESOURCE_LIMIT;
#endif
#ifdef EPIPE
        case EPIPE:     return NUR_E_CLOSED;
#endif
#ifdef EBADF
        case EBADF:
#endif
#if defined(EINVAL) && (!defined(EBADF) || EINVAL != EBADF)
        case EINVAL:
#endif
#if defined(EBADF) || defined(EINVAL)
        return NUR_E_INVALID_ARGUMENT;
#endif
#ifdef ERANGE
        case ERANGE:    return NUR_E_OUT_OF_RANGE;
#endif
#ifdef EOVERFLOW
        case EOVERFLOW: return NUR_E_OVERFLOW;
#endif
#ifdef ENOSPC
        case ENOSPC:    return NUR_E_RESOURCE_LIMIT;
#endif
#ifdef ECONNREFUSED
        case ECONNREFUSED: return NUR_E_REFUSED;
#endif
#ifdef ECONNRESET
        case ECONNRESET:
#endif
#if defined(ENOTCONN) && (!defined(ECONNRESET) || ENOTCONN != ECONNRESET)
        case ENOTCONN:
#endif
#if defined(ECONNRESET) || defined(ENOTCONN)
        return NUR_E_CONNECTION;
#endif
#ifdef ENOSYS
        case ENOSYS:    return NUR_E_UNSUPPORTED;
#endif
#ifdef EDEADLK
        case EDEADLK:   return NUR_E_DEADLOCK;
#endif
#ifdef ECANCELED
        case ECANCELED: return NUR_E_CANCELLED;
#endif
        default:        return NUR_E_IO;
    }
}

/*============================================================================
 * STRING SINK FORMATTING (Allocation-free)
 *===========================================================================*/

typedef struct {
    char  *buffer;
    size_t capacity;
    size_t length;
} nur_internal_sink_t;

NUR_INLINE void nur_internal_sink_putc(nur_internal_sink_t *sink, char c) {
    if (sink->buffer != NULL && sink->capacity != 0 && sink->length < sink->capacity - 1) {
        sink->buffer[sink->length] = c;
    }
    ++sink->length;
}

NUR_INLINE void nur_internal_sink_puts(nur_internal_sink_t *sink, const char *text) {
    if (!text) return;
    while (*text != '\0') {
        nur_internal_sink_putc(sink, *text++);
    }
}

#ifdef NUR_C89
    typedef unsigned long nur_internal_uintmax_t;
    typedef long nur_internal_intmax_t;
#else
    typedef unsigned long long nur_internal_uintmax_t;
    typedef long long nur_internal_intmax_t;
#endif

NUR_INLINE void nur_internal_sink_putu(nur_internal_sink_t *sink, nur_internal_uintmax_t value) {
    char digits[24];
    size_t count = 0;
    do {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    } while (value != 0);
    while (count > 0) {
        nur_internal_sink_putc(sink, digits[--count]);
    }
}

NUR_INLINE void nur_internal_sink_puti(nur_internal_sink_t *sink, nur_internal_intmax_t value) {
    if (value < 0) {
        nur_internal_sink_putc(sink, '-');
        /* Fix -Wsign-conversion: cast through unsigned integer explicitly */
        nur_internal_sink_putu(sink, 0 - (nur_internal_uintmax_t)value);
    } else {
        nur_internal_sink_putu(sink, (nur_internal_uintmax_t)value);
    }
}

/**
 * Renders error into buffer as a single line.
 * Returns required length excluding NUL. If >= capacity, output was truncated.
 *
 * Fix: if error is NULL, writes NUL to buffer[0] so buf is always valid.
 */
NUR_INLINE size_t nur_error_format(const nur_error_t *error, char *buffer, size_t capacity) {
    nur_internal_sink_t sink;
#ifndef NUR_COMPACT_ERRORS
    unsigned char i_;
#endif
    if (!error) {
        if (buffer && capacity > 0) buffer[0] = '\0';
        return 0;
    }
    sink.buffer = buffer;
    sink.capacity = capacity;
    sink.length = 0;
    
    nur_internal_sink_puts(&sink, nur_error_code_name(error->code));
    nur_internal_sink_puts(&sink, ": ");
    nur_internal_sink_puts(&sink, error->message ? error->message : "unknown");
    
#ifndef NUR_COMPACT_ERRORS
    if (error->context && error->context[0] != '\0') {
        nur_internal_sink_puts(&sink, " [");
        nur_internal_sink_puts(&sink, error->context);
        nur_internal_sink_putc(&sink, ']');
    }
    
    if (error->cause && error->cause[0] != '\0') {
        nur_internal_sink_puts(&sink, " (caused by: ");
        nur_internal_sink_puts(&sink, error->cause);
        nur_internal_sink_putc(&sink, ')');
    }

    if (error->context_count > 2) {
        nur_internal_sink_puts(&sink, " (chain: ");
        for (i_ = error->context_count; i_ > 0; --i_) {
            if (error->context_chain[i_ - 1] != (const char *)0) {
                if (i_ < error->context_count) nur_internal_sink_puts(&sink, " -> ");
                nur_internal_sink_puts(&sink, error->context_chain[i_ - 1]);
            }
        }
        nur_internal_sink_putc(&sink, ')');
    }
    
    if (error->sys != 0) {
        nur_internal_sink_puts(&sink, " (sys: ");
        nur_internal_sink_puti(&sink, error->sys);
        nur_internal_sink_putc(&sink, ')');
    }
    
    if (error->file) {
        nur_internal_sink_puts(&sink, " at ");
        nur_internal_sink_puts(&sink, error->file);
        nur_internal_sink_putc(&sink, ':');
        nur_internal_sink_putu(&sink, error->line);
        if (error->func) {
            nur_internal_sink_puts(&sink, " in ");
            nur_internal_sink_puts(&sink, error->func);
            nur_internal_sink_puts(&sink, "()");
        }
    }
#else
    nur_internal_sink_puts(&sink, " at line ");
    nur_internal_sink_putu(&sink, error->line);
#endif
    
    if (buffer && capacity > 0) {
        buffer[sink.length < capacity ? sink.length : capacity - 1] = '\0';
    }
    return sink.length;
}

/**
 * Renders error as a valid RFC 8259 JSON string into buffer.
 * Returns required length excluding NUL. If >= capacity, output was truncated.
 * Thread-safe, allocation-free.
 *
 * Example output:
 * {"ok":false,"code":30,"name":"NUR_E_NOT_FOUND","message":"file missing","context":"cfg.json","sys":2,"file":"main.c","func":"run","line":42}
 */
NUR_INLINE size_t nur_error_json(const nur_error_t *error, char *buffer, size_t capacity) {
    nur_internal_sink_t sink;
#ifndef NUR_COMPACT_ERRORS
    unsigned char i_;
#endif
    const char *p;
    if (!error) {
        if (buffer && capacity > 0) {
            static const char null_json[] = "{\"ok\":true}";
            size_t len = sizeof(null_json) - 1;
            size_t copy_len = (capacity > len) ? len : (capacity - 1);
            if (copy_len > 0) {
                size_t k;
                /* cppcheck-suppress unsignedLessThanZero */
                for (k = 0; k < copy_len; ++k) {
                    buffer[k] = null_json[k];
                }
            }
            buffer[copy_len] = '\0';
            return len;
        }
        return 11;
    }
    sink.buffer = buffer;
    sink.capacity = capacity;
    sink.length = 0;
    nur_internal_sink_puts(&sink, "{\"ok\":false,\"code\":");
    nur_internal_sink_putu(&sink, (nur_internal_uintmax_t)error->code);
    nur_internal_sink_puts(&sink, ",\"name\":\"");
    nur_internal_sink_puts(&sink, nur_error_code_name(error->code));
    nur_internal_sink_puts(&sink, "\",\"message\":");
    if (error->message) {
        nur_internal_sink_putc(&sink, '"');
        p = error->message;
        while (*p) {
            if (*p == '"') nur_internal_sink_puts(&sink, "\\\"");
            else if (*p == '\\') nur_internal_sink_puts(&sink, "\\\\");
            else if (*p == '\n') nur_internal_sink_puts(&sink, "\\n");
            else if (*p == '\r') nur_internal_sink_puts(&sink, "\\r");
            else if (*p == '\t') nur_internal_sink_puts(&sink, "\\t");
            else nur_internal_sink_putc(&sink, *p);
            p++;
        }
        nur_internal_sink_putc(&sink, '"');
    } else {
        nur_internal_sink_puts(&sink, "null");
    }

#ifndef NUR_COMPACT_ERRORS
    if (error->context && error->context[0] != '\0') {
        nur_internal_sink_puts(&sink, ",\"context\":\"");
        p = error->context;
        while (*p) {
            if (*p == '"') nur_internal_sink_puts(&sink, "\\\"");
            else if (*p == '\\') nur_internal_sink_puts(&sink, "\\\\");
            else if (*p == '\n') nur_internal_sink_puts(&sink, "\\n");
            else if (*p == '\r') nur_internal_sink_puts(&sink, "\\r");
            else if (*p == '\t') nur_internal_sink_puts(&sink, "\\t");
            else nur_internal_sink_putc(&sink, *p);
            p++;
        }
        nur_internal_sink_putc(&sink, '"');
    }

    if (error->cause && error->cause[0] != '\0') {
        nur_internal_sink_puts(&sink, ",\"cause\":\"");
        p = error->cause;
        while (*p) {
            if (*p == '"') nur_internal_sink_puts(&sink, "\\\"");
            else if (*p == '\\') nur_internal_sink_puts(&sink, "\\\\");
            else if (*p == '\n') nur_internal_sink_puts(&sink, "\\n");
            else if (*p == '\r') nur_internal_sink_puts(&sink, "\\r");
            else if (*p == '\t') nur_internal_sink_puts(&sink, "\\t");
            else nur_internal_sink_putc(&sink, *p);
            p++;
        }
        nur_internal_sink_putc(&sink, '"');
    }

    if (error->context_count > 1) {
        nur_internal_sink_puts(&sink, ",\"chain\":[");
        for (i_ = 0; i_ < error->context_count; ++i_) {
            if (i_ > 0) nur_internal_sink_putc(&sink, ',');
            nur_internal_sink_putc(&sink, '"');
            p = error->context_chain[i_];
            while (p && *p) {
                if (*p == '"') nur_internal_sink_puts(&sink, "\\\"");
                else if (*p == '\\') nur_internal_sink_puts(&sink, "\\\\");
                else if (*p == '\n') nur_internal_sink_puts(&sink, "\\n");
                else if (*p == '\r') nur_internal_sink_puts(&sink, "\\r");
                else if (*p == '\t') nur_internal_sink_puts(&sink, "\\t");
                else nur_internal_sink_putc(&sink, *p);
                p++;
            }
            nur_internal_sink_putc(&sink, '"');
        }
        nur_internal_sink_putc(&sink, ']');
    }

    if (error->sys != 0) {
        nur_internal_sink_puts(&sink, ",\"sys\":");
        nur_internal_sink_puti(&sink, error->sys);
    }

    if (error->file) {
        nur_internal_sink_puts(&sink, ",\"file\":\"");
        p = error->file;
        while (*p) {
            if (*p == '"') nur_internal_sink_puts(&sink, "\\\"");
            else if (*p == '\\') nur_internal_sink_puts(&sink, "\\\\");
            else nur_internal_sink_putc(&sink, *p);
            p++;
        }
        nur_internal_sink_puts(&sink, "\",\"line\":");
        nur_internal_sink_putu(&sink, error->line);
        if (error->func) {
            nur_internal_sink_puts(&sink, ",\"func\":\"");
            nur_internal_sink_puts(&sink, error->func);
            nur_internal_sink_putc(&sink, '"');
        }
    }
#else
    nur_internal_sink_puts(&sink, ",\"line\":");
    nur_internal_sink_putu(&sink, error->line);
#endif
    nur_internal_sink_putc(&sink, '}');

    if (buffer && capacity > 0) {
        buffer[sink.length < capacity ? sink.length : capacity - 1] = '\0';
    }
    return sink.length;
}

/*============================================================================
 * PANIC INFRASTRUCTURE & RECOVERY HOOKS
 *===========================================================================*/

#ifndef NUR_PRINT_BUFFER_SIZE
#define NUR_PRINT_BUFFER_SIZE 1024
#endif

/*
 * Low-level hardware/runtime abort hook.
 * Redefine NUR_PANIC_ABORT() for bare-metal, kernel, or custom fault handlers.
 */
#ifndef NUR_PANIC_ABORT
    #ifndef NUR_NO_STDLIB
        #define NUR_PANIC_ABORT() abort()
    #elif defined(__GNUC__) || defined(__clang__)
        #define NUR_PANIC_ABORT() __builtin_trap()
    #else
        #define NUR_PANIC_ABORT() do { volatile int *p_ = 0; (void)*p_; for(;;) {} } while(0)
    #endif
#endif

/*
 * Configurable Panic Handler
 * By default, abort() is called after printing to stderr (if stdio is available).
 * Users can register a custom handler to log, unwind, reset hardware, or recover.
 */
typedef void (*nur_panic_handler_t)(const char *msg, const nur_error_t *e,
                                    const char *file, unsigned int line);

#ifndef NUR_CUSTOM_PANIC_HANDLER_STORAGE
    #ifdef _MSC_VER
        #define NUR_CUSTOM_PANIC_HANDLER_STORAGE __declspec(selectany)
    #elif defined(__GNUC__) || defined(__clang__)
        #define NUR_CUSTOM_PANIC_HANDLER_STORAGE __attribute__((weak))
    #else
        #define NUR_CUSTOM_PANIC_HANDLER_STORAGE static
    #endif
#endif

NUR_UNUSED NUR_CUSTOM_PANIC_HANDLER_STORAGE nur_panic_handler_t nur_internal_custom_panic_handler = NULL;

/**
 * Set custom panic handler. Synchronized across all translation units.
 * Set NULL to restore default stderr+abort behaviour.
 */
NUR_INLINE void nur_set_panic_handler(nur_panic_handler_t handler) {
    nur_internal_custom_panic_handler = handler;
}

/*
 * Panic helpers: noinline + cold + noreturn
 *   noinline  : 1 KiB stack buffer lives ONCE in .text, not at every
 *               NUR_UNWRAP call site (~11x code-size reduction).
 *   cold      : optimizer marks call path as rarely taken.
 *   noreturn  : analyzer knows control never continues past this call.
 *   NUR_UNUSED: suppresses -Wunused-function when translation unit avoids UNWRAP.
 */
NUR_NOINLINE NUR_COLD NUR_NORETURN NUR_UNUSED
static void nur_internal_panic_result(nur_error_t e,
                                      const char *file, unsigned int line) {
#ifndef NUR_NO_STDIO
    char buffer[NUR_PRINT_BUFFER_SIZE];
#endif
    if (nur_internal_custom_panic_handler != NULL) {
        nur_internal_custom_panic_handler(NULL, &e, file, line);
    }
#ifndef NUR_NO_STDIO
    nur_error_format(&e, buffer, sizeof(buffer));
    (void)fprintf(stderr, "nur panic at %s:%u: %s\n", file ? file : "<unknown>", line, buffer);
    (void)fflush(stderr);
#endif
    NUR_PANIC_ABORT();
}

NUR_NOINLINE NUR_COLD NUR_NORETURN NUR_UNUSED
static void nur_internal_panic_msg(const char *msg, nur_error_t e,
                                   const char *file, unsigned int line) {
#ifndef NUR_NO_STDIO
    char buffer[NUR_PRINT_BUFFER_SIZE];
#endif
    if (nur_internal_custom_panic_handler != NULL) {
        nur_internal_custom_panic_handler(msg, &e, file, line);
    }
#ifndef NUR_NO_STDIO
    nur_error_format(&e, buffer, sizeof(buffer));
    (void)fprintf(stderr, "nur panic at %s:%u: %s: %s\n", file ? file : "<unknown>", line, msg ? msg : "", buffer);
    (void)fflush(stderr);
#endif
    NUR_PANIC_ABORT();
}

#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)

#define NUR_UNWRAP(r) \
    __extension__ ({ \
        NUR_AUTO _nur_unwrap_r_ = (r); \
        if (NUR_UNLIKELY(!_nur_unwrap_r_.ok)) { \
            nur_internal_panic_result(_nur_unwrap_r_.u.error, NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        _nur_unwrap_r_.u.value; \
    })

#define NUR_EXPECT(r, msg) \
    __extension__ ({ \
        NUR_AUTO _nur_expect_r_ = (r); \
        if (NUR_UNLIKELY(!_nur_expect_r_.ok)) { \
            nur_internal_panic_msg((msg), _nur_expect_r_.u.error, NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        _nur_expect_r_.u.value; \
    })

#else

/* Universal single/portable expression fallback supporting all standard and custom Result types */
#define NUR_UNWRAP(r) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_result((r).u.error, NUR_LOC_FILE, NUR_LOC_LINE), (r).u.value))

#define NUR_EXPECT(r, msg) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_msg((msg), (r).u.error, NUR_LOC_FILE, NUR_LOC_LINE), (r).u.value))
#endif

/**
 * Assert that a NurResult_void is successful, panicking on error.
 * Evaluates 'r' exactly once across all C standards.
 */
NUR_INLINE void nur_unwrap_void_internal(NurResult_void r_, const char *file_, unsigned int line_) {
    if (NUR_UNLIKELY(!r_.ok)) {
        nur_internal_panic_result(NUR_ERROR_OF(r_), file_, line_);
    }
}

/**
 * Assert that a NurResult_void is successful with custom message, panicking on error.
 * Evaluates 'r' exactly once across all C standards.
 */
NUR_INLINE void nur_expect_void_internal(NurResult_void r_, const char *msg_, const char *file_, unsigned int line_) {
    if (NUR_UNLIKELY(!r_.ok)) {
        nur_internal_panic_msg(msg_, NUR_ERROR_OF(r_), file_, line_);
    }
}

#define NUR_UNWRAP_VOID(r) nur_unwrap_void_internal((r), NUR_LOC_FILE, NUR_LOC_LINE)
#define NUR_EXPECT_VOID(r, msg) nur_expect_void_internal((r), (msg), NUR_LOC_FILE, NUR_LOC_LINE)

/*============================================================================
 * SAFE VALUE EXTRACTION
 *===========================================================================*/

#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)

/**
 * Get value or fallback if error (evaluates r exactly once).
 * 
 * Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);
 */
#define NUR_UNWRAP_OR(r, fallback) \
    __extension__ ({ \
        NUR_AUTO _nur_unwrap_or_r_ = (r); \
        NUR_LIKELY(_nur_unwrap_or_r_.ok) ? _nur_unwrap_or_r_.u.value : (fallback); \
    })

/**
 * Get option value or fallback if None (evaluates o exactly once).
 * 
 * Usage: int val = NUR_OPTION_UNWRAP_OR(opt, 0);
 */
#define NUR_OPTION_UNWRAP_OR(o, fallback) \
    __extension__ ({ \
        NUR_AUTO _nur_opt_unwrap_or_o_ = (o); \
        NUR_LIKELY(_nur_opt_unwrap_or_o_.some) ? _nur_opt_unwrap_or_o_.value : (fallback); \
    })

/**
 * Explicitly ignore a result - documents intent and suppresses compiler warnings.
 * 
 * Usage: NUR_IGNORE(write_log(msg));
 */
#define NUR_IGNORE(expr) \
    do { \
        NUR_AUTO _nur_ignored_ = (expr); \
        (void)_nur_ignored_; \
    } while (0)

#else

/**
 * Get value or fallback if error.
 *
 * Single-evaluation on C11+ and MSVC (when _Generic is available) via typed
 * inline functions (nur_unwrap_or_<suffix>). Evaluates 'r' exactly once.
 * Fallback to ternary on strict C99 where pre-assigning to a variable is recommended.
 *
 * Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);
 */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || (defined(_MSC_VER) && _MSC_VER >= 1928)
#define NUR_UNWRAP_OR(r, fallback) \
    _Generic((r), \
        NurResult_i32:        nur_unwrap_or_i32, \
        NurResult_i64:        nur_unwrap_or_i64, \
        NurResult_usize:      nur_unwrap_or_usize, \
        NurResult_f32:        nur_unwrap_or_f32, \
        NurResult_f64:        nur_unwrap_or_f64, \
        NurResult_cstr:       nur_unwrap_or_cstr, \
        NurResult_cstr_const: nur_unwrap_or_cstr_const, \
        NurResult_ptr:        nur_unwrap_or_ptr, \
        NurResult_bool:       nur_unwrap_or_bool, \
        NurResult_u32:        nur_unwrap_or_u32, \
        NurResult_u64:        nur_unwrap_or_u64, \
        NurResult_i8:         nur_unwrap_or_i8, \
        NurResult_i16:        nur_unwrap_or_i16, \
        NurResult_u8:         nur_unwrap_or_u8, \
        NurResult_u16:        nur_unwrap_or_u16 \
    )((r), (fallback))
#else
#define NUR_UNWRAP_OR(r, fallback) \
    (NUR_LIKELY((r).ok) ? (r).u.value : (fallback))
#endif

/**
 * Get option value or fallback if None.
 *
 * Usage: int val = NUR_OPTION_UNWRAP_OR(opt, 0);
 */
#define NUR_OPTION_UNWRAP_OR(o, fallback) \
    (NUR_LIKELY((o).some) ? (o).value : (fallback))

/**
 * Explicitly ignore a result - documents intent and suppresses warnings.
 *
 * Usage: NUR_IGNORE(write_log(msg));
 */
#define NUR_IGNORE(expr) ((void)(expr))

#endif

/**
 * Convert Option<T> to Result<T, nur_error_t>.
 * If opt has value, returns Ok(value); otherwise returns Err(err).
 *
 * Usage:
 *   NurResult_i32 res = NUR_OK_OR(i32, opt, NUR_ERROR(NUR_E_NOT_FOUND, "no value"));
 */
#define NUR_OK_OR(suffix, opt, err) \
    ((opt).some ? nur_ok_##suffix((opt).value) : nur_err_##suffix(err))

/**
 * Unwrap an Option<T>, panicking if None.
 */
#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)
#define NUR_UNWRAP_OPTION(o) \
    __extension__ ({ \
        NUR_AUTO _nur_uwo_ = (o); \
        if (NUR_UNLIKELY(!_nur_uwo_.some)) { \
            nur_internal_panic_msg("called NUR_UNWRAP_OPTION on a None value", \
                NUR_ERROR(NUR_E_INVALID_STATE, "Option is None"), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        _nur_uwo_.value; \
    })
#else
#define NUR_UNWRAP_OPTION(o) \
    (NUR_LIKELY((o).some) \
        ? (o).value \
        : (nur_internal_panic_msg("called NUR_UNWRAP_OPTION on a None value", \
            NUR_ERROR(NUR_E_INVALID_STATE, "Option is None"), NUR_LOC_FILE, NUR_LOC_LINE), (o).value))
#endif

/**
 * Propagate None from Option in functions returning an Option.
 * If expr is None, returns None early from enclosing function.
 */
#ifdef NUR_GNU_EXTENSIONS
#define NUR_OPTION_TRY(lhs, expr) \
    do { \
        __auto_type _nur_opt_tmp_ = (expr); \
        if (NUR_UNLIKELY(!_nur_opt_tmp_.some)) { \
            __typeof__(_nur_opt_tmp_) _nur_none_ret_ = {0}; \
            return _nur_none_ret_; \
        } \
        (lhs) = _nur_opt_tmp_.value; \
    } while (0)
#elif defined(NUR_C23)
#define NUR_OPTION_TRY(lhs, expr) \
    do { \
        auto _nur_opt_tmp_ = (expr); \
        if (NUR_UNLIKELY(!_nur_opt_tmp_.some)) { \
            typeof(_nur_opt_tmp_) _nur_none_ret_ = {0}; \
            return _nur_none_ret_; \
        } \
        (lhs) = _nur_opt_tmp_.value; \
    } while (0)
#endif

#define NUR_OPTION_TRY_TO(suffix, lhs, expr) \
    do { \
        NurOption_##suffix _nur_opt_tmp_ = (expr); \
        if (NUR_UNLIKELY(!_nur_opt_tmp_.some)) { \
            return nur_none_##suffix(); \
        } \
        (lhs) = _nur_opt_tmp_.value; \
    } while (0)

/* Semantic alias for NUR_UNWRAP_OR */
#define NUR_OR_ELSE(r, fallback) NUR_UNWRAP_OR((r), (fallback))

/* Portable value-or alias (same semantics, more explicit name) */
#define NUR_VALUE_OR(r, fallback) NUR_UNWRAP_OR((r), (fallback))

/*============================================================================
 * ERROR PREDICATES
 *===========================================================================*/

/**
 * Test whether a nur_error_t has a specific error code.
 *
 * Usage:
 *   nur_error_t e = NUR_ERROR_OF(r);
 *   if (NUR_ERROR_IS(e, NUR_E_NOT_FOUND)) { ... }
 */
#define NUR_ERROR_IS(err, errcode_) ((err).code == (errcode_))

/**
 * Convenience alias: wrap an existing error with a new context string,
 * creating a new nur_error_t that preserves original code, sys, message,
 * file, func, line but replaces .context.
 *
 * This is an ergonomic alias for NUR_WRAP_ERROR (which preserves original
 * source location). Use for adding caller context without losing the origin.
 *
 * Usage:
 *   nur_error_t wrapped = NUR_ERROR_WRAP(NUR_ERROR_OF(r), "while parsing config");
 *   return NUR_ERR(i32, wrapped);
 */
#define NUR_ERROR_WRAP(err_val, new_ctx) NUR_WRAP_ERROR((err_val), (new_ctx))

/*============================================================================
 * RESULT COMBINATORS  (NUR_MAP, NUR_AND_THEN)
 *===========================================================================*/

#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)

/**
 * Map a successful Result value using func, preserving error.
 *
 * Evaluates result once. If Ok, applies func to the payload and returns
 * a new Ok with the result. If Err, propagates the existing error.
 *
 * NOTE: NUR_MAP is restricted to same-type transformations (T -> T).
 * For mapping between different types (T -> U), use NUR_MAP_TO(target_suffix, result, func).
 *
 * func signature: <value_type> func(<value_type>)
 *
 * Usage:
 *   static int double_it(int x) { return x * 2; }
 *   NurResult_i32 r2 = NUR_MAP(r, double_it);
 */
#define NUR_MAP(result, func) \
    __extension__ ({ \
        NUR_AUTO _nur_map_r_ = (result); \
        if (_nur_map_r_.ok) { \
            _nur_map_r_.u.value = (func)(_nur_map_r_.u.value); \
        } \
        _nur_map_r_; \
    })

/**
 * Typed map converting Ok value into target type 'suffix'.
 * On error, constructs a result of target type 'suffix' with the original error.
 */
#define NUR_MAP_TO(suffix, result, func) \
    __extension__ ({ \
        NUR_AUTO _nur_mapto_r_ = (result); \
        _nur_mapto_r_.ok ? nur_ok_##suffix((func)(_nur_mapto_r_.u.value)) : nur_err_##suffix(_nur_mapto_r_.u.error); \
    })

/**
 * Chain a result-returning function on a successful result.
 *
 * NUR_AND_THEN(result, func) evaluates result once. If Ok, calls func with
 * the payload and returns its NurResult. If Err, passes the error through.
 *
 * func must have signature: NurResult_<same_suffix> func(<value_type>).
 *
 * GNU extension: uses statement expressions for single-evaluation.
 *
 * Usage:
 *   NurResult_i32 parse_uid(int raw);  // some transform
 *   NurResult_i32 r2 = NUR_AND_THEN(r, parse_uid);
 */
#define NUR_AND_THEN(result, func) \
    __extension__ ({ \
        NUR_AUTO _nur_at_r_ = (result); \
        _nur_at_r_.ok ? (func)(_nur_at_r_.u.value) : _nur_at_r_; \
    })

#else /* Non-GNU */

/**
 * NUR_MAP_TO - Portable typed map for non-GNU and ANSI C compilers.
 * Maps Ok value using 'func' into a new Ok result of target type 'suffix'.
 * On error, constructs a result of target type 'suffix' with the original error.
 */
#define NUR_MAP_TO(suffix, result, func) \
    ((result).ok ? nur_ok_##suffix((func)((result).u.value)) : nur_err_##suffix((result).u.error))

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
/* Restricted to same-type mappings where func returns the same type as result */
#define NUR_MAP(result, func) \
    ((result).ok ? NUR_H((func)((result).u.value)) : (result))
#endif

/**
 * NUR_AND_THEN - Non-GNU version.
 *
 * Same double-evaluation caveat as NUR_UNWRAP_OR. Pre-assign result:
 *   NurResult_i32 _r = compute();
 *   NurResult_i32 r2 = NUR_AND_THEN(_r, parse_uid);
 */
#define NUR_AND_THEN(result, func) \
    (NUR_IS_OK(result) ? (func)(NUR_VALUE(result)) : (result))

#endif /* NUR_GNU_EXTENSIONS */

/*============================================================================
 * C-PURIST & EXPLICIT CONTROL FLOW APIS
 *===========================================================================*/

/**
 * When NUR_NO_HIDDEN_RETURNS or NUR_PURIST is defined, all macros containing
 * hidden early-return statements (NUR_TRY*, NUR_RETURN_IF_ERR, NUR_CHECK*)
 * are disabled.
 *
 * This provides compile-time enforcement for codebases requiring 100% explicit
 * control flow, zero-hidden-return safety, and zero leak guarantees.
 */
#if defined(NUR_NO_HIDDEN_RETURNS) || defined(NUR_PURIST)
    #define NUR_PURIST_MODE 1
#endif

/**
 * Universal pointer predicates for pure function checks (100% macro-free).
 * Works across all NurResult_<T> types since .ok is at byte offset 0.
 */
NUR_INLINE bool nur_result_is_ok(const void *result_ptr) {
    return (bool)(result_ptr != NULL && *(const bool *)result_ptr);
}

NUR_INLINE bool nur_result_is_err(const void *result_ptr) {
    return (bool)(result_ptr == NULL || !(*(const bool *)result_ptr));
}

NUR_INLINE nur_error_code_t nur_error_code_of(const nur_error_t *err) {
    return err ? err->code : NUR_E_NONE;
}

NUR_INLINE const char *nur_error_message_of(const nur_error_t *err) {
    if (!err) return "";
#ifndef NUR_COMPACT_ERRORS
    if (err->inline_msg[0] != '\0') return err->inline_msg;
#endif
    return err->message ? err->message : "";
}

NUR_INLINE const char *nur_error_message(const nur_error_t *err) {
    return nur_error_message_of(err);
}

#define NUR_ERROR_MSG(err) nur_error_message_of(&(err))

/*============================================================================
 * EARLY-RETURN HELPERS
 *===========================================================================*/

#ifndef NUR_PURIST_MODE

/**
 * Return an error result from the enclosing function if expr is Err.
 * The enclosing function's return type must be NurResult_void.
 *
 * This is a portable (no GNU extensions) early-return helper.
 * It evaluates expr exactly once via a named temporary.
 *
 * Usage:
 *   NUR_RETURN_IF_ERR(validate_input(buf));
 */
#define NUR_RETURN_IF_ERR(expr) \
    do { \
        NurResult_void _nur_rife_tmp_ = (NurResult_void)(expr); \
        if (NUR_UNLIKELY(!_nur_rife_tmp_.ok)) { \
            return _nur_rife_tmp_; \
        } \
    } while (0)

/**
 * Like NUR_CHECK but with an explicit message argument for more context.
 * Returns NurResult_void error if condition is false.
 *
 * Usage:
 *   NUR_CHECK_MSG(port > 0 && port < 65536, NUR_E_OUT_OF_RANGE, "port must be 1-65535");
 */
#define NUR_CHECK_MSG(cond, code, msg) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_void(NUR_ERROR((code), (msg))); \
        } \
    } while (0)

/**
 * Like NUR_CHECK_AS but with an explicit message (typed return).
 * Returns typed NurResult error if condition is false.
 *
 * Usage:
 *   NUR_CHECK_AS_MSG(i32, size > 0, NUR_E_INVALID_ARGUMENT, "size must be positive");
 */
#define NUR_CHECK_AS_MSG(DestSuffix, cond, code, msg) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_##DestSuffix(NUR_ERROR((code), (msg))); \
        } \
    } while (0)

#endif /* !NUR_PURIST_MODE */


/**
 * Branch on error and bind error struct into err_var.
 *
 * Usage:
 *   NUR_IF_ERR(res, err, {
 *       nur_error_eprint(&err);
 *   });
 */
#define NUR_IF_ERR(r, err_var, body) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            nur_error_t err_var = NUR_ERROR_OF(r); \
            body; \
        } \
    } while (0)

/**
 * Branch on error for void result and bind error struct into err_var.
 */
#define NUR_VOID_IF_ERR(r, err_var, body) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            nur_error_t err_var = NUR_VOID_ERROR_OF(r); \
            body; \
        } \
    } while (0)

/**
 * Pattern match style construct for typed results with explicit type (C99 compatible).
 *
 * Usage:
 *   NUR_MATCH_AS(int, res, val, { printf("Val: %d\n", val); }, err, { nur_error_eprint(&err); });
 */
#define NUR_MATCH_AS(Type, r, val_var, ok_body, err_var, err_body) \
    do { \
        if (NUR_IS_OK(r)) { \
            Type val_var = NUR_VALUE(r); \
            ok_body; \
        } else { \
            nur_error_t err_var = NUR_ERROR_OF(r); \
            err_body; \
        } \
    } while (0)

#if defined(NUR_GNU_EXTENSIONS) || defined(NUR_C23) || defined(__cplusplus)
/**
 * Type-inferring pattern match construct (GNU/C23/C++).
 *
 * Usage:
 *   NUR_MATCH(res, val, { printf("Val: %d\n", val); }, err, { nur_error_eprint(&err); });
 */
#define NUR_MATCH(r, val_var, ok_body, err_var, err_body) \
    do { \
        NUR_AUTO _nur_m_ = (r); \
        if (NUR_IS_OK(_nur_m_)) { \
            NUR_AUTO val_var = _nur_m_.u.value; \
            ok_body; \
        } else { \
            nur_error_t err_var = _nur_m_.u.error; \
            err_body; \
        } \
    } while (0)
#endif

/*============================================================================
 * ERROR PROPAGATION
 *===========================================================================*/

/**
 * Evaluate expr -> NurResult_SrcSuffix.
 * On error: return nur_err_DestSuffix(error) from enclosing function.
 * On success: assign .u.value to lhs.
 * 
 * Usage:
 *   int value;
 *   NUR_TRY_TO(i32, i32, value, parse_number(str));
 *   // value now contains the parsed number, or function returned error
 */
#ifndef NUR_PURIST_MODE

#define NUR_TRY_TO(DestSuffix, SrcSuffix, lhs, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_##DestSuffix(NUR_ERROR_OF(_nur_tmp_)); \
        } \
        (lhs) = NUR_VALUE(_nur_tmp_); \
    } while (0)

/* NUR_TRY_AUTO: Fix typeof -> __typeof__
 * typeof without __ is NOT a keyword in strict -std=c99/c11/c17 (compile error).
 * __typeof__ works in ALL GCC/Clang modes including strict -std=c11.
 * typeof (without __) is only a keyword in C23. */
#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)
#define NUR_TRY_AUTO(DestSuffix, lhs, expr) \
    do { \
        NUR_TYPEOF(expr) _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_##DestSuffix(NUR_ERROR_OF(_nur_tmp_)); \
        } \
        (lhs) = NUR_VALUE(_nur_tmp_); \
    } while (0)
#elif defined(NUR_C23)
#define NUR_TRY_AUTO(DestSuffix, lhs, expr) \
    do { \
        typeof(expr) _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_##DestSuffix(NUR_ERROR_OF(_nur_tmp_)); \
        } \
        (lhs) = NUR_VALUE(_nur_tmp_); \
    } while (0)
#else
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
#define NUR_TRY_AUTO(DestSuffix, lhs, expr) \
    _Static_assert(0, "NUR_TRY_AUTO requires GNU extensions (__typeof__) or C23 (typeof). On MSVC or strict ISO C, use NUR_TRY_TO or NUR_TRY_GOTO.")
#endif
#endif

/**
 * Like NUR_TRY_TO but for functions returning NurResult_void.
 *
 * Usage:
 *   NUR_TRY_VOID(i32, open_file(path));
 */
#define NUR_TRY_VOID(SrcSuffix, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)

/* NUR_TRY_VOID_AUTO */
#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)
#define NUR_TRY_VOID_AUTO(expr) \
    do { \
        NUR_TYPEOF(expr) _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)
#elif defined(NUR_C23)
#define NUR_TRY_VOID_AUTO(expr) \
    do { \
        typeof(expr) _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)
#else
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
#define NUR_TRY_VOID_AUTO(expr) \
    _Static_assert(0, "NUR_TRY_VOID_AUTO requires GNU extensions (__typeof__) or C23 (typeof). On MSVC or strict ISO C, use NUR_TRY_VOID or NUR_TRY_VOID_GOTO.")
#endif
#endif

/**
 * Like NUR_TRY_VOID but specifically for chaining NurResult_void -> NurResult_void.
 * Use this when the source function also returns NurResult_void.
 * 
 * Usage:
 *   NUR_TRY_VOID_FROM_VOID(validate_input());
 */
/* NurResult_void uses flat .error (not .u.error) */
#define NUR_TRY_VOID_FROM_VOID(expr) \
    do { \
        NurResult_void _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)

#endif /* !NUR_PURIST_MODE */

/**
 * NUR_TRY_GOTO(lhs, expr, errvar, label) -- portable, leak-free.
 * On error: stores error in errvar, jumps to label for cleanup.
 * On success: lhs = value. Evaluates expr exactly once.
 *
 * Available in all modes (including NUR_PURIST_MODE) because control flow
 * is explicitly routed to label, without hidden returns.
 *
 * Usage:
 *   nur_error_t err = {0}; int val;
 *   NUR_TRY_GOTO(val, parse_int(str), err, cleanup);
 *   // ...use val...
 * cleanup:
 *   if (fp) fclose(fp);
 *   if (err.code) return nur_err_i32(err);
 *
 * Non-GNU / C89: NUR_TRY_GOTO uses pre-assigned named variable:
 *   NurResult_i32 _r = f(); NUR_TRY_GOTO(v,_r,err,lbl);
 */
#if (defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)) && !defined(NUR_C89)
#define NUR_TRY_GOTO(lhs, expr, errvar, label) \
    do { \
        __extension__ NUR_AUTO _ntg_ = (expr); \
        if (NUR_UNLIKELY(!_ntg_.ok)) { (errvar) = _ntg_.u.error; goto label; } \
        (lhs) = _ntg_.u.value; \
    } while (0)
#else
/* Non-GNU & C89 portable: user must pre-assign result to a named variable before
 * passing to NUR_TRY_GOTO to avoid double-evaluation of expr.
 * Example:
 *   NurResult_i32 _r = parse_int(str);
 *   NUR_TRY_GOTO(val, _r, err, cleanup);
 * This is documented in docs/PORTABILITY.md.
 */
#define NUR_TRY_GOTO(lhs, expr_lval, errvar, label) \
    do { \
        if (NUR_UNLIKELY(!(expr_lval).ok)) { \
            (errvar) = (expr_lval).u.error; \
            goto label; \
        } \
        (lhs) = (expr_lval).u.value; \
    } while (0)
#endif

/**
 * NUR_TRY_VOID_GOTO(expr, errvar, label) -- for NurResult_void sources.
 */
#if (defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)) && !defined(NUR_C89)
#define NUR_TRY_VOID_GOTO(expr, errvar, label) \
    do { \
        __extension__ NUR_AUTO _ntvg_ = (expr); \
        if (NUR_UNLIKELY(!_ntvg_.ok)) { (errvar) = NUR_ERROR_OF(_ntvg_); goto label; } \
    } while (0)
#else
#define NUR_TRY_VOID_GOTO(expr, errvar, label) \
    do { \
        NurResult_void _ntvg_ = (expr); \
        if (NUR_UNLIKELY(!_ntvg_.ok)) { (errvar) = NUR_ERROR_OF(_ntvg_); goto label; } \
    } while (0)
#endif

#ifndef NUR_PURIST_MODE

#if defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)

/**
 * Statement expression version of NUR_TRY (GNU extension / C++).
 * Evaluates to the unwrapped value on success.
 * Enclosing function must return NurResult_void.
 * 
 * Usage:
 *   int value = NUR_TRY(parse_number(str));
 *   // value contains the number, or function returned error
 */
#define NUR_TRY(expr) \
    __extension__ ({ \
        NUR_AUTO _nur_r_ = (expr); \
        if (NUR_UNLIKELY(!_nur_r_.ok)) { \
            return nur_err_void(_nur_r_.u.error); \
        } \
        _nur_r_.u.value; \
    })

/**
 * Propagate into a specific return type (GNU extension / C++).
 * 
 * Usage:
 *   int value = NUR_TRY_AS(i32, parse_number(str));
 */
#define NUR_TRY_AS(DestSuffix, expr) \
    __extension__ ({ \
        NUR_AUTO _nur_r_ = (expr); \
        if (NUR_UNLIKELY(!_nur_r_.ok)) { \
            return nur_err_##DestSuffix(_nur_r_.u.error); \
        } \
        _nur_r_.u.value; \
    })

#endif /* NUR_GNU_EXTENSIONS || __cplusplus */

/*============================================================================
 * CONDITION CHECKING
 *===========================================================================*/

/**
 * Return NurResult_void error if condition is false.
 * 
 * Usage:
 *   NUR_CHECK(buffer != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "buffer is NULL"));
 */
#define NUR_CHECK(cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_void(err); \
        } \
    } while (0)

/**
 * Return typed result error if condition is false.
 * 
 * Usage:
 *   NUR_CHECK_AS(i32, size > 0, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "size must be positive"));
 */
#define NUR_CHECK_AS(DestSuffix, cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_##DestSuffix(err); \
        } \
    } while (0)

#endif /* !NUR_PURIST_MODE */

/*============================================================================
 * MEMORY ALLOCATION WRAPPERS
 *===========================================================================*/

#if !defined(NUR_NO_STDLIB) && !defined(NUR_NO_ALLOC)

/**
 * malloc wrapper returning NurResult_ptr.
 * Never returns NULL inside the result - failures become error results.
 * 
 * Ownership: caller must free the returned pointer on success.
 * 
 * Usage:
 *   NurResult_ptr result = nur_malloc(1024);
 *   if (NUR_IS_OK(result)) {
 *       void *ptr = NUR_VALUE(result);
 *       // ... use ptr ...
 *       free(ptr);
 *   }
 */
NUR_NODISCARD NUR_INLINE NurResult_ptr nur_malloc(size_t size) {
    void *ptr;
    if (size == 0) {
        return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "malloc: size cannot be zero"));
    }
    
    ptr = malloc(size);
    if (ptr == NULL) {
        return nur_err_ptr(NUR_ERROR(NUR_E_OUT_OF_MEMORY, "malloc failed"));
    }
    
    return nur_ok_ptr(ptr);
}

/**
 * realloc wrapper returning NurResult_ptr.
 * 
 * Ownership:
 *   - On success: caller must free the returned pointer
 *   - On failure: original ptr remains valid and unchanged
 * 
 * Usage:
 *   NurResult_ptr result = nur_realloc(old_ptr, new_size);
 *   if (NUR_IS_OK(result)) {
 *       void *new_ptr = NUR_VALUE(result);
 *       // old_ptr is now invalid, use new_ptr
 *   } else {
 *       // old_ptr is still valid
 *   }
 */
NUR_NODISCARD NUR_INLINE NurResult_ptr nur_realloc(void *ptr, size_t size) {
    void *new_ptr;
    if (size == 0) {
        return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "realloc: size cannot be zero"));
    }
    
    new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        return nur_err_ptr(NUR_ERROR(NUR_E_OUT_OF_MEMORY, "realloc failed"));
    }
    
    return nur_ok_ptr(new_ptr);
}

/**
 * Free wrapper for API symmetry.
 * 
 * Usage:
 *   nur_free(ptr);
 */
NUR_INLINE void nur_free(void *ptr) {
    free(ptr);
}

#endif /* !NUR_NO_STDLIB && !NUR_NO_ALLOC */

/*============================================================================
 * SCOPE-BASED RESOURCE CLEANUP (GCC/Clang __attribute__((cleanup)))
 *===========================================================================*/

#ifdef NUR_GNU_EXTENSIONS

#if !defined(NUR_NO_STDLIB) && !defined(NUR_NO_ALLOC)
NUR_INLINE void nur_internal_cleanup_free(void *ptr_to_ptr) {
    if (ptr_to_ptr != NULL) {
        void **pp = (void **)ptr_to_ptr;
        if (*pp != NULL) {
            free(*pp);
            *pp = NULL;
        }
    }
}
#endif

#ifndef NUR_NO_STDIO
NUR_INLINE void nur_internal_cleanup_fclose(void *ptr_to_file) {
    if (ptr_to_file != NULL) {
        FILE **fp = (FILE **)ptr_to_file;
        if (*fp != NULL) {
            (void)fclose(*fp);
            *fp = NULL;
        }
    }
}
#endif

/**
 * Attach automatic cleanup function to a variable when it leaves scope.
 * Supported on GCC and Clang. On MSVC/standard C, use NUR_TRY_GOTO.
 *
 * Usage:
 *   NUR_CLEANUP(free_buf) char *buf = malloc(64);
 */
#define NUR_CLEANUP(func) __attribute__((cleanup(func)))

#if !defined(NUR_NO_STDLIB) && !defined(NUR_NO_ALLOC)
/**
 * Automatically frees heap memory when pointer leaves scope.
 *
 * Usage:
 *   NUR_AUTO_FREE char *buf = malloc(1024);
 */
#define NUR_AUTO_FREE __attribute__((cleanup(nur_internal_cleanup_free)))
#else
#define NUR_AUTO_FREE
#endif

#ifndef NUR_NO_STDIO
/**
 * Automatically closes FILE stream when pointer leaves scope.
 *
 * Usage:
 *   NUR_AUTO_FILE FILE *f = fopen("cfg.json", "r");
 */
#define NUR_AUTO_FILE __attribute__((cleanup(nur_internal_cleanup_fclose)))
#else
#define NUR_AUTO_FILE
#endif

#else /* Non-GNU / MSVC fallback */

#define NUR_CLEANUP(func)
#define NUR_AUTO_FREE
#define NUR_AUTO_FILE

#endif /* NUR_GNU_EXTENSIONS */

/*============================================================================
 * ERROR PRINTING UTILITIES
 *===========================================================================*/

#ifndef NUR_NO_STDIO

/**
 * Print error information to a FILE stream.
 * Format: [CODE_NAME] message (context: ...) (sys: N) at file:line in func()
 * 
 * Usage:
 *   if (NUR_IS_ERR(result)) {
 *       nur_error_print(&NUR_ERROR_OF(result), stderr);
 *   }
 */
NUR_INLINE void nur_error_print(const nur_error_t *err, FILE *f) {
    if (err == NULL || f == NULL) return;
    
    (void)fprintf(f, "[%s]", nur_error_code_name(err->code));
    
    if (err->message) {
        (void)fprintf(f, " %s", err->message);
    }

#ifdef NUR_COMPACT_ERRORS
    if (err->line > 0) {
        (void)fprintf(f, " (line: %u)", err->line);
    }
    (void)fprintf(f, "\n");
#else
    if (err->context) {
        (void)fprintf(f, " (context: %s)", err->context);
    }
    
    if (err->cause) {
        (void)fprintf(f, " (caused by: %s)", err->cause);
    }
    
    if (err->sys != 0) {
        (void)fprintf(f, " (sys: %d)", err->sys);
    }
    
    if (err->file && err->func) {
        (void)fprintf(f, " at %s:%u in %s()", err->file, err->line, err->func);
    }
    
    (void)fprintf(f, "\n");

#ifdef NUR_ENABLE_BACKTRACE
    if (err->trace_count > 0) {
        int i;
        (void)fprintf(f, "Stacktrace:\n");
#if defined(__linux__) || defined(__APPLE__)
        backtrace_symbols_fd(err->trace, err->trace_count, fileno(f));
#else
        for (i = 0; i < err->trace_count; i++) {
            (void)fprintf(f, "  [%d] %p\n", i, err->trace[i]);
        }
#endif
    }
#endif
#endif
}

/**
 * Convenience macro to print error to stderr.
 * 
 * Usage:
 *   if (NUR_IS_ERR(result)) {
 *       nur_error_eprint(&NUR_ERROR_OF(result));
 *   }
 */
#define nur_error_eprint(err) nur_error_print((err), stderr)

/**
 * Stream structured JSON error directly to a FILE stream (e.g. stdout or file).
 * Thread-safe, zero dynamic heap allocation. Appends a newline and flushes.
 *
 * Usage:
 *   nur_error_log_json(&NUR_ERROR_OF(res), stderr);
 */
NUR_INLINE void nur_error_log_json(const nur_error_t *err, FILE *f) {
    char buffer[NUR_PRINT_BUFFER_SIZE];
    size_t len;
    if (err == NULL || f == NULL) return;
    len = nur_error_json(err, buffer, sizeof(buffer));
    (void)len;
    (void)fputs(buffer, f);
    (void)fputc('\n', f);
    (void)fflush(f);
}

/**
 * Convenience macro: Stream structured JSON error directly to stderr.
 *
 * Usage:
 *   nur_error_elog_json(&NUR_ERROR_OF(res));
 */
#define nur_error_elog_json(err) nur_error_log_json((err), stderr)

#endif /* NUR_NO_STDIO */

/*============================================================================
 * TEST ASSERTION HELPERS
 *===========================================================================*/

/**
 * Asserts that result is Ok. Panics with diagnostic if error.
 */
#define NUR_ASSERT_OK(r) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            nur_internal_panic_msg("NUR_ASSERT_OK failed", NUR_ERROR_OF(r), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
    } while (0)

/**
 * Asserts that result is Err with expected error code.
 */
#define NUR_ASSERT_ERR(r, expected_code) \
    do { \
        if (NUR_UNLIKELY(NUR_IS_OK(r))) { \
            nur_internal_panic_msg("NUR_ASSERT_ERR failed: expected Err, got Ok", \
                NUR_ERROR(NUR_E_INVALID_STATE, "result was Ok"), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
        if (NUR_UNLIKELY(NUR_ERROR_OF(r).code != (expected_code))) { \
            nur_internal_panic_msg("NUR_ASSERT_ERR failed: error code mismatch", \
                NUR_ERROR_OF(r), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
    } while (0)

/**
 * Asserts that result is Ok, printing full error details before aborting.
 *
 * Usage:
 *   NUR_REQUIRE_OK(res);
 */
#define NUR_REQUIRE_OK(r) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            nur_internal_panic_result(NUR_ERROR_OF(r), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
    } while (0)

/**
 * Asserts that result is Ok with custom failure message, printing details before aborting.
 *
 * Usage:
 *   NUR_REQUIRE_OK_MSG(res, "Configuration loading failed");
 */
#define NUR_REQUIRE_OK_MSG(r, msg) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            nur_internal_panic_msg((msg), NUR_ERROR_OF(r), NUR_LOC_FILE, NUR_LOC_LINE); \
        } \
    } while (0)

/*============================================================================
 * EXTENSIONS: SAFE ARITHMETIC & FILE I/O HELPERS
 * (Disabled in NUR_LITE mode or with NUR_NO_EXTENSIONS)
 *===========================================================================*/
#if !defined(NUR_NO_EXTENSIONS) && !defined(NUR_LITE)

/**
 * Safely adds two 32-bit integers, returning NurResult_i32.
 * Returns NUR_E_OVERFLOW if integer overflow occurs.
 */
NUR_NODISCARD
NUR_INLINE NurResult_i32 nur_add_i32(int32_t a, int32_t b) {
#if defined(NUR_GNU_EXTENSIONS) && __GNUC__ >= 5
    int32_t res;
    if (__builtin_add_overflow(a, b, &res)) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow in 32-bit addition", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(res);
#else
    if ((b > 0 && a > (INT32_MAX - b)) || (b < 0 && a < (INT32_MIN - b))) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow in 32-bit addition", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(a + b);
#endif
}

/**
 * Safely subtracts two 32-bit integers, returning NurResult_i32.
 * Returns NUR_E_OVERFLOW if integer overflow/underflow occurs.
 */
NUR_NODISCARD
NUR_INLINE NurResult_i32 nur_sub_i32(int32_t a, int32_t b) {
#if defined(NUR_GNU_EXTENSIONS) && __GNUC__ >= 5
    int32_t res;
    if (__builtin_sub_overflow(a, b, &res)) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow/underflow in 32-bit subtraction", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(res);
#else
    if ((b > 0 && a < (INT32_MIN + b)) || (b < 0 && a > (INT32_MAX + b))) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow/underflow in 32-bit subtraction", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(a - b);
#endif
}

/**
 * Safely multiplies two 32-bit integers, returning NurResult_i32.
 * Returns NUR_E_OVERFLOW if integer overflow occurs.
 */
NUR_NODISCARD
NUR_INLINE NurResult_i32 nur_mul_i32(int32_t a, int32_t b) {
#if defined(NUR_GNU_EXTENSIONS) && __GNUC__ >= 5
    int32_t res;
    if (__builtin_mul_overflow(a, b, &res)) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow in 32-bit multiplication", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(res);
#else
    int64_t prod = (int64_t)a * (int64_t)b;
    if (prod > INT32_MAX || prod < INT32_MIN) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow in 32-bit multiplication", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32((int32_t)prod);
#endif
}

/**
 * Safely divides two 32-bit integers, returning NurResult_i32.
 * Returns NUR_E_INVALID_ARGUMENT on division by zero, or NUR_E_OVERFLOW on INT32_MIN / -1.
 */
NUR_NODISCARD
NUR_INLINE NurResult_i32 nur_div_i32(int32_t a, int32_t b) {
    if (b == 0) {
        return nur_err_i32(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0,
            "Division by zero", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    if (a == INT32_MIN && b == -1) {
        return nur_err_i32(nur_internal_make_error(NUR_E_OVERFLOW, 0,
            "Integer overflow in division (INT32_MIN / -1)", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_i32(a / b);
}

#ifndef NUR_NO_STDIO
/**
 * Safely reads an entire text file into a caller-supplied buffer with zero heap allocation.
 * Guarantees automatic file closure, null termination, and robust error classification.
 *
 * Usage:
 *   char buf[512];
 *   NurResult_cstr r = nur_file_read_text("config.json", buf, sizeof(buf));
 *   if (NUR_IS_OK(r)) { printf("%s\n", NUR_VALUE(r)); }
 */
NUR_NODISCARD
NUR_INLINE NurResult_cstr nur_file_read_text(const char *path, char *buf, size_t buf_size) {
    FILE *f;
    size_t read_bytes;
    if (!path || !buf || buf_size == 0) {
        return nur_err_cstr(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0,
            "Invalid argument passed to nur_file_read_text", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    f = fopen(path, "rb");
    if (!f) {
        return nur_err_cstr(nur_internal_make_error(
            nur_error_code_from_errno(errno), errno,
            "Failed to open file", path,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    read_bytes = fread(buf, 1, buf_size - 1, f);
    if (ferror(f)) {
        (void)fclose(f);
        return nur_err_cstr(nur_internal_make_error(
            NUR_E_IO, errno,
            "I/O error while reading file", path,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    /* Guard against silent truncation: verify file does not have remaining unread bytes */
    if (read_bytes == buf_size - 1) {
        int next_ch = fgetc(f);
        if (next_ch != EOF) {
            (void)fclose(f);
            return nur_err_cstr(nur_internal_make_error(
                NUR_E_TRUNCATED, 0,
                "Buffer capacity insufficient: file truncated", path,
                NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
        }
        if (ferror(f)) {
            (void)fclose(f);
            return nur_err_cstr(nur_internal_make_error(
                NUR_E_IO, errno,
                "I/O error while reading file", path,
                NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
        }
    }
    buf[read_bytes] = '\0';
    (void)fclose(f);
    return nur_ok_cstr(buf);
}
#endif /* !NUR_NO_STDIO */

/**
 * Safely computes square root with domain protection, returning NurResult_f64.
 * Returns NUR_E_INVALID_ARGUMENT on negative input, preventing silent NaN generation.
 * Implemented using pure, high-precision Newton-Raphson approximation (no <math.h> needed).
 *
 * Usage:
 *   NurResult_f64 r = nur_sqrt(16.0);
 *   if (NUR_IS_OK(r)) { printf("%f\n", NUR_VALUE(r)); }
 */
NUR_NODISCARD
NUR_INLINE NurResult_f64 nur_sqrt(double x) {
    if (x < 0.0) {
        return nur_err_f64(nur_internal_make_error(NUR_E_INVALID_ARGUMENT, 0,
            "Square root of negative number is undefined in real domain", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    if (x == 0.0) {
        return nur_ok_f64(0.0);
    }
    {
        double guess = x > 1.0 ? x * 0.5 : 1.0;
        int iter;
        for (iter = 0; iter < 64; ++iter) {
            double next = 0.5 * (guess + (x / guess));
            if (next == guess) break;
            guess = next;
        }
        return nur_ok_f64(guess);
    }
}

#ifndef NUR_NO_STRING
/**
 * Safely computes string length with null-pointer protection, returning NurResult_usize.
 * Returns NUR_E_NULL_POINTER if str is NULL, preventing segmentation fault.
 *
 * Usage:
 *   NurResult_usize r = nur_strlen("Hello");
 *   if (NUR_IS_OK(r)) { printf("%zu\n", NUR_VALUE(r)); }
 */
NUR_NODISCARD
NUR_INLINE NurResult_usize nur_strlen(const char *str) {
    if (str == NULL) {
        return nur_err_usize(nur_internal_make_error(NUR_E_NULL_POINTER, 0,
            "Cannot calculate length of NULL string pointer", NULL,
            NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE));
    }
    return nur_ok_usize(strlen(str));
}
#endif /* !NUR_NO_STRING */

/*============================================================================
 * COMPILE-TIME SAFETY & DIAGNOSTIC GUARDS
 *===========================================================================*/
#ifndef NUR_NO_SECURITY_SHIELD

#if defined(NUR_C11) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
    #define gets(buf) _Static_assert(0, "[nur.h Security Shield] 'gets()' is inherently unsafe and banned due to buffer overflow! Use 'fgets()' instead.")
    #define prinf(...) _Static_assert(0, "[nur.h Typo Guard] Typo detected: 'prinf' is not a valid C function. Did you mean 'printf'?")
#elif (defined(__GNUC__) || defined(__clang__))
    #define gets(buf) do { \
        extern void nur_internal_shield_banned_gets(void) __attribute__((error("[nur.h Security Shield] 'gets()' is inherently unsafe and banned! Use 'fgets()' instead."))); \
        nur_internal_shield_banned_gets(); \
    } while(0)
    #if defined(NUR_C99) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
    #define prinf(...) do { \
        extern void nur_internal_shield_typo_prinf(void) __attribute__((error("[nur.h Typo Guard] Typo detected: 'prinf'. Did you mean 'printf'?"))); \
        nur_internal_shield_typo_prinf(); \
    } while(0)
    #endif
#endif

#ifdef NUR_STRICT_POISON
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC poison gets
        #pragma GCC poison strcpy
        #pragma GCC poison strcat
    #endif
#endif

#endif /* !NUR_NO_SECURITY_SHIELD */

#endif /* !NUR_NO_EXTENSIONS && !NUR_LITE */

#ifdef __cplusplus
} /* extern "C" */

/*============================================================================
 * C++ NAMESPACE WRAPPER (namespace nur)
 * Provides clean, modern C++ type aliases and wrappers without macros.
 * Active ONLY when compiling with a C++ compiler (__cplusplus).
 * Zero runtime overhead, zero impact on pure ISO C compilation.
 *===========================================================================*/
namespace nur {
    /* Core Types */
    using error_t = ::nur_error_t;
    using error_code_t = ::nur_error_code_t;

    /* Result types */
    using Result_i32 = ::NurResult_i32;
    using Result_i64 = ::NurResult_i64;
    using Result_usize = ::NurResult_usize;
    using Result_f32 = ::NurResult_f32;
    using Result_f64 = ::NurResult_f64;
    using Result_cstr = ::NurResult_cstr;
    using Result_cstr_const = ::NurResult_cstr_const;
    using Result_ptr = ::NurResult_ptr;
    using Result_bool = ::NurResult_bool;
    using Result_void = ::NurResult_void;

    /* Option types */
    using Option_i32 = ::NurOption_i32;
    using Option_i64 = ::NurOption_i64;
    using Option_usize = ::NurOption_usize;
    using Option_f32 = ::NurOption_f32;
    using Option_f64 = ::NurOption_f64;
    using Option_cstr = ::NurOption_cstr;
    using Option_ptr = ::NurOption_ptr;
    using Option_bool = ::NurOption_bool;

    /* Void constructors */
    inline ::NurResult_void ok() { return ::nur_ok_void(); }
    inline ::NurResult_void err(const ::nur_error_t &e) { return ::nur_err_void(e); }

    /* Single-token constructors using C++ template traits */
    template <typename T>
    inline auto ok(T val) -> decltype(::NUR_OK(val)) {
        return ::NUR_OK(val);
    }

    template <typename T>
    inline bool is_ok(const T &res) {
        return NUR_IS_OK(res);
    }

    template <typename T>
    inline bool is_err(const T &res) {
        return NUR_IS_ERR(res);
    }

    template <typename T>
    inline auto value(const T &res) -> decltype(res.u.value) {
        return res.u.value;
    }

    template <typename T>
    inline const ::nur_error_t& error_of(const T &res) {
        return res.u.error;
    }

    template <typename T, typename D>
    inline D unwrap_or(const T &res, D default_val) {
        return res.ok ? static_cast<D>(res.u.value) : default_val;
    }

#ifndef NUR_NO_MATH
    inline ::NurResult_f64 sqrt(double x) { return ::nur_sqrt(x); }
#endif
#ifndef NUR_NO_STRING
    inline ::NurResult_usize strlen(const char *str) { return ::nur_strlen(str); }
#endif
}
#endif

#endif /* NUR_H_INCLUDED */
