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

#ifndef NUR_H
#define NUR_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * VERSION INFORMATION
 *===========================================================================*/
#define NUR_VERSION_MAJOR 1
#define NUR_VERSION_MINOR 1
#define NUR_VERSION_PATCH 0
#define NUR_VERSION_STRING "1.1.0"
#define NUR_VERSION_NUMBER 10100

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
#endif

/*============================================================================
 * COMPILER DETECTION
 *===========================================================================*/
#if defined(__GNUC__) || defined(__clang__)
    #define NUR_GNU_EXTENSIONS 1
#endif

#if defined(_MSC_VER)
    #define NUR_MSVC 1
#endif

/* Type deduction macros */
#if defined(__cplusplus)
    #define NUR_TYPEOF(expr) decltype(expr)
    #define NUR_AUTO auto
#elif defined(NUR_C23)
    #define NUR_TYPEOF(expr) typeof(expr)
    #define NUR_AUTO auto
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_TYPEOF(expr) __typeof__(expr)
    #define NUR_AUTO __auto_type
#endif

/*============================================================================
 * REQUIRED HEADERS
 *===========================================================================*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef NUR_NO_ERRNO
    #include <errno.h>
#endif

#ifndef NUR_NO_STRING
    #include <string.h>
#endif

#ifndef NUR_NO_STDIO
    #include <stdio.h>
#endif

#ifndef NUR_NO_STDLIB
    #include <stdlib.h>
#endif

/*============================================================================
 * ATTRIBUTE MACROS
 *===========================================================================*/

/* [[nodiscard]] - warns when result is ignored */
#ifdef NUR_C23
    #define NUR_NODISCARD [[nodiscard]]
    #define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
    #define NUR_NODISCARD_TYPE [[nodiscard]]
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

/* Unused attribute to prevent -Wunused-function in headers */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_UNUSED __attribute__((unused))
#else
    #define NUR_UNUSED
#endif

/* Force inlining
 * NUR_INLINE does NOT use always_inline -- panic helpers are noinline+cold.
 * Removing always_inline avoids inlining the 1 KiB panic buffer at every
 * NUR_UNWRAP call site (~11x code size reduction). */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_INLINE  static inline
    #define NUR_NOINLINE __attribute__((noinline))
#elif defined(NUR_MSVC)
    #define NUR_INLINE   static __forceinline
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
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_COLD __attribute__((cold))
#else
    #define NUR_COLD
#endif

/* Source location tracking configuration */
#ifdef NUR_NO_SOURCE_LOCATION
    #define NUR_LOC_FILE ((const char *)0)
    #define NUR_LOC_FUNC ((const char *)0)
    #define NUR_LOC_LINE 0U
#else
    #define NUR_LOC_FILE __FILE__
    #define NUR_LOC_FUNC __func__
    #define NUR_LOC_LINE ((unsigned int)__LINE__)
#endif

/* Branch prediction hints */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define NUR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define NUR_LIKELY(x)   (x)
    #define NUR_UNLIKELY(x) (x)
#endif

