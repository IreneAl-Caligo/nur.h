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
 *
 * Copyright (c) 2024
 * SPDX-License-Identifier: MIT
 */

#ifndef NUR_H
#define NUR_H

/*
 * =============================================================================
 * SECTION 1: VERSION & CONFIG
 * =============================================================================
 */

#define NUR_VERSION_MAJOR 1
#define NUR_VERSION_MINOR 0  
#define NUR_VERSION_PATCH 0
#define NUR_VERSION_STRING "1.0.0"

/* C standard detection */
#if __STDC_VERSION__ >= 202311L
    #define NUR_C23 1
    #define NUR_C17 1
    #define NUR_C11 1  
    #define NUR_C99 1
#elif __STDC_VERSION__ >= 201710L
    #define NUR_C17 1
    #define NUR_C11 1
    #define NUR_C99 1
#elif __STDC_VERSION__ >= 201112L
    #define NUR_C11 1
    #define NUR_C99 1
#elif __STDC_VERSION__ >= 199901L
    #define NUR_C99 1
#endif

/* Compiler extensions detection */
#if defined(__GNUC__) || defined(__clang__)
    #define NUR_GNU_EXTENSIONS 1
#endif

#if defined(_MSC_VER)
    #define NUR_MSVC 1
#endif

/*
 * =============================================================================
 * SECTION 2: INCLUDES
 * =============================================================================
 */

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
    #include <stdlib.h>
#endif

/*
 * =============================================================================
 * SECTION 3: ATTRIBUTES
 * =============================================================================
 */

/* Warn when result is not checked */
#ifdef NUR_C23
    #define NUR_NODISCARD [[nodiscard]]
    #define NUR_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_NODISCARD __attribute__((warn_unused_result))
    #define NUR_NODISCARD_MSG(msg) __attribute__((warn_unused_result))
#elif defined(NUR_MSVC)
    #define NUR_NODISCARD _Check_return_
    #define NUR_NODISCARD_MSG(msg) _Check_return_
#else
    #define NUR_NODISCARD
    #define NUR_NODISCARD_MSG(msg)
#endif

/* Force inlining */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_INLINE static inline __attribute__((always_inline))
#elif defined(NUR_MSVC)
    #define NUR_INLINE static __forceinline
#else
    #define NUR_INLINE static inline
#endif

/* Branch prediction hints */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define NUR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define NUR_LIKELY(x)   (x)
    #define NUR_UNLIKELY(x) (x)
#endif

/* Printf format checking */
#if defined(NUR_GNU_EXTENSIONS)
    #define NUR_PRINTF_LIKE(fmt_idx, va_idx) __attribute__((format(printf, fmt_idx, va_idx)))
#else
    #define NUR_PRINTF_LIKE(fmt_idx, va_idx)
#endif

/*
 * =============================================================================
 * SECTION 4: ERROR SYSTEM
 * =============================================================================
 */

/**
 * Error codes following a systematic taxonomy.
 * Ranges: 1-9 input, 10-19 data, 20-29 resources,
 * 30-39 I/O, 40-49 network/async, 50-59 system, >=1000 user-defined.
 */
typedef enum {
    NUR_E_NONE = 0,
    
    /* 1-9: Input/validation errors */
    NUR_E_INVALID_ARGUMENT = 1,
    NUR_E_OUT_OF_RANGE = 2,
    NUR_E_NULL_POINTER = 3,
    NUR_E_INVALID_STATE = 4,
    NUR_E_OVERFLOW = 5,
    
    /* 10-19: Data format/parsing errors */
    NUR_E_PARSE = 10,
    NUR_E_FORMAT = 11,
    NUR_E_ENCODING = 12,
    NUR_E_CORRUPTED = 13,
    NUR_E_TRUNCATED = 14,
    
    /* 20-29: Resource management errors */
    NUR_E_OUT_OF_MEMORY = 20,
    NUR_E_RESOURCE_LIMIT = 21,
    NUR_E_BUSY = 22,
    NUR_E_QUOTA = 23,
    
    /* 30-39: I/O and filesystem errors */
    NUR_E_NOT_FOUND = 30,
    NUR_E_PERMISSION = 31,
    NUR_E_ALREADY_EXISTS = 32,
    NUR_E_IO = 33,
    NUR_E_EOF = 34,
    NUR_E_CLOSED = 35,
    
    /* 40-49: Network and asynchronous errors */
    NUR_E_TIMEOUT = 40,
    NUR_E_CANCELLED = 41,
    NUR_E_CONNECTION = 42,
    NUR_E_PROTOCOL = 43,
    NUR_E_AGAIN = 44,
    
    /* 50-59: System and internal errors */
    NUR_E_UNSUPPORTED = 50,
    NUR_E_INTERNAL = 51,
    NUR_E_BUG = 52,
    NUR_E_VERSION = 53,
    
    /* User-defined error codes start here */
    NUR_E_USER_BASE = 1000
} nur_error_code_t;

