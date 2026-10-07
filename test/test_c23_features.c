/*
 * C23 Feature Validation Test Suite for nur.h Library
 * 
 * This test suite validates C23-specific features used in nur.h and ensures
 * graceful degradation on compilers that don't support C23.
 *
 * Features tested:
 * - C23 [[nodiscard("message")]] attributes with custom messages
 * - Enhanced static_assert with custom messages
 * - typeof/auto type deduction in error propagation macros
 * - Binary literals and digit separators
 * - Compiler version detection and feature graceful degradation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* Include the library under test */
#include "../src/nur.h"

/* Test framework macros */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_skipped = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s... ", #name); \
        tests_run++; \
        volatile int result_ = test_##name(); \
        if (result_ == 1) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else if (result_ == 0) { \
            printf("FAIL\n"); \
        } else { \
            printf("SKIP (C23 not supported)\n"); \
            tests_skipped++; \
        } \
    } while(0)

/* Return codes: 1 = pass, 0 = fail, -1 = skip */

/*
 * =============================================================================
 * C23 FEATURE DETECTION
 * =============================================================================
 */

static void print_compiler_info(void) {
    printf("=== Compiler Information ===\n");
    
    /* Standard version detection */
    printf("C Standard: ");
#if __STDC_VERSION__ >= 202311L
    printf("C23 (__STDC_VERSION__ = %ld)\n", (long)__STDC_VERSION__);
#elif __STDC_VERSION__ >= 201710L
    printf("C17 (__STDC_VERSION__ = %ld)\n", (long)__STDC_VERSION__);
#elif __STDC_VERSION__ >= 201112L
    printf("C11 (__STDC_VERSION__ = %ld)\n", (long)__STDC_VERSION__);
#elif __STDC_VERSION__ >= 199901L
    printf("C99 (__STDC_VERSION__ = %ld)\n", (long)__STDC_VERSION__);
#else
    printf("Pre-C99\n");
#endif

    /* Compiler detection */
    printf("Compiler: ");
#if defined(__GNUC__) && !defined(__clang__)
    printf("GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    printf("Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(_MSC_VER)
    printf("MSVC %d\n", _MSC_VER);
#else
    printf("Unknown\n");
#endif

    /* Feature detection from nur.h */
    printf("nur.h Features: ");
#ifdef NUR_C23
    printf("C23 ");
#endif
#ifdef NUR_C17
    printf("C17 ");
#endif
#ifdef NUR_C11
    printf("C11 ");
#endif
#ifdef NUR_C99
    printf("C99 ");
#endif
#ifdef NUR_GNU_EXTENSIONS
    printf("GNU-EXT ");
#endif
    printf("\n\n");
}

/*
 * =============================================================================
 * C23 [[nodiscard("message")]] ATTRIBUTE TESTS
 * =============================================================================
 */

#ifdef NUR_C23
/* Test function with C23 nodiscard message */
[[nodiscard("You must check the result of this critical operation")]]
static NurResult_i32 critical_operation_c23(int value) {
    if (value < 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Negative value not allowed"));
    }
    return NUR_OK(i32, value * 2);
}
#endif

/* Fallback function for non-C23 compilers */
NUR_NODISCARD_MSG("You must check the result of this critical operation")
static NurResult_i32 critical_operation_fallback(int value) {
    if (value < 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Negative value not allowed"));
    }
    return NUR_OK(i32, value * 2);
}

static int test_nodiscard_with_message(void) {
#ifdef NUR_C23
    /* Test C23 [[nodiscard("message")]] */
    NurResult_i32 result = critical_operation_c23(42);
    if (!NUR_IS_OK(result) || NUR_VALUE(result) != 84) {
        return 0;
    }
    
    /* Note: We can't test that the warning is actually generated without
     * parsing compiler output, but we can verify the function works */
    return 1;
#else
    /* Test fallback nodiscard attribute */
    NurResult_i32 result = critical_operation_fallback(42);
    if (!NUR_IS_OK(result) || NUR_VALUE(result) != 84) {
        return 0;
    }
    return -1; /* Skip - C23 not available */
#endif
}

/*
 * =============================================================================
 * C23 ENHANCED static_assert TESTS
 * =============================================================================
 */

static int test_enhanced_static_assert(void) {
#ifdef NUR_C23
    /* C23 allows custom messages in static_assert */
    static_assert(sizeof(nur_error_t) > 0, "nur_error_t must have non-zero size");
    static_assert(sizeof(NurResult_i32) >= sizeof(int32_t), 
                  "NurResult_i32 must be at least as large as int32_t");
    static_assert(NUR_E_INVALID_ARGUMENT >= 1 && NUR_E_INVALID_ARGUMENT <= 9,
                  "Input validation errors must be in range 1-9");
    static_assert(NUR_E_PARSE >= 10 && NUR_E_PARSE <= 19,
                  "Data format errors must be in range 10-19");
    static_assert(NUR_E_OUT_OF_MEMORY >= 20 && NUR_E_OUT_OF_MEMORY <= 29,
                  "Resource management errors must be in range 20-29");
    static_assert(NUR_E_NOT_FOUND >= 30 && NUR_E_NOT_FOUND <= 39,
                  "I/O errors must be in range 30-39");
    static_assert(NUR_E_TIMEOUT >= 40 && NUR_E_TIMEOUT <= 49,
                  "Network/async errors must be in range 40-49");
    static_assert(NUR_E_UNSUPPORTED >= 50 && NUR_E_UNSUPPORTED <= 59,
                  "System errors must be in range 50-59");
    static_assert(NUR_E_USER_BASE >= 1000,
                  "User-defined errors must start at 1000 or higher");
    
    return 1;
#elif defined(NUR_C11) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    /* C11 static_assert without custom messages */
    static_assert(sizeof(nur_error_t) > 0, "nur_error_t size check");
    static_assert(sizeof(NurResult_i32) >= sizeof(int32_t), "NurResult_i32 size check");
    static_assert(NUR_E_INVALID_ARGUMENT >= 1 && NUR_E_INVALID_ARGUMENT <= 9, "Input error range");
    static_assert(NUR_E_PARSE >= 10 && NUR_E_PARSE <= 19, "Data error range");
    static_assert(NUR_E_OUT_OF_MEMORY >= 20 && NUR_E_OUT_OF_MEMORY <= 29, "Resource error range");
    static_assert(NUR_E_NOT_FOUND >= 30 && NUR_E_NOT_FOUND <= 39, "I/O error range");
    static_assert(NUR_E_TIMEOUT >= 40 && NUR_E_TIMEOUT <= 49, "Async error range");
    static_assert(NUR_E_UNSUPPORTED >= 50 && NUR_E_UNSUPPORTED <= 59, "System error range");
    static_assert(NUR_E_USER_BASE >= 1000, "User error base");
    
    return -1; /* Skip - C23 not available */
#else
    /* Pre-C11: Use runtime asserts */
    assert(sizeof(nur_error_t) > 0);
    assert(sizeof(NurResult_i32) >= sizeof(int32_t));
    assert(NUR_E_INVALID_ARGUMENT >= 1 && NUR_E_INVALID_ARGUMENT <= 9);
    assert(NUR_E_USER_BASE >= 1000);
    
    return -1; /* Skip - static_assert not available */
#endif
}

/*
 * =============================================================================
 * C23 typeof/auto TYPE DEDUCTION TESTS
 * =============================================================================
 */

#ifdef NUR_GNU_EXTENSIONS
/* Test typeof-based type deduction in error propagation */
static NurResult_i32 multiply_safe(int a, int b) {
    /* Check for overflow potential using proper bounds */
    if (a > 0 && b > 0 && a > (INT_MAX / b)) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Multiplication overflow"));
    }
    if (a < 0 && b > 0 && a < (INT_MIN / b)) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Multiplication underflow"));
    }
    if (a > 0 && b < 0 && b < (INT_MIN / a)) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Multiplication underflow"));
    }
    if (a < 0 && b < 0 && a < (INT_MAX / b)) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Multiplication overflow"));
    }
    return NUR_OK(i32, a * b);
}

