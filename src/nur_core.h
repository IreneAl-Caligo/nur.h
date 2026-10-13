/*
 * nur_core.h - Minimal, Dependency-Free Tagged Result & Option Primitives for ISO C
 *
 * A single-file, zero-allocation, ultra-lean C header providing core Result<T,E>
 * and Option<T> type primitives for ISO C (C89, C99, C11, C17, C23) and C++11+.
 *
 * Standard Dependencies:
 * - Strictly limited to <stdbool.h>, <stddef.h>, and <stdint.h>.
 * - Zero external runtime dependencies; fully freestanding and embedded compatible.
 *
 * Copyright (c) 2024-2026 Irene Al-Caligo
 * Author: Irene Al-Caligo
 * SPDX-License-Identifier: MIT
 */

#ifndef NUR_CORE_H_INCLUDED
#define NUR_CORE_H_INCLUDED

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
    #define NUR_TYPEOF(expr) __typeof__(expr)
    #define NUR_AUTO __auto_type
#endif

/*============================================================================
 * STANDARD TYPES & COMPATIBILITY
 *===========================================================================*/
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

/*============================================================================
 * ATTRIBUTE MACROS
 *===========================================================================*/
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

#ifdef NUR_GNU_EXTENSIONS
    #define NUR_UNUSED __attribute__((unused))
#else
    #define NUR_UNUSED
#endif

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
    #define NUR_INLINE   static __forceinline
    #define NUR_NOINLINE __declspec(noinline)
#else
    #define NUR_INLINE   static inline
    #define NUR_NOINLINE
#endif

#ifdef NUR_C11
    #define NUR_NORETURN _Noreturn
#elif defined(NUR_GNU_EXTENSIONS)
    #define NUR_NORETURN __attribute__((noreturn))
#elif defined(NUR_MSVC)
    #define NUR_NORETURN __declspec(noreturn)
#else
    #define NUR_NORETURN
#endif

#ifdef NUR_GNU_EXTENSIONS
    #define NUR_COLD __attribute__((cold))
#else
    #define NUR_COLD
#endif

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

#ifdef NUR_GNU_EXTENSIONS
    #define NUR_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define NUR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define NUR_LIKELY(x)   (x)
    #define NUR_UNLIKELY(x) (x)
#endif

/*============================================================================
 * ERROR TAXONOMY
 *===========================================================================*/
#ifdef NUR_GNU_EXTENSIONS
__extension__
#endif
typedef enum nur_error_code_t {
    NUR_E_NONE = 0,
    NUR_E_INVALID_ARGUMENT = 1,
    NUR_E_OUT_OF_RANGE = 2,
    NUR_E_NULL_POINTER = 3,
    NUR_E_INVALID_STATE = 4,
    NUR_E_OVERFLOW = 5,
    NUR_E_UNDERFLOW = 6,
    NUR_E_PARSE = 10,
    NUR_E_FORMAT = 11,
    NUR_E_ENCODING = 12,
    NUR_E_CORRUPTED = 13,
    NUR_E_TRUNCATED = 14,
    NUR_E_CHECKSUM = 15,
    NUR_E_OUT_OF_MEMORY = 20,
    NUR_E_RESOURCE_LIMIT = 21,
    NUR_E_BUSY = 22,
    NUR_E_QUOTA = 23,
    NUR_E_DEADLOCK = 24,
    NUR_E_NOT_FOUND = 30,
    NUR_E_PERMISSION = 31,
    NUR_E_ALREADY_EXISTS = 32,
    NUR_E_IO = 33,
    NUR_E_EOF = 34,
    NUR_E_CLOSED = 35,
    NUR_E_WOULD_BLOCK = 36,
    NUR_E_TIMEOUT = 40,
    NUR_E_CANCELLED = 41,
    NUR_E_CONNECTION = 42,
    NUR_E_PROTOCOL = 43,
    NUR_E_AGAIN = 44,
    NUR_E_REFUSED = 45,
    NUR_E_UNSUPPORTED = 50,
    NUR_E_INTERNAL = 51,
    NUR_E_BUG = 52,
    NUR_E_VERSION = 53,
    NUR_E_UNINITIALIZED = 54,
    NUR_E_SYSTEM = 55,
    NUR_E_USER_BASE = 1000
} nur_error_code_t;