/**
 * Complete error information with source location tracking.
 * All fields are always present - source location is always captured.
 */
typedef struct {
    nur_error_code_t code;     /* Machine-readable error code */
    int              sys;      /* errno/Win32 error code, 0 if none */
    const char      *message;  /* Human-readable description (static string) */
    const char      *context;  /* Optional operation context (static string) */
    const char      *file;     /* Source file where error was raised */
    const char      *func;     /* Function where error was raised */
    unsigned int     line;     /* Line number where error was raised */
} nur_error_t;

/*
 * =============================================================================
 * SECTION 5: ERROR CONSTRUCTORS
 * =============================================================================
 */

/**
 * Internal error constructor. Do not use directly - use the macros below.
 */
NUR_INLINE nur_error_t nur__make_error(
    nur_error_code_t code, int sys,
    const char *message, const char *context,
    const char *file, const char *func, unsigned int line
) {
    nur_error_t err;
    err.code = code;
    err.sys = sys;
    err.message = message;
    err.context = context;
    err.file = file;
    err.func = func;
    err.line = line;
    return err;
}

/**
 * Create an error with code and message, capturing source location.
 */
#define NUR_ERROR(code, msg) \
    nur__make_error((code), 0, (msg), NULL, __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an error with code, message, and context, capturing source location.
 */
#define NUR_ERROR_CTX(code, msg, ctx) \
    nur__make_error((code), 0, (msg), (ctx), __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an I/O error from errno, capturing source location.
 */
#define NUR_ERROR_ERRNO(msg) \
    nur__make_error(NUR_E_IO, errno, (msg), NULL, __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an I/O error from errno with context, capturing source location.
 */
#define NUR_ERROR_ERRNO_CTX(msg, ctx) \
    nur__make_error(NUR_E_IO, errno, (msg), (ctx), __FILE__, __func__, (unsigned int)__LINE__)

/*
 * =============================================================================
 * SECTION 6: RESULT TYPE GENERATOR
 * =============================================================================
 */

/**
 * Generate a Result<T> type and its constructor functions.
 * Creates NurResult_<suffix>, nur_ok_<suffix>, and nur_err_<suffix>.
 */
#define NUR_RESULT_TYPE(T, suffix) \
    typedef struct NurResult_##suffix { \
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
    }

/* Predefined Result types for common C types */
NUR_RESULT_TYPE(int32_t, i32)
NUR_RESULT_TYPE(int64_t, i64)
NUR_RESULT_TYPE(size_t, usize)
NUR_RESULT_TYPE(float, f32)
NUR_RESULT_TYPE(double, f64)
NUR_RESULT_TYPE(char *, cstr)
NUR_RESULT_TYPE(void *, ptr)

/**
 * Special void result type - represents success/failure without a value.
 * Error field is always present but only meaningful when ok == false.
 */
typedef struct NurResult_void {
    bool ok;
    nur_error_t error; /* Always present, only meaningful when !ok */
} NurResult_void;

/**
 * Create a successful void result.
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_ok_void(void) {
    NurResult_void r;
    r.ok = true;
    /* error field left uninitialized - not meaningful for success */
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

/*
 * =============================================================================
 * SECTION 7: OPTION TYPE GENERATOR
 * =============================================================================
 */

/**
 * Generate an Option<T> type and its constructor functions.
 * Creates NurOption_<suffix>, nur_some_<suffix>, and nur_none_<suffix>.
 */
#define NUR_OPTION_TYPE(T, suffix) \
    typedef struct NurOption_##suffix { \
        bool some; \
        T    value; /* Only valid when some == true */ \
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
        /* value field left uninitialized */ \
        return o_; \
    }

/* Predefined Option types */
NUR_OPTION_TYPE(int32_t, i32)
NUR_OPTION_TYPE(int64_t, i64)  
NUR_OPTION_TYPE(size_t, usize)
NUR_OPTION_TYPE(char *, cstr)
NUR_OPTION_TYPE(void *, ptr)

/*
 * =============================================================================
 * SECTION 8: CONVENIENCE MACROS
 * =============================================================================
 */

/* Construction shortcuts */
#define NUR_OK(suffix, val)   nur_ok_##suffix(val)
#define NUR_ERR(suffix, err)  nur_err_##suffix(err)
#define NUR_OK_VOID()         nur_ok_void()
#define NUR_ERR_VOID(err)     nur_err_void(err)

#define NUR_SOME(suffix, val) nur_some_##suffix(val)
#define NUR_NONE(suffix)      nur_none_##suffix()

/* Inspection macros */
#define NUR_IS_OK(r)      ((r).ok)
#define NUR_IS_ERR(r)     (!(r).ok)
#define NUR_IS_SOME(o)    ((o).some)
#define NUR_IS_NONE(o)    (!(o).some)

/* Unsafe extraction - caller must check first! */
#define NUR_VALUE(r)          ((r).u.value)
#define NUR_ERROR_OF(r)       ((r).u.error)
#define NUR_OPTION_VALUE(o)   ((o).value)

/**
 * Returns a static string name for the error code.
 * Never returns NULL - unknown codes return "NUR_E_UNKNOWN".
 */
NUR_INLINE const char *nur_error_code_name(nur_error_code_t code) {
    switch (code) {
        case NUR_E_NONE: return "NUR_E_NONE";
        
        /* Input/validation errors */
        case NUR_E_INVALID_ARGUMENT: return "NUR_E_INVALID_ARGUMENT";
        case NUR_E_OUT_OF_RANGE: return "NUR_E_OUT_OF_RANGE";
        case NUR_E_NULL_POINTER: return "NUR_E_NULL_POINTER";
        case NUR_E_INVALID_STATE: return "NUR_E_INVALID_STATE";
        case NUR_E_OVERFLOW: return "NUR_E_OVERFLOW";
        
        /* Data format/parsing errors */
        case NUR_E_PARSE: return "NUR_E_PARSE";
        case NUR_E_FORMAT: return "NUR_E_FORMAT";
        case NUR_E_ENCODING: return "NUR_E_ENCODING";
        case NUR_E_CORRUPTED: return "NUR_E_CORRUPTED";
        case NUR_E_TRUNCATED: return "NUR_E_TRUNCATED";
        
        /* Resource management errors */
        case NUR_E_OUT_OF_MEMORY: return "NUR_E_OUT_OF_MEMORY";
        case NUR_E_RESOURCE_LIMIT: return "NUR_E_RESOURCE_LIMIT";
        case NUR_E_BUSY: return "NUR_E_BUSY";
        case NUR_E_QUOTA: return "NUR_E_QUOTA";
        
        /* I/O and filesystem errors */
        case NUR_E_NOT_FOUND: return "NUR_E_NOT_FOUND";
        case NUR_E_PERMISSION: return "NUR_E_PERMISSION";
        case NUR_E_ALREADY_EXISTS: return "NUR_E_ALREADY_EXISTS";
        case NUR_E_IO: return "NUR_E_IO";
        case NUR_E_EOF: return "NUR_E_EOF";
        case NUR_E_CLOSED: return "NUR_E_CLOSED";
        
        /* Network and asynchronous errors */
        case NUR_E_TIMEOUT: return "NUR_E_TIMEOUT";
        case NUR_E_CANCELLED: return "NUR_E_CANCELLED";
        case NUR_E_CONNECTION: return "NUR_E_CONNECTION";
        case NUR_E_PROTOCOL: return "NUR_E_PROTOCOL";
        case NUR_E_AGAIN: return "NUR_E_AGAIN";
        
        /* System and internal errors */
        case NUR_E_UNSUPPORTED: return "NUR_E_UNSUPPORTED";
        case NUR_E_INTERNAL: return "NUR_E_INTERNAL";
        case NUR_E_BUG: return "NUR_E_BUG";
        case NUR_E_VERSION: return "NUR_E_VERSION";
        
        default:
            if (code >= NUR_E_USER_BASE) {
                return "NUR_E_USER_DEFINED";
            }
            return "NUR_E_UNKNOWN";
    }
}

/* Panic helpers for NUR_UNWRAP and NUR_EXPECT */
#ifndef NUR_NO_STDIO
NUR_INLINE void nur__panic_result(const nur_error_t *e, const char *file, unsigned int line) {
    fprintf(stderr, "nur panic at %s:%u: [%s] %s", 
            file, line, nur_error_code_name(e->code), e->message ? e->message : "");
    if (e->context) fprintf(stderr, " (context: %s)", e->context);
    if (e->sys != 0) fprintf(stderr, " (sys: %d)", e->sys);
    fprintf(stderr, "\n");
    abort();
}

NUR_INLINE void nur__panic_msg(const char *msg, const nur_error_t *e, const char *file, unsigned int line) {
    fprintf(stderr, "nur panic at %s:%u: %s: [%s] %s", 
            file, line, msg, nur_error_code_name(e->code), e->message ? e->message : "");
    if (e->context) fprintf(stderr, " (context: %s)", e->context);
    if (e->sys != 0) fprintf(stderr, " (sys: %d)", e->sys);
    fprintf(stderr, "\n");
    abort();
}
#endif

/* Unwrap macros - panic on error */
#ifndef NUR_NO_STDIO
/**
 * Get the value from a result or abort with error message.
 * Only use when you are certain the result is Ok.
 */
#define NUR_UNWRAP(r) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur__panic_result(&(r).u.error, __FILE__, (unsigned int)__LINE__), (r).u.value))

/**
 * Get the value from a result or abort with custom message.
 */  
#define NUR_EXPECT(r, msg) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur__panic_msg((msg), &(r).u.error, __FILE__, (unsigned int)__LINE__), (r).u.value))
#endif

/* Fallback macros - always available */
#define NUR_UNWRAP_OR(r, fallback) \
    (NUR_LIKELY((r).ok) ? (r).u.value : (fallback))

#define NUR_OPTION_UNWRAP_OR(o, fallback) \
    (NUR_LIKELY((o).some) ? (o).value : (fallback))

/**
 * Explicitly ignore a result - documents intent and suppresses warnings.
 */
#define NUR_IGNORE(expr) ((void)(expr))

/*
 * =============================================================================
 * SECTION 9: ERROR PROPAGATION MACROS  
 * =============================================================================
 */

/**
 * Evaluate expr -> NurResult_SrcSuffix.
 * On error: return nur_err_DestSuffix(error) from enclosing function.
 * On success: assign .u.value to lhs.
 */
#define NUR_TRY_TO(DestSuffix, SrcSuffix, lhs, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(!_nur_tmp_.ok)) { \
            return nur_err_##DestSuffix(_nur_tmp_.u.error); \
        } \
        (lhs) = _nur_tmp_.u.value; \
    } while (0)