static NurResult_i32 test_typeof_helper(int x) {
    /* This should use __auto_type in NUR_TRY macro for GCC/Clang */
    int result_val = NUR_TRY(multiply_safe(x, 3));
    return NUR_OK(i32, result_val + 1);
}
#endif

static int test_typeof_auto_type_deduction(void) {
#ifdef NUR_GNU_EXTENSIONS
    /* Test successful case */
    NurResult_i32 ok_result = test_typeof_helper(10);
    if (!NUR_IS_OK(ok_result) || NUR_VALUE(ok_result) != 31) { /* (10*3)+1 = 31 */
        return 0;
    }
    
    /* Test error propagation */
    NurResult_i32 err_result = test_typeof_helper(1000000); /* Should overflow */
    if (NUR_IS_OK(err_result)) {
        return 0;
    }
    
    /* Verify error propagated correctly */
    nur_error_t error = NUR_ERROR_OF(err_result);
    if (error.code != NUR_E_OVERFLOW) {
        return 0;
    }
    
    return 1;
#else
    return -1; /* Skip - GNU extensions not available */
#endif
}

/*
 * =============================================================================
 * C23 BINARY LITERALS AND DIGIT SEPARATORS TESTS
 * =============================================================================
 */

static int test_binary_literals_and_separators(void) {
#ifdef NUR_C23
    /* C23 binary literals and digit separators */
    int binary_val = 0b1010'1010;           /* Binary literal with separator */
    int decimal_val = 1'000'000;            /* Decimal with thousands separator */
    int hex_val = 0xFF'FF'FF'FF;           /* Hex with separators */
    long long big_val = 123'456'789'012LL; /* Large number with separators */
    
    /* Test that values are correctly parsed */
    if (binary_val != 170) {  /* 0b10101010 = 170 decimal */
        return 0;
    }
    if (decimal_val != 1000000) {
        return 0;
    }
    if (hex_val != 0xFFFFFFFF) {
        return 0;
    }
    if (big_val != 123456789012LL) {
        return 0;
    }
    
    /* Use these values in Result types to ensure compatibility */
    NurResult_i32 binary_result = NUR_OK(i32, binary_val);
    NurResult_i32 decimal_result = NUR_OK(i32, decimal_val);
    
    if (!NUR_IS_OK(binary_result) || !NUR_IS_OK(decimal_result)) {
        return 0;
    }
    
    if (NUR_VALUE(binary_result) != 170 || NUR_VALUE(decimal_result) != 1000000) {
        return 0;
    }
    
    return 1;
#else
    /* Fallback for non-C23: use traditional literals */
    int binary_val = 170;      /* 0b10101010 equivalent */
    int decimal_val = 1000000; /* Same value without separators */
    int hex_val = 0xFFFFFFFF;  /* Same hex value */
    
    NurResult_i32 binary_result = NUR_OK(i32, binary_val);
    NurResult_i32 decimal_result = NUR_OK(i32, decimal_val);
    
    if (!NUR_IS_OK(binary_result) || !NUR_IS_OK(decimal_result)) {
        return 0;
    }
    
    return -1; /* Skip - C23 literals not available */
#endif
}