/* Printf-style format checking */
#if defined(NUR_GNU_EXTENSIONS)
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
typedef enum {
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
    NUR_E_USER_BASE = 1000
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
typedef struct {
    nur_error_code_t code;     
    int              sys;      
    const char      *message;  
    const char      *context;  
    const char      *cause;    
    const char      *file;     
    const char      *func;     
    unsigned int     line;     
#ifdef NUR_ENABLE_BACKTRACE
    void *trace[16];
    int trace_count;
#endif
} nur_error_t;

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

NUR_INLINE nur_error_t nur_internal_make_error(
    nur_error_code_t code, int sys,
    const char *message, const char *context,
    const char *file, const char *func, unsigned int line
) {
    nur_error_t err;
    err.code = code;
    err.sys = sys;
    err.message = message;
    err.context = context;
    err.cause = (const char *)0;
    err.file = file;
    err.func = func;
    err.line = line;
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
}

/**
 * Wrap an existing error with new context, PRESERVING original code, sys,
 * message, file, func, line, and backtrace.
 *
 * ROOT CAUSE CHAINING: If the error already had a .context, the previous
 * context is automatically migrated into .cause so the origin isn't lost!
 *
 * Usage: NUR_WRAP_ERROR(NUR_ERROR_OF(res), "failed to parse config")
 */
NUR_INLINE nur_error_t nur_error_with_context(nur_error_t e, const char *ctx) {
    if (e.context != NULL && e.cause == NULL) {
        e.cause = e.context;
    }
    e.context = ctx;
    return e;
}

/**
 * Explicitly wrap an error with a high-level context and specific root cause.
 */
NUR_INLINE nur_error_t nur_error_with_cause(nur_error_t e, const char *ctx, const char *cause_str) {
    e.context = ctx;
    e.cause = cause_str;
    return e;
}

#define NUR_WRAP_ERROR(err_val, new_ctx) \
    nur_error_with_context((err_val), (new_ctx))

/**
 * Read the root cause of an error (NULL if no cause attached).
 */
#define NUR_ERROR_CAUSE(err) ((err).cause)

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
#define NUR_RESULT_TYPE(T, suffix) \
    typedef struct NUR_NODISCARD_TYPE NurResult_##suffix { \
        bool ok; \
        union { \
            T            value; \
            nur_error_t  error; \
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
    NUR_INLINE T nur_unwrap_or_##suffix(NurResult_##suffix r_, T fallback_) { \
        return r_.ok ? r_.u.value : fallback_; \
    } \
    struct _nur_dummy_decl_##suffix

/*============================================================================
 * PREDEFINED RESULT TYPES
 *===========================================================================*/

/* Numeric types */
NUR_RESULT_TYPE(int32_t, i32);
NUR_RESULT_TYPE(int64_t, i64);
NUR_RESULT_TYPE(size_t, usize);
NUR_RESULT_TYPE(float, f32);
NUR_RESULT_TYPE(double, f64);

/* Pointer types */
NUR_RESULT_TYPE(char *, cstr);
NUR_RESULT_TYPE(void *, ptr);

/**
 * Void result type - represents success/failure without a value.
 *
 * Layout fix: simplified to a flat struct with a single .error field.
 * The previous union-based design caused .error vs .u.error aliasing
 * mismatches on MSVC/pre-C11 paths, leading to garbage error codes.
 *
 * Access: use r.error (or NUR_VOID_ERROR_OF(r)).
 * For NurResult_<T>, use NUR_ERROR_OF(r) which accesses .u.error.
 */
typedef struct NUR_NODISCARD_TYPE NurResult_void {
    bool ok;
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11) || (defined(_MSC_VER) && !defined(__GNUC__) && !defined(__clang__))
    union {
        nur_error_t error;
        struct {
            nur_error_t error;
        } u;
    };
#else
    nur_error_t error;
#endif
} NurResult_void;

#define NUR_VOID_ERROR_OF(r) ((r).error)

/**
 * Create a successful void result. (Freestanding-safe: no memset needed)
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_ok_void(void) {
    NurResult_void r;
    r.ok = true;
    r.error.code = NUR_E_NONE;
    r.error.sys = 0;
    r.error.message = (const char *)0;
    r.error.context = (const char *)0;
    r.error.cause = (const char *)0;
    r.error.file = (const char *)0;
    r.error.func = (const char *)0;
    r.error.line = 0U;
#ifdef NUR_ENABLE_BACKTRACE
    r.error.trace_count = 0;
#endif
    return r;
}

/**
 * Create a failed void result with the given error.
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_err_void(nur_error_t err_) {
    NurResult_void r;
    r.ok = false;
    r.error = err_;
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
        o_.some = false; \
        unsigned char *p_ = (unsigned char *)(void *)&o_.value; \
        for (size_t i_ = 0; i_ < sizeof(T); ++i_) p_[i_] = 0; \
        return o_; \
    } \
    struct _nur_dummy_opt_decl_##suffix

/*============================================================================
 * PREDEFINED OPTION TYPES
 *===========================================================================*/

NUR_OPTION_TYPE(int32_t, i32);
NUR_OPTION_TYPE(int64_t, i64);  
NUR_OPTION_TYPE(size_t, usize);
NUR_OPTION_TYPE(char *, cstr);
NUR_OPTION_TYPE(void *, ptr);

/*============================================================================
 * CONVENIENCE MACROS
 *===========================================================================*/