/**
 * Like NUR_TRY_TO but for functions returning NurResult_void.
 */
#define NUR_TRY_VOID(SrcSuffix, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(!_nur_tmp_.ok)) { \
            return nur_err_void(_nur_tmp_.u.error); \
        } \
    } while (0)

/* GCC/Clang statement-expression versions for more ergonomic use */
#ifdef NUR_GNU_EXTENSIONS

/**
 * Statement expression version of NUR_TRY.
 * Evaluates to the unwrapped value on success.
 * Enclosing function must return NurResult_void.
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
 * Propagate into a specific return type.
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

/* Condition checking macros */

/**
 * Return NurResult_void error if condition is false.
 */
#define NUR_CHECK(cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_void(err); \
        } \
    } while (0)

/**
 * Return typed result error if condition is false.
 */
#define NUR_CHECK_AS(DestSuffix, cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_##DestSuffix(err); \
        } \
    } while (0)

/*
 * =============================================================================
 * SECTION 10: OPTIONAL ALLOCATION ADAPTER
 * =============================================================================
 */

#if !defined(NUR_NO_STDLIB) && !defined(NUR_NO_ALLOC)

/**
 * malloc wrapper returning NurResult_ptr.
 * Never returns NULL inside the result - failures become error results.
 * Ownership: caller must free the returned pointer.
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
 * Ownership: on success, caller must free the returned pointer.
 * On failure, original ptr remains valid and unchanged.
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

#endif /* !NUR_NO_STDLIB && !NUR_NO_ALLOC */

/*
 * =============================================================================
 * SECTION 11: UTILITIES
 * =============================================================================
 */

#ifndef NUR_NO_STDIO

/**
 * Print error information to a FILE stream.
 * Format: [CODE_NAME] message (context: ...) (sys: N) at file:line in func()
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
    
    if (err->sys != 0) {
        fprintf(f, " (sys: %d)", err->sys);
    }
    
    if (err->file && err->func) {
        fprintf(f, " at %s:%u in %s()", err->file, err->line, err->func);
    }
    
    fprintf(f, "\n");
}

/**
 * Convenience macro to print error to stderr.
 */
#define nur_error_eprint(err) nur_error_print((err), stderr)

#endif /* NUR_NO_STDIO */

#endif /* NUR_H */