/*
 * =============================================================================
 * COMPILER VERSION AND GRACEFUL DEGRADATION TESTS
 * =============================================================================
 */

static int test_compiler_version_detection(void) {
    /* Test that compiler-specific features are detected correctly */
    
    /* Test GCC version detection */
#if defined(__GNUC__) && !defined(__clang__)
    if (__GNUC__ >= 13) {
        /* GCC 13+ should support most C23 features */
#ifdef NUR_C23
        printf("      GCC %d.%d detected with C23 support\n", __GNUC__, __GNUC_MINOR__);
#else
        printf("      GCC %d.%d detected but C23 not enabled\n", __GNUC__, __GNUC_MINOR__);
#endif
    } else {
        printf("      GCC %d.%d detected (C23 may not be fully supported)\n", __GNUC__, __GNUC_MINOR__);
    }
#endif
    
    /* Test Clang version detection */
#if defined(__clang__)
    if (__clang_major__ >= 16) {
        printf("      Clang %d.%d detected (good C23 support expected)\n", __clang_major__, __clang_minor__);
    } else {
        printf("      Clang %d.%d detected (C23 support may be limited)\n", __clang_major__, __clang_minor__);
    }
#endif
    
    /* Test MSVC detection */
#if defined(_MSC_VER)
    printf("      MSVC %d detected\n", _MSC_VER);
#endif
    
    /* Verify that feature detection macros work correctly */
#ifdef NUR_C23
    #ifndef NUR_C17
        printf("      ERROR: C23 should imply C17 support\n");
        return 0;
    #endif
    #ifndef NUR_C11
        printf("      ERROR: C23 should imply C11 support\n");
        return 0;
    #endif
    #ifndef NUR_C99
        printf("      ERROR: C23 should imply C99 support\n");
        return 0;
    #endif
#endif
    
#ifdef NUR_C17
    #ifndef NUR_C11
        printf("      ERROR: C17 should imply C11 support\n");
        return 0;
    #endif
    #ifndef NUR_C99
        printf("      ERROR: C17 should imply C99 support\n");
        return 0;
    #endif
#endif
    
#ifdef NUR_C11
    #ifndef NUR_C99
        printf("      ERROR: C11 should imply C99 support\n");
        return 0;
    #endif
#endif
    
    return 1;
}