/* Result constructors */
#define NUR_OK(suffix, val)   nur_ok_##suffix(val)
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
        case 109UL: case 232UL: case 10054UL: case 10057UL:
            return NUR_E_CONNECTION;
        /* 10053 = WSAECONNABORTED -> CONNECTION (fix: was NUR_E_REFUSED) */
        case 10053UL:
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
        case EACCES:    return NUR_E_PERMISSION;
#endif
#ifdef EPERM
        case EPERM:     return NUR_E_PERMISSION;
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
        case ENOBUFS:   return NUR_E_RESOURCE_LIMIT;
#endif
#ifdef ENFILE
        case ENFILE:    return NUR_E_RESOURCE_LIMIT;
#endif
#ifdef EMFILE
        case EMFILE:    return NUR_E_RESOURCE_LIMIT;
#endif
#ifdef EPIPE
        case EPIPE:     return NUR_E_CLOSED;
#endif
#ifdef EBADF
        case EBADF:     return NUR_E_INVALID_ARGUMENT;
#endif
#ifdef EINVAL
        case EINVAL:    return NUR_E_INVALID_ARGUMENT;
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
        case ECONNRESET:   return NUR_E_CONNECTION;
#endif
#ifdef ENOTCONN
        case ENOTCONN:     return NUR_E_CONNECTION;
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