#define NUR_E_PERMISSION_DENIED NUR_E_PERMISSION

#ifndef NUR_MAX_CONTEXT_DEPTH
#define NUR_MAX_CONTEXT_DEPTH 4
#endif

#ifndef NUR_ERROR_INLINE_MSG_SIZE
#define NUR_ERROR_INLINE_MSG_SIZE 64
#endif

/* Complete error structure with inline string buffer option */
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

/* Core internal error constructor */
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
    err.trace_count = 0;
#endif
    return err;
#endif
}

/* Safe dynamic message copy into inline buffer to eliminate dangling pointers */
NUR_INLINE nur_error_t nur_error_copy_msg(nur_error_t e, const char *src_str) {
#ifndef NUR_COMPACT_ERRORS
    size_t i = 0;
    if (src_str != (const char *)0) {
        while (src_str[i] != '\0' && i < (NUR_ERROR_INLINE_MSG_SIZE - 1)) {
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

#define NUR_ERROR(code, msg) \
    nur_internal_make_error((code), 0, (msg), (const char *)0, NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

#define NUR_ERROR_CTX(code, msg, ctx) \
    nur_internal_make_error((code), 0, (msg), (ctx), NUR_LOC_FILE, NUR_LOC_FUNC, NUR_LOC_LINE)

#define NUR_ERROR_LIT(code, literal_msg) \
    NUR_ERROR((code), ("" literal_msg))

#define NUR_ERROR_LIT_CTX(code, literal_msg, literal_ctx) \
    NUR_ERROR_CTX((code), ("" literal_msg), ("" literal_ctx))

#define NUR_ERROR_COPY(code, dynamic_str) \
    nur_error_copy_msg(NUR_ERROR((code), (const char *)0), (dynamic_str))

/*============================================================================
 * RESULT TYPE GENERATOR
 *===========================================================================*/
#define NUR_RESULT_TYPE_E(T, E, suffix) \
    typedef struct NUR_NODISCARD_TYPE NurResult_##suffix { \
        bool ok; \
        union { \
            T value; \
            E error; \
        } u; \
    } NurResult_##suffix; \
    NUR_NODISCARD NUR_INLINE NurResult_##suffix nur_ok_##suffix(T val_) { \
        NurResult_##suffix r_; r_.ok = true; r_.u.value = val_; return r_; \
    } \
    NUR_NODISCARD NUR_INLINE NurResult_##suffix nur_err_##suffix(E err_) { \
        NurResult_##suffix r_; r_.ok = false; r_.u.error = err_; return r_; \
    } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_ok_##suffix(NurResult_##suffix r_) { return r_.ok; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_err_##suffix(NurResult_##suffix r_) { return !r_.ok; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE T nur_value_##suffix(NurResult_##suffix r_) { return r_.u.value; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE E nur_error_##suffix(NurResult_##suffix r_) { return r_.u.error; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE T nur_unwrap_or_##suffix(NurResult_##suffix r_, T fallback_) { \
        return r_.ok ? r_.u.value : fallback_; \
    } \
    struct nur_internal_dummy_decl_e_##suffix

