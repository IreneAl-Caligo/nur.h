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
 * - Comprehensive error taxonomy with 50+ error codes
 * - Source location tracking (file, function, line)
 * - Error chaining via NUR_WRAP_ERROR
 * - Optional native stacktraces via NUR_ENABLE_BACKTRACE
 * - C23 typeof type deduction via NUR_TRY_AUTO
 * - errno integration for system errors
 *
 * Copyright (c) 2024-2026
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
#define NUR_VERSION_MINOR 0  
#define NUR_VERSION_PATCH 0
#define NUR_VERSION_STRING "1.0.0"

/*============================================================================
 * STANDARD DETECTION
 *===========================================================================*/
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

/*============================================================================
 * COMPILER DETECTION
 *===========================================================================*/
#if defined(__GNUC__) || defined(__clang__)
    #define NUR_GNU_EXTENSIONS 1
#endif

#if defined(_MSC_VER)
    #define NUR_MSVC 1
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
    #include <stdlib.h>
#endif

/*============================================================================
 * ATTRIBUTE MACROS
 *===========================================================================*/

/* [[nodiscard]] - warns when result is ignored */
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

/*============================================================================
 * ERROR STRUCTURE
 *===========================================================================*/

/**
 * Complete error information with source location tracking.
 * 
 * All fields are always present. Source location is captured automatically
 * by error constructor macros.
 * 
 * Fields:
 *   code:    Machine-readable error code (enum)
 *   sys:     System error code (errno/Win32), 0 if none
 *   message: Human-readable description (static string, never NULL)
 *   context: Optional operation context (static string, may be NULL)
 *   file:    Source file where error was raised
 *   func:    Function where error was raised
 *   line:    Line number where error was raised
 */