NUR_INLINE void nur_internal_sink_putu(nur_internal_sink_t *sink, unsigned long long value) {
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

NUR_INLINE void nur_internal_sink_puti(nur_internal_sink_t *sink, long long value) {
    if (value < 0) {
        nur_internal_sink_putc(sink, '-');
        /* Fix -Wsign-conversion: cast through unsigned long long explicitly */
        nur_internal_sink_putu(sink, (unsigned long long)(0ULL - (unsigned long long)value));
    } else {
        nur_internal_sink_putu(sink, (unsigned long long)value);
    }
}

/**
 * Renders error into buffer as a single line.
 * Returns required length excluding NUL. If >= capacity, output was truncated.
 *
 * Fix: if error is NULL, writes NUL to buffer[0] so buf is always valid.
 */
NUR_INLINE size_t nur_error_format(const nur_error_t *error, char *buffer, size_t capacity) {
    if (!error) {
        if (buffer && capacity > 0) buffer[0] = '\0';
        return 0;
    }
    nur_internal_sink_t sink = { buffer, capacity, 0 };
    
    nur_internal_sink_puts(&sink, nur_error_code_name(error->code));
    nur_internal_sink_puts(&sink, ": ");
    nur_internal_sink_puts(&sink, error->message ? error->message : "unknown");
    
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
    if (!error) {
        if (buffer && capacity > 0) {
            static const char null_json[] = "{\"ok\":true}";
            size_t len = sizeof(null_json) - 1;
            size_t copy_len = len < capacity ? len : capacity - 1;
            for (size_t i = 0; i < copy_len; ++i) buffer[i] = null_json[i];
            buffer[copy_len] = '\0';
            return len;
        }
        return 11;
    }
    nur_internal_sink_t sink = { buffer, capacity, 0 };
    nur_internal_sink_puts(&sink, "{\"ok\":false,\"code\":");
    nur_internal_sink_putu(&sink, (unsigned long long)error->code);
    nur_internal_sink_puts(&sink, ",\"name\":\"");
    nur_internal_sink_puts(&sink, nur_error_code_name(error->code));
    nur_internal_sink_puts(&sink, "\",\"message\":");
    if (error->message) {
        nur_internal_sink_putc(&sink, '"');
        const char *p = error->message;
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

    if (error->context && error->context[0] != '\0') {
        nur_internal_sink_puts(&sink, ",\"context\":\"");
        const char *p = error->context;
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
        const char *p = error->cause;
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

    if (error->sys != 0) {
        nur_internal_sink_puts(&sink, ",\"sys\":");
        nur_internal_sink_puti(&sink, error->sys);
    }

    if (error->file) {
        nur_internal_sink_puts(&sink, ",\"file\":\"");
        const char *p = error->file;
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
    nur_internal_sink_putc(&sink, '}');

    if (buffer && capacity > 0) {
        buffer[sink.length < capacity ? sink.length : capacity - 1] = '\0';
    }
    return sink.length;
}

/*============================================================================
 * PANIC HELPERS (require stdio)
 *===========================================================================*/

#ifndef NUR_NO_STDIO
#ifndef NUR_NO_STDLIB

#ifndef NUR_PRINT_BUFFER_SIZE
#define NUR_PRINT_BUFFER_SIZE 1024
#endif

/*
 * Configurable Panic Handler
 * By default, abort() is called after printing to stderr.
 * Users can register a custom handler to log, unwind, or recover.
 */
typedef void (*nur_panic_handler_t)(const char *msg, const nur_error_t *e,
                                    const char *file, unsigned int line);

#ifndef NUR_CUSTOM_PANIC_HANDLER_STORAGE
#define NUR_CUSTOM_PANIC_HANDLER_STORAGE static
#endif

NUR_CUSTOM_PANIC_HANDLER_STORAGE nur_panic_handler_t _nur_custom_panic_handler = NULL;

/**
 * Set custom panic handler. Set NULL to restore default stderr+abort behaviour.
 */
NUR_INLINE void nur_set_panic_handler(nur_panic_handler_t handler) {
    _nur_custom_panic_handler = handler;
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
    if (_nur_custom_panic_handler != NULL) {
        _nur_custom_panic_handler(NULL, &e, file, line);
    }
    char buffer[NUR_PRINT_BUFFER_SIZE];
    nur_error_format(&e, buffer, sizeof(buffer));
    fprintf(stderr, "nur panic at %s:%u: %s\n", file ? file : "<unknown>", line, buffer);
    fflush(stderr);
    abort();
}

NUR_NOINLINE NUR_COLD NUR_NORETURN NUR_UNUSED
static void nur_internal_panic_msg(const char *msg, nur_error_t e,
                                   const char *file, unsigned int line) {
    if (_nur_custom_panic_handler != NULL) {
        _nur_custom_panic_handler(msg, &e, file, line);
    }
    char buffer[NUR_PRINT_BUFFER_SIZE];
    nur_error_format(&e, buffer, sizeof(buffer));
    fprintf(stderr, "nur panic at %s:%u: %s: %s\n", file ? file : "<unknown>", line, msg, buffer);
    fflush(stderr);
    abort();
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

/* Non-GNU: evaluates 'r' twice (once for .ok, once for .u.value / .u.error).
 * IMPORTANT: Store the result in a named variable before calling if the
 * expression has side effects (e.g., a function call with observable effects).
 * Example safe usage:
 *   NurResult_i32 _r = parse_int(str);
 *   int val = NUR_UNWRAP(_r);
 * See docs/PORTABILITY.md for full guidance. */
#define NUR_UNWRAP(r) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_result((r).u.error, NUR_LOC_FILE, NUR_LOC_LINE), (r).u.value))

#define NUR_EXPECT(r, msg) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_msg((msg), (r).u.error, NUR_LOC_FILE, NUR_LOC_LINE), (r).u.value))

#endif

#endif /* !NUR_NO_STDLIB */
#endif /* !NUR_NO_STDIO */

/*============================================================================
 * SAFE VALUE EXTRACTION
 *===========================================================================*/

#if defined(NUR_GNU_EXTENSIONS)

/**
 * Get value or fallback if error (evaluates r exactly once).
 * 
 * Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);
 */
#define NUR_UNWRAP_OR(r, fallback) \
    __extension__ ({ \
        __auto_type _nur_unwrap_or_r_ = (r); \
        NUR_LIKELY(_nur_unwrap_or_r_.ok) ? _nur_unwrap_or_r_.u.value : (fallback); \
    })

/**
 * Get option value or fallback if None (evaluates o exactly once).
 * 
 * Usage: int val = NUR_OPTION_UNWRAP_OR(opt, 0);
 */
#define NUR_OPTION_UNWRAP_OR(o, fallback) \
    __extension__ ({ \
        __auto_type _nur_opt_unwrap_or_o_ = (o); \
        NUR_LIKELY(_nur_opt_unwrap_or_o_.some) ? _nur_opt_unwrap_or_o_.value : (fallback); \
    })

/**
 * Explicitly ignore a result - documents intent and suppresses compiler warnings.
 * 
 * Usage: NUR_IGNORE(write_log(msg));
 */
#define NUR_IGNORE(expr) \
    do { \
        __auto_type _nur_ignored_ = (expr); \
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
        NurResult_i32:   nur_unwrap_or_i32, \
        NurResult_i64:   nur_unwrap_or_i64, \
        NurResult_usize: nur_unwrap_or_usize, \
        NurResult_f32:   nur_unwrap_or_f32, \
        NurResult_f64:   nur_unwrap_or_f64, \
        NurResult_cstr:  nur_unwrap_or_cstr, \
        NurResult_ptr:   nur_unwrap_or_ptr \
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

#if defined(NUR_GNU_EXTENSIONS)

/**
 * Map a successful result through a transform function, preserving error.
 *
 * NUR_MAP(result, func) evaluates result once. If Ok, applies func to the
 * value and returns a new Ok with the result. If Err, returns the error.
 *
 * IMPORTANT: func must accept the payload type and return the SAME type.
 * For type-changing maps, use NUR_AND_THEN with a wrapper function that
 * returns the new NurResult_<suffix> type.
 *
 * GNU extension: uses statement expressions for single-evaluation.
 *
 * Usage:
 *   static int double_it(int v) { return v * 2; }
 *   NurResult_i32 r2 = NUR_MAP(r, double_it);
 */
#define NUR_MAP(result, func) \
    __extension__ ({ \
        __auto_type _nur_map_r_ = (result); \
        if (_nur_map_r_.ok) { \
            _nur_map_r_.u.value = (func)(_nur_map_r_.u.value); \
        } \
        _nur_map_r_; \
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
        __auto_type _nur_at_r_ = (result); \
        _nur_at_r_.ok ? (func)(_nur_at_r_.u.value) : _nur_at_r_; \
    })

#else /* Non-GNU */

/**
 * NUR_MAP - Non-GNU version.
 *
 * NON-GNU LIMITATION: Without statement expressions, NUR_MAP cannot both
 * transform the value and return the same result type in a single expression.
 * On non-GNU compilers, use the explicit pattern instead:
 *
 *   if (NUR_IS_OK(r)) { r.u.value = (func)(r.u.value); }
 *   // r now has the transformed value
 *
 * Or use NUR_AND_THEN which works on non-GNU (with double-eval caveat).
 * For full NUR_MAP support, compile with GCC or Clang (-std=c11 or later).
 *
 * DOUBLE-EVAL NOTE: 'result' is evaluated TWICE. Pre-assign to a variable:
 *   NurResult_i32 _r = compute();
 *   NurResult_i32 r2 = NUR_AND_THEN(_r, your_result_returning_func);
 *
 * NUR_MAP is defined as a passthrough (returns result unchanged) on non-GNU.
 */
#define NUR_MAP(result, func) (result)
/* Passthrough stub: the caller should apply (func) manually on success. */

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
 * EARLY-RETURN HELPERS
 *===========================================================================*/

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

#if defined(NUR_GNU_EXTENSIONS) || defined(NUR_C23)
/**
 * Type-inferring pattern match construct (GNU/C23).
 *
 * Usage:
 *   NUR_MATCH(res, val, { printf("Val: %d\n", val); }, err, { nur_error_eprint(&err); });
 */
#define NUR_MATCH(r, val_var, ok_body, err_var, err_body) \
    do { \
        __auto_type _nur_m_ = (r); \
        if (NUR_IS_OK(_nur_m_)) { \
            __auto_type val_var = _nur_m_.u.value; \
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
#if defined(NUR_GNU_EXTENSIONS)
#define NUR_TRY_AUTO(DestSuffix, lhs, expr) \
    do { \
        __typeof__(expr) _nur_tmp_ = (expr); \
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

/* NUR_TRY_VOID_AUTO: same typeof -> __typeof__ fix */
#if defined(NUR_GNU_EXTENSIONS)
#define NUR_TRY_VOID_AUTO(expr) \
    do { \
        __typeof__(expr) _nur_tmp_ = (expr); \
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
            return nur_err_void(_nur_tmp_.error); \
        } \
    } while (0)

/**
 * NUR_TRY_GOTO(lhs, expr, errvar, label) -- portable, leak-free.
 * On error: stores error in errvar, jumps to label for cleanup.
 * On success: lhs = value. Evaluates expr exactly once.
 *
 * Usage:
 *   nur_error_t err = {0}; int val;
 *   NUR_TRY_GOTO(val, parse_int(str), err, cleanup);
 *   // ...use val...
 * cleanup:
 *   if (fp) fclose(fp);
 *   if (err.code) return nur_err_i32(err);
 *
 * Non-GNU: NUR_TRY_GOTO uses __auto_type via GNU block. For MSVC, store
 * result in a named variable first: NurResult_i32 _r = f(); NUR_TRY_GOTO(v,_r,err,lbl);
 */
#if defined(NUR_GNU_EXTENSIONS)
#define NUR_TRY_GOTO(lhs, expr, errvar, label) \
    do { \
        __extension__ __auto_type _ntg_ = (expr); \
        if (NUR_UNLIKELY(!_ntg_.ok)) { (errvar) = _ntg_.u.error; goto label; } \
        (lhs) = _ntg_.u.value; \
    } while (0)
#else
/* Non-GNU portable: user must pre-assign result to a named variable before
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
#if defined(NUR_GNU_EXTENSIONS)
#define NUR_TRY_VOID_GOTO(expr, errvar, label) \
    do { \
        __extension__ __auto_type _ntvg_ = (expr); \
        if (NUR_UNLIKELY(!_ntvg_.ok)) { (errvar) = _ntvg_.error; goto label; } \
    } while (0)
#else
#define NUR_TRY_VOID_GOTO(expr, errvar, label) \
    do { \
        NurResult_void _ntvg_ = (expr); \
        if (NUR_UNLIKELY(!_ntvg_.ok)) { (errvar) = _ntvg_.error; goto label; } \
    } while (0)
#endif

#ifdef NUR_GNU_EXTENSIONS

/**
 * Statement expression version of NUR_TRY (GNU extension).
 * Evaluates to the unwrapped value on success.
 * Enclosing function must return NurResult_void.
 * 
 * Usage:
 *   int value = NUR_TRY(parse_number(str));
 *   // value contains the number, or function returned error
 */
#define NUR_TRY(expr) \
    __extension__ ({ \
        __auto_type _nur_r_ = (expr); \
        if (NUR_UNLIKELY(!_nur_r_.ok)) { \
            return nur_err_void(_nur_r_.u.error); \
        } \
        _nur_r_.u.value; \
    })

/**
 * Propagate into a specific return type (GNU extension).
 * 
 * Usage:
 *   int value = NUR_TRY_AS(i32, parse_number(str));
 */
#define NUR_TRY_AS(DestSuffix, expr) \
    __extension__ ({ \
        __auto_type _nur_r_ = (expr); \
        if (NUR_UNLIKELY(!_nur_r_.ok)) { \
            return nur_err_##DestSuffix(_nur_r_.u.error); \
        } \
        _nur_r_.u.value; \
    })

#endif /* NUR_GNU_EXTENSIONS */

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
    if (size == 0) {
        return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "malloc: size cannot be zero"));
    }
    
    void *ptr = malloc(size);
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
    if (size == 0) {
        return nur_err_ptr(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "realloc: size cannot be zero"));
    }
    
    void *new_ptr = realloc(ptr, size);
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

#if defined(NUR_GNU_EXTENSIONS)

NUR_INLINE void nur_internal_cleanup_free(void *ptr_to_ptr) {
    if (ptr_to_ptr != NULL) {
        void **pp = (void **)ptr_to_ptr;
        if (*pp != NULL) {
            free(*pp);
            *pp = NULL;
        }
    }
}

#ifndef NUR_NO_STDIO
NUR_INLINE void nur_internal_cleanup_fclose(void *ptr_to_file) {
    if (ptr_to_file != NULL) {
        FILE **fp = (FILE **)ptr_to_file;
        if (*fp != NULL) {
            fclose(*fp);
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

/**
 * Automatically frees heap memory when pointer leaves scope.
 *
 * Usage:
 *   NUR_AUTO_FREE char *buf = malloc(1024);
 */
#define NUR_AUTO_FREE __attribute__((cleanup(nur_internal_cleanup_free)))

#ifndef NUR_NO_STDIO
/**
 * Automatically closes FILE stream when pointer leaves scope.
 *
 * Usage:
 *   NUR_AUTO_FILE FILE *f = fopen("cfg.json", "r");
 */
#define NUR_AUTO_FILE __attribute__((cleanup(nur_internal_cleanup_fclose)))
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
    
    fprintf(f, "[%s]", nur_error_code_name(err->code));
    
    if (err->message) {
        fprintf(f, " %s", err->message);
    }
    
    if (err->context) {
        fprintf(f, " (context: %s)", err->context);
    }
    
    if (err->cause) {
        fprintf(f, " (caused by: %s)", err->cause);
    }
    
    if (err->sys != 0) {
        fprintf(f, " (sys: %d)", err->sys);
    }
    
    if (err->file && err->func) {
        fprintf(f, " at %s:%u in %s()", err->file, err->line, err->func);
    }
    
    fprintf(f, "\n");

#ifdef NUR_ENABLE_BACKTRACE
    if (err->trace_count > 0) {
        fprintf(f, "Stacktrace:\n");
#if defined(__linux__) || defined(__APPLE__)
        backtrace_symbols_fd(err->trace, err->trace_count, fileno(f));
#else
        for (int i = 0; i < err->trace_count; i++) {
            fprintf(f, "  [%d] %p\n", i, err->trace[i]);
        }
#endif
    }
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
    if (err == NULL || f == NULL) return;
    char buffer[512];
    size_t len = nur_error_json(err, buffer, sizeof(buffer));
    if (len < sizeof(buffer)) {
        fputs(buffer, f);
    } else {
        fputs(buffer, f);
    }
    fputc('\n', f);
    fflush(f);
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

#ifndef NUR_NO_STDIO
#ifndef NUR_NO_STDLIB

/**
 * Asserts that result is Ok. Aborts with diagnostic if error.
 */
#define NUR_ASSERT_OK(r) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            fprintf(stderr, "NUR_ASSERT_OK failed at %s:%u\n", \
                    NUR_LOC_FILE ? NUR_LOC_FILE : "<unknown>", NUR_LOC_LINE); \
            abort(); \
        } \
    } while (0)

/**
 * Asserts that result is Err with expected error code.
 */
#define NUR_ASSERT_ERR(r, expected_code) \
    do { \
        if (NUR_UNLIKELY(NUR_IS_OK(r) || NUR_ERROR_OF(r).code != (expected_code))) { \
            fprintf(stderr, "NUR_ASSERT_ERR failed at %s:%u (expected code %d)\n", \
                    NUR_LOC_FILE ? NUR_LOC_FILE : "<unknown>", NUR_LOC_LINE, (int)(expected_code)); \
            abort(); \
        } \
    } while (0)

/**
 * Asserts that result is Ok, printing full error details to stderr before aborting.
 *
 * Usage:
 *   NUR_REQUIRE_OK(res);
 */
#define NUR_REQUIRE_OK(r) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            fprintf(stderr, "NUR_REQUIRE_OK failed at %s:%u\n", \
                    NUR_LOC_FILE ? NUR_LOC_FILE : "<unknown>", NUR_LOC_LINE); \
            nur_error_eprint(&NUR_ERROR_OF(r)); \
            abort(); \
        } \
    } while (0)

/**
 * Asserts that result is Ok with custom failure message, printing full error details before aborting.
 *
 * Usage:
 *   NUR_REQUIRE_OK_MSG(res, "Configuration loading failed");
 */
#define NUR_REQUIRE_OK_MSG(r, msg) \
    do { \
        if (NUR_UNLIKELY(!NUR_IS_OK(r))) { \
            fprintf(stderr, "NUR_REQUIRE_OK failed: %s at %s:%u\n", \
                    (msg), NUR_LOC_FILE ? NUR_LOC_FILE : "<unknown>", NUR_LOC_LINE); \
            nur_error_eprint(&NUR_ERROR_OF(r)); \
            abort(); \
        } \
    } while (0)

#endif /* !NUR_NO_STDLIB */
#endif /* !NUR_NO_STDIO */

#ifdef __cplusplus
}
#endif

#endif /* NUR_H */