#define NUR_RESULT_TYPE(T, suffix) \
    typedef struct NUR_NODISCARD_TYPE NurResult_##suffix { \
        bool ok; \
        union { \
            T value; \
            nur_error_t error; \
        } u; \
    } NurResult_##suffix; \
    NUR_NODISCARD NUR_INLINE NurResult_##suffix nur_ok_##suffix(T val_) { \
        NurResult_##suffix r_; r_.ok = true; r_.u.value = val_; return r_; \
    } \
    NUR_NODISCARD NUR_INLINE NurResult_##suffix nur_err_##suffix(nur_error_t err_) { \
        NurResult_##suffix r_; r_.ok = false; r_.u.error = err_; return r_; \
    } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_ok_##suffix(NurResult_##suffix r_) { return r_.ok; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_err_##suffix(NurResult_##suffix r_) { return !r_.ok; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE T nur_value_##suffix(NurResult_##suffix r_) { return r_.u.value; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE nur_error_t nur_error_##suffix(NurResult_##suffix r_) { return r_.u.error; } \
    NUR_NODISCARD NUR_UNUSED NUR_INLINE T nur_unwrap_or_##suffix(NurResult_##suffix r_, T fallback_) { \
        return r_.ok ? r_.u.value : fallback_; \
    } \
    struct nur_internal_dummy_decl_##suffix

/* Predefined canonical Result types */
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
NUR_RESULT_TYPE(char *, cstr);
NUR_RESULT_TYPE(const char *, cstr_const);
NUR_RESULT_TYPE(void *, ptr);

/* Void result */
typedef struct NUR_NODISCARD_TYPE NurResult_void {
    bool ok;
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11) || defined(NUR_C23) || (defined(_MSC_VER) && !defined(__GNUC__) && !defined(__clang__))
    union {
        nur_error_t error;
        struct { nur_error_t error; } u;
    };
#elif defined(NUR_GNU_EXTENSIONS)
    __extension__ union {
        nur_error_t error;
        struct { nur_error_t error; } u;
    };
#else
    struct { nur_error_t error; } u;
#endif
} NurResult_void;

#define NUR_VOID_ERROR_OF(r) ((r).u.error)
NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_ok_void(NurResult_void r_) { return r_.ok; }
NUR_NODISCARD NUR_UNUSED NUR_INLINE bool nur_is_err_void(NurResult_void r_) { return (bool)!r_.ok; }
NUR_NODISCARD NUR_UNUSED NUR_INLINE nur_error_t nur_error_void_val(NurResult_void r_) { return r_.u.error; }

NUR_NODISCARD NUR_INLINE NurResult_void nur_ok_void(void) {
    NurResult_void r;
    r.ok = true;
    r.u.error.code = NUR_E_NONE;
    r.u.error.line = 0U;
    r.u.error.message = (const char *)0;
#ifndef NUR_COMPACT_ERRORS
    r.u.error.sys = 0;
    r.u.error.context = (const char *)0;
    r.u.error.cause = (const char *)0;
    r.u.error.file = (const char *)0;
    r.u.error.func = (const char *)0;
    r.u.error.context_count = 0;
    r.u.error.inline_msg[0] = '\0';
#endif
    return r;
}

NUR_NODISCARD NUR_INLINE NurResult_void nur_err_void(nur_error_t err_) {
    NurResult_void r; r.ok = false; r.u.error = err_; return r;
}

/*============================================================================
 * OPTION TYPE GENERATOR
 *===========================================================================*/
#define NUR_OPTION_TYPE(T, suffix) \
    typedef struct NurOption_##suffix { \
        bool some; \
        T    value; \
    } NurOption_##suffix; \
    NUR_INLINE NurOption_##suffix nur_some_##suffix(T val_) { \
        NurOption_##suffix o_; o_.some = true; o_.value = val_; return o_; \
    } \
    NUR_INLINE NurOption_##suffix nur_none_##suffix(void) { \
        NurOption_##suffix o_; \
        unsigned char *p_ = (unsigned char *)(void *)&o_.value; \
        size_t i_; o_.some = false; \
        for (i_ = 0; i_ < sizeof(T); ++i_) p_[i_] = 0; \
        return o_; \
    } \
    struct nur_internal_dummy_opt_decl_##suffix

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
 * UNIVERSAL CONSTRUCTOR & STATUS CHECKING
 *===========================================================================*/
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
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
    const void*:        nur_internal_gen_const_ptr \
)(val)