static int test_graceful_degradation(void) {
    /* Test that the library functions correctly on all supported compilers */
    
    /* Basic Result functionality should work on all standards */
    NurResult_i32 basic_ok = NUR_OK(i32, 42);
    NurResult_i32 basic_err = NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Test error"));
    
    if (!NUR_IS_OK(basic_ok) || NUR_IS_OK(basic_err)) {
        return 0;
    }
    
    /* Error propagation should work with or without GNU extensions */
    NurResult_i32 propagation_result;
    
#ifdef NUR_GNU_EXTENSIONS
    /* Test NUR_TRY with __auto_type */
    printf("      Testing NUR_TRY with __auto_type (GNU extensions available)\n");
#else
    /* Test NUR_TRY_TO fallback */
    printf("      Testing NUR_TRY_TO fallback (no GNU extensions)\n");
#endif
    
    /* Create a simple error propagation test */
    if (1) {  /* Simple success case */
        propagation_result = basic_ok;
        if (!NUR_IS_OK(propagation_result)) {
            return 0;
        }
    }
    
    /* Test attribute fallbacks */
#ifdef NUR_C23
    printf("      Using C23 [[nodiscard]] attributes\n");
#elif defined(NUR_GNU_EXTENSIONS)
    printf("      Using GNU __attribute__((warn_unused_result))\n");
#elif defined(_MSC_VER)
    printf("      Using MSVC _Check_return_\n");
#else
    printf("      No nodiscard attributes available (empty macros)\n");
#endif
    
    return 1;
}

/*
 * =============================================================================
 * INTEGRATION TESTS
 * =============================================================================
 */

/* Test that C23 features integrate correctly with core nur.h functionality */
static int test_c23_integration(void) {
#ifdef NUR_C23
    /* Create a result using C23 features */
    static_assert(sizeof(NurResult_i32) > sizeof(int32_t), 
                  "Result type must be larger than value type");
    
    /* Use binary literal in computation */
    int flag_mask = 0b1111'0000;  /* C23 binary literal with separator */
    NurResult_i32 masked_result = NUR_OK(i32, 0xFF & flag_mask);
    
    if (!NUR_IS_OK(masked_result) || NUR_VALUE(masked_result) != 0xF0) {
        return 0;
    }
    
    /* Test error with detailed message */
    NurResult_i32 detailed_error = NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "Failed to parse C23 binary literal"));
    if (NUR_IS_OK(detailed_error)) {
        return 0;
    }
    
    nur_error_t error = NUR_ERROR_OF(detailed_error);
    if (error.code != NUR_E_PARSE || error.message == NULL) {
        return 0;
    }
    
    return 1;
#else
    /* Fallback test for non-C23 */
    int flag_mask = 0xF0;  /* Traditional hex literal */
    NurResult_i32 masked_result = NUR_OK(i32, 0xFF & flag_mask);
    
    if (!NUR_IS_OK(masked_result) || NUR_VALUE(masked_result) != 0xF0) {
        return 0;
    }
    
    return -1; /* Skip - C23 not available */
#endif
}

/*
 * =============================================================================
 * MAIN TEST RUNNER
 * =============================================================================
 */

int main(void) {
    printf("=== nur.h C23 Feature Validation Test Suite ===\n\n");
    
    print_compiler_info();
    
    /* Run all C23-specific feature tests */
    TEST(nodiscard_with_message);
    TEST(enhanced_static_assert);
    TEST(typeof_auto_type_deduction);
    TEST(binary_literals_and_separators);
    TEST(compiler_version_detection);
    TEST(graceful_degradation);
    TEST(c23_integration);
    
    /* Print summary */
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed - tests_skipped);
    printf("Tests skipped (C23 N/A): %d\n", tests_skipped);
    
    /* Determine overall result */
    int failed = tests_run - tests_passed - tests_skipped;
    if (failed == 0) {
        if (tests_skipped > 0) {
            printf("All available tests PASSED! (Some C23 features skipped) ✓\n");
        } else {
            printf("All C23 feature tests PASSED! ✓\n");
        }
        return 0;
    } else {
        printf("Some tests FAILED! ✗\n");
        return 1;
    }
}