typedef struct {
    nur_error_code_t code;     
    int              sys;      
    const char      *message;  
    const char      *context;  
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
 * Wrap an existing error with new context, keeping the original code, sys, and source location.
 * This enables Error Chaining.
 * 
 * Usage: NUR_WRAP_ERROR(NUR_ERROR_OF(res), "failed to parse config")
 */
#define NUR_WRAP_ERROR(err_val, new_ctx) \
    nur_internal_make_error((err_val).code, (err_val).sys, (err_val).message, (new_ctx), (err_val).file, (err_val).func, (err_val).line)

/**
 * Create an error with code and message, capturing source location.
 * 
 * Usage: NUR_ERROR(NUR_E_INVALID_ARGUMENT, "buffer size must be positive")
 */
#define NUR_ERROR(code, msg) \
    nur_internal_make_error((code), 0, (msg), NULL, __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an error with code, message, and context, capturing source location.
 * 
 * Usage: NUR_ERROR_CTX(NUR_E_PARSE, "invalid JSON", "reading config.json")
 */
#define NUR_ERROR_CTX(code, msg, ctx) \
    nur_internal_make_error((code), 0, (msg), (ctx), __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an I/O error from errno, capturing source location.
 * Captures errno immediately to prevent it from being overwritten.
 * 
 * Usage: NUR_ERROR_ERRNO("failed to open file")
 */
#define NUR_ERROR_ERRNO(msg) \
    nur_internal_make_error(NUR_E_IO, errno, (msg), NULL, __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an I/O error from errno with context, capturing source location.
 * 
 * Usage: NUR_ERROR_ERRNO_CTX("failed to open", "config.json")
 */
#define NUR_ERROR_ERRNO_CTX(msg, ctx) \
    nur_internal_make_error(NUR_E_IO, errno, (msg), (ctx), __FILE__, __func__, (unsigned int)__LINE__)

/**
 * Create an error from a Win32 error code, capturing source location.
 * 
 * Usage: NUR_ERROR_FROM_WIN32(GetLastError(), "failed to open handle")
 */
#define NUR_ERROR_FROM_WIN32(err_code, msg) \
    nur_internal_make_error(NUR_E_SYSTEM, (int)(err_code), (msg), NULL, __FILE__, __func__, (unsigned int)__LINE__)

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
 *   NUR_RESULT_TYPE(MyStruct, my_struct)
 * 
 * Note: Do NOT add a trailing semicolon to avoid -Wpedantic warnings.
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

/*============================================================================
 * PREDEFINED RESULT TYPES
 *===========================================================================*/

/* Numeric types */
NUR_RESULT_TYPE(int32_t, i32)
NUR_RESULT_TYPE(int64_t, i64)
NUR_RESULT_TYPE(size_t, usize)
NUR_RESULT_TYPE(float, f32)
NUR_RESULT_TYPE(double, f64)

/* Pointer types */
NUR_RESULT_TYPE(char *, cstr)
NUR_RESULT_TYPE(void *, ptr)

/**
 * Special void result type - represents success/failure without a value.
 * 
 * The error field is always present but only meaningful when ok == false.
 */
typedef struct NurResult_void {
    bool ok;
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
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
    union {
        nur_error_t error;
    } u;
    nur_error_t error;
#endif
} NurResult_void;

/**
 * Create a successful void result.
 */
NUR_NODISCARD NUR_INLINE NurResult_void nur_ok_void(void) {
    NurResult_void r;
    r.ok = true;
    memset(&r.error, 0, sizeof(r.error));
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
 *   NUR_OPTION_TYPE(MyStruct, my_struct)
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
        memset(&o_.value, 0, sizeof(T)); \
        return o_; \
    }

/*============================================================================
 * PREDEFINED OPTION TYPES
 *===========================================================================*/

NUR_OPTION_TYPE(int32_t, i32)
NUR_OPTION_TYPE(int64_t, i64)  
NUR_OPTION_TYPE(size_t, usize)
NUR_OPTION_TYPE(char *, cstr)
NUR_OPTION_TYPE(void *, ptr)

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
 * Maps a Win32 / Winsock error number to a semantic code.
 */
NUR_INLINE nur_error_code_t nur_error_code_from_win32(unsigned long error) {
    switch (error) {
        case 0UL: return NUR_E_NONE;
        case 2UL: case 3UL: case 15UL: case 126UL: case 127UL: case 1168UL: return NUR_E_NOT_FOUND;
        case 5UL: case 19UL: case 1314UL: return NUR_E_PERMISSION;
        case 6UL: case 87UL: case 123UL: case 161UL: case 267UL: return NUR_E_INVALID_ARGUMENT;
        case 8UL: case 14UL: return NUR_E_OUT_OF_MEMORY;
        case 4UL: case 39UL: case 112UL: case 122UL: return NUR_E_RESOURCE_LIMIT;
        case 21UL: case 32UL: case 33UL: case 170UL: case 231UL: case 10035UL: return NUR_E_BUSY;
        case 23UL: return NUR_E_CORRUPTED;
        case 38UL: case 1117UL: return NUR_E_IO;
        case 50UL: case 120UL: return NUR_E_UNSUPPORTED;
        case 80UL: case 183UL: return NUR_E_ALREADY_EXISTS;
        case 145UL: return NUR_E_INVALID_STATE;
        case 109UL: case 232UL: case 10054UL: case 10057UL: return NUR_E_CONNECTION;
        case 121UL: case 258UL: case 1460UL: case 10060UL: return NUR_E_TIMEOUT;
        case 995UL: case 1223UL: return NUR_E_CANCELLED;
        case 1225UL: case 1236UL: case 10051UL: case 10053UL: case 10061UL: case 10065UL: return NUR_E_REFUSED;
        case 10004UL: return NUR_E_CANCELLED;
        default: break;
    }
    return NUR_E_SYSTEM;
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
        nur_internal_sink_putu(sink, (unsigned long long)(0ULL - value));
    } else {
        nur_internal_sink_putu(sink, (unsigned long long)value);
    }
}

/**
 * Renders error into buffer as a single line.
 * Returns required length excluding NUL. If >= capacity, output was truncated.
 */
NUR_INLINE size_t nur_error_format(const nur_error_t *error, char *buffer, size_t capacity) {
    nur_internal_sink_t sink = { buffer, capacity, 0 };
    if (!error) return 0;
    
    nur_internal_sink_puts(&sink, nur_error_code_name(error->code));
    nur_internal_sink_puts(&sink, ": ");
    nur_internal_sink_puts(&sink, error->message ? error->message : "unknown");
    
    if (error->context && error->context[0] != '\0') {
        nur_internal_sink_puts(&sink, " [");
        nur_internal_sink_puts(&sink, error->context);
        nur_internal_sink_putc(&sink, ']');
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

/*============================================================================
 * PANIC HELPERS (require stdio)
 *===========================================================================*/

#ifndef NUR_NO_STDIO

#ifndef NUR_PRINT_BUFFER_SIZE
#define NUR_PRINT_BUFFER_SIZE 1024
#endif

NUR_INLINE void nur_internal_panic_result(const nur_error_t *e, const char *file, unsigned int line) {
    char buffer[NUR_PRINT_BUFFER_SIZE];
    nur_error_format(e, buffer, sizeof(buffer));
    fprintf(stderr, "nur panic at %s:%u: %s\n", file, line, buffer);
    abort();
}

NUR_INLINE void nur_internal_panic_msg(const char *msg, const nur_error_t *e, const char *file, unsigned int line) {
    char buffer[NUR_PRINT_BUFFER_SIZE];
    nur_error_format(e, buffer, sizeof(buffer));
    fprintf(stderr, "nur panic at %s:%u: %s: %s\n", file, line, msg, buffer);
    abort();
}

#if defined(NUR_GNU_EXTENSIONS)

#define NUR_UNWRAP(r) \
    __extension__ ({ \
        __auto_type _nur_unwrap_r_ = (r); \
        if (NUR_UNLIKELY(!_nur_unwrap_r_.ok)) { \
            nur_internal_panic_result(&_nur_unwrap_r_.u.error, __FILE__, (unsigned int)__LINE__); \
        } \
        _nur_unwrap_r_.u.value; \
    })

#define NUR_EXPECT(r, msg) \
    __extension__ ({ \
        __auto_type _nur_expect_r_ = (r); \
        if (NUR_UNLIKELY(!_nur_expect_r_.ok)) { \
            nur_internal_panic_msg((msg), &_nur_expect_r_.u.error, __FILE__, (unsigned int)__LINE__); \
        } \
        _nur_expect_r_.u.value; \
    })

#else

#define NUR_UNWRAP(r) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_result(&(r).u.error, __FILE__, (unsigned int)__LINE__), (r).u.value))

#define NUR_EXPECT(r, msg) \
    (NUR_LIKELY((r).ok) \
        ? (r).u.value \
        : (nur_internal_panic_msg((msg), &(r).u.error, __FILE__, (unsigned int)__LINE__), (r).u.value))

#endif

#endif /* NUR_NO_STDIO */

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
 * Usage: int port = NUR_UNWRAP_OR(parse_port(str), 8080);
 */
#define NUR_UNWRAP_OR(r, fallback) \
    (NUR_LIKELY((r).ok) ? (r).u.value : (fallback))

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

#if defined(NUR_C23) || defined(NUR_GNU_EXTENSIONS)
/**
 * Auto-typing NUR_TRY_TO for C23 or GNU extensions.
 * No need to specify SrcSuffix.
 */
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

#if defined(NUR_C23) || defined(NUR_GNU_EXTENSIONS)
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
#define NUR_TRY_VOID_FROM_VOID(expr) \
    do { \
        NurResult_void _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)

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

#endif /* NUR_NO_STDIO */

#ifdef __cplusplus
}
#endif

#endif /* NUR_H */
