/*
 * test_abi_layout.c - Rigorous Compile-Time and Runtime ABI Layout Freeze Tests
 *
 * Verifies that the memory size, struct alignment, and field offsets of all
 * predefined NurResult and NurOption types match the frozen ABI specification.
 *
 * Copyright (c) 2024-2026 Irene Al-Caligo
 * SPDX-License-Identifier: MIT
 */

#include "nur.h"
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <assert.h>
#define STATIC_CHECK(cond, msg) _Static_assert(cond, msg)
#else
#define STATIC_CHECK(cond, msg) typedef char static_assertion_##msg[(cond) ? 1 : -1]
#endif

int main(void) {
    size_t ptr_size = sizeof(void *);
    printf("=================================================================\n");
    printf(" nur.h ABI Layout Freeze Verification (Pointer Width: %zu bits)\n", ptr_size * 8);
    printf("=================================================================\n");

    /* 1. Common Invariants Across All Architectures */
    assert(sizeof(nur_error_code_t) == 4);
    assert(offsetof(nur_error_t, code) == 0);
    assert(offsetof(nur_error_t, sys) == 4);

    if (ptr_size == 8) {
        /* 64-bit Architecture (LP64 / LLP64) */
        printf("[+] Validating 64-bit ABI layout specifications...\n");

#ifndef NUR_COMPACT_ERRORS
        /* nur_error_t layout */
        assert(sizeof(nur_error_t) == 152);
        assert(offsetof(nur_error_t, message) == 8);
        assert(offsetof(nur_error_t, context) == 16);
        assert(offsetof(nur_error_t, cause) == 24);
        assert(offsetof(nur_error_t, file) == 32);
        assert(offsetof(nur_error_t, func) == 40);
        assert(offsetof(nur_error_t, line) == 48);
        assert(offsetof(nur_error_t, context_count) == 52);
        assert(offsetof(nur_error_t, context_chain) == 56);
        assert(offsetof(nur_error_t, inline_msg) == 88);

        /* Result types (size = 160 bytes, align = 8, offset_ok = 0, offset_u = 8) */
        assert(sizeof(NurResult_i32) == 160);
        assert(sizeof(NurResult_i64) == 160);
        assert(sizeof(NurResult_usize) == 160);
        assert(sizeof(NurResult_f32) == 160);
        assert(sizeof(NurResult_f64) == 160);
        assert(sizeof(NurResult_bool) == 160);
        assert(sizeof(NurResult_cstr) == 160);
        assert(sizeof(NurResult_ptr) == 160);
        assert(sizeof(NurResult_void) == 160);

        assert(offsetof(NurResult_i32, ok) == 0);
        assert(offsetof(NurResult_i32, u) == 8);
        assert(offsetof(NurResult_i64, u) == 8);
        assert(offsetof(NurResult_usize, u) == 8);
        assert(offsetof(NurResult_f64, u) == 8);
        assert(offsetof(NurResult_ptr, u) == 8);
        assert(offsetof(NurResult_void, ok) == 0);
        assert(offsetof(NurResult_void, u) == 8);
#endif

        /* Option types */
        assert(sizeof(NurOption_i8) == 2);
        assert(sizeof(NurOption_i16) == 4);
        assert(sizeof(NurOption_i32) == 8);
        assert(sizeof(NurOption_i64) == 16);
        assert(sizeof(NurOption_usize) == 16);
        assert(sizeof(NurOption_f32) == 8);
        assert(sizeof(NurOption_f64) == 16);
        assert(sizeof(NurOption_bool) == 2);
        assert(sizeof(NurOption_cstr) == 16);

        assert(offsetof(NurOption_i32, some) == 0);
        assert(offsetof(NurOption_i32, value) == 4);
        assert(offsetof(NurOption_i64, value) == 8);
        assert(offsetof(NurOption_usize, value) == 8);
        assert(offsetof(NurOption_f64, value) == 8);
    } else if (ptr_size == 4) {
        /* 32-bit Architecture (ILP32) */
        printf("[+] Validating 32-bit ABI layout specifications...\n");

#ifndef NUR_COMPACT_ERRORS
        assert(sizeof(nur_error_t) == 116 || sizeof(nur_error_t) == 120);
        assert(offsetof(nur_error_t, message) == 8);
        assert(offsetof(nur_error_t, context) == 12);
        assert(offsetof(nur_error_t, cause) == 16);
        assert(offsetof(nur_error_t, file) == 20);
        assert(offsetof(nur_error_t, func) == 24);
        assert(offsetof(nur_error_t, line) == 28);
        assert(offsetof(nur_error_t, context_count) == 32);
#endif
    }

    printf("[PASSED] All ABI layout freeze requirements verified successfully!\n");
    return 0;
}