#define NUR_INTERNAL_GET_MACRO_3(_1, _2, _3, ...) _3
#define NUR_INTERNAL_OK_1(val)         NUR_H(val)
#define NUR_INTERNAL_OK_2(suffix, val) nur_ok_##suffix(val)
#define NUR_OK(...) NUR_INTERNAL_GET_MACRO_3(__VA_ARGS__, NUR_INTERNAL_OK_2, NUR_INTERNAL_OK_1, nur_internal_unused)(__VA_ARGS__)
#else
#define NUR_OK(suffix, val)   nur_ok_##suffix(val)
#define NUR_H(val)            (val)
#endif

#define NUR_ERR(suffix, err)  nur_err_##suffix(err)
#define NUR_OK_VOID()         nur_ok_void()
#define NUR_ERR_VOID(err)     nur_err_void(err)
#define NUR_SOME(suffix, val) nur_some_##suffix(val)
#define NUR_NONE(suffix)      nur_none_##suffix()

#define NUR_IS_OK(r)          ((r).ok)
#define NUR_IS_ERR(r)         (!(r).ok)
#define NUR_IS_SOME(o)        ((o).some)
#define NUR_IS_NONE(o)        (!(o).some)
#define NUR_VALUE(r)          ((r).u.value)
#define NUR_ERROR_OF(r)       ((r).u.error)
#define NUR_OPTION_VALUE(o)   ((o).value)

#define NUR_UNWRAP_OR(r, fallback) \
    ((r).ok ? (r).u.value : (fallback))
#define NUR_OPTION_UNWRAP_OR(o, fallback) \
    ((o).some ? (o).value : (fallback))
#define NUR_IGNORE(expr)      ((void)(expr))

/*============================================================================
 * PURE-FUNCTION MACRO-FREE API
 *===========================================================================*/
NUR_INLINE bool nur_result_is_ok(const void *result_ptr) {
    return (bool)(result_ptr != (const void *)0 && *(const bool *)result_ptr);
}

NUR_INLINE bool nur_result_is_err(const void *result_ptr) {
    return (bool)(result_ptr == (const void *)0 || !(*(const bool *)result_ptr));
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
 * CONTROL FLOW & PROPAGATION
 *===========================================================================*/
#define NUR_TRY_TO(DestSuffix, SrcSuffix, lhs, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_##DestSuffix(NUR_ERROR_OF(_nur_tmp_)); \
        } \
        (lhs) = NUR_VALUE(_nur_tmp_); \
    } while (0)

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
#endif

#define NUR_TRY_VOID(SrcSuffix, expr) \
    do { \
        NurResult_##SrcSuffix _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)

#define NUR_TRY_VOID_FROM_VOID(expr) \
    do { \
        NurResult_void _nur_tmp_ = (expr); \
        if (NUR_UNLIKELY(NUR_IS_ERR(_nur_tmp_))) { \
            return nur_err_void(NUR_ERROR_OF(_nur_tmp_)); \
        } \
    } while (0)

#if (defined(NUR_GNU_EXTENSIONS) || defined(__cplusplus)) && !defined(NUR_C89)
#define NUR_TRY_GOTO(lhs, expr, errvar, label) \
    do { \
        __extension__ NUR_AUTO _ntg_ = (expr); \
        if (NUR_UNLIKELY(!_ntg_.ok)) { (errvar) = _ntg_.u.error; goto label; } \
        (lhs) = _ntg_.u.value; \
    } while (0)
#else
#define NUR_TRY_GOTO(lhs, expr_lval, errvar, label) \
    do { \
        if (NUR_UNLIKELY(!(expr_lval).ok)) { \
            (errvar) = (expr_lval).u.error; \
            goto label; \
        } \
        (lhs) = (expr_lval).u.value; \
    } while (0)
#endif

#define NUR_CHECK(cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_void(err); \
        } \
    } while (0)

#define NUR_CHECK_AS(DestSuffix, cond, err) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_##DestSuffix(err); \
        } \
    } while (0)

#define NUR_CHECK_MSG(cond, code, msg) \
    do { \
        if (NUR_UNLIKELY(!(cond))) { \
            return nur_err_void(NUR_ERROR((code), (msg))); \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* NUR_CORE_H_INCLUDED */
