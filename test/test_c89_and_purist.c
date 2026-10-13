/**
 * @file test_c89_and_purist.c
 * @brief Test suite verifying ISO C89/C90 compliance and C-purist pure-function API.
 *
 * Verifies:
 * 1. Zero warnings under ISO C89 with -Wall -Wextra -Wpedantic.
 * 2. Pure-function result checking (nur_is_ok_*, nur_value_*, nur_error_*).
 * 3. Universal result pointer predicates (nur_result_is_ok, nur_result_is_err).
 * 4. Zero hidden returns in NUR_PURIST mode (compile-time safety).
 * 5. Safe goto cleanup pattern (NUR_TRY_GOTO / explicit goto).
 */

#define NUR_PURIST 1

#include "../src/nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_RUN(name) do { \
    printf("Running test: %s... ", #name); \
    g_tests_run++; \
    if (test_##name()) { \
        printf("PASS\n"); \
        g_tests_passed++; \
    } else { \
        printf("FAIL\n"); \
    } \
} while (0)

/* Helper function returning NurResult_i32 */
static NurResult_i32 safe_div(int32_t a, int32_t b) {
    if (b == 0) {
        return nur_err_i32(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "divide by zero"));
    }
    return nur_ok_i32(a / b);
}

/* Helper function returning NurResult_void */
static NurResult_void check_positive(int32_t n) {
    if (n <= 0) {
        return nur_err_void(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "must be positive"));
    }
    return nur_ok_void();
}

/* Test 1: Basic pure function API (macro-free checks) */
static int test_pure_function_api(void) {
    NurResult_i32 ok_res;
    NurResult_i32 err_res;
    int32_t val;
    nur_error_t err;

    ok_res = safe_div(42, 2);
    if (!nur_is_ok_i32(ok_res)) return 0;
    if (nur_is_err_i32(ok_res)) return 0;
    val = nur_value_i32(ok_res);
    if (val != 21) return 0;
    if (nur_unwrap_or_i32(ok_res, 0) != 21) return 0;

    err_res = safe_div(42, 0);
    if (nur_is_ok_i32(err_res)) return 0;
    if (!nur_is_err_i32(err_res)) return 0;
    err = nur_error_i32(err_res);
    if (nur_error_code_of(&err) != NUR_E_INVALID_ARGUMENT) return 0;
    if (strcmp(nur_error_message_of(&err), "divide by zero") != 0) return 0;
    if (nur_unwrap_or_i32(err_res, -1) != -1) return 0;

    return 1;
}

/* Test 2: Universal pointer predicates */
static int test_universal_pointer_predicates(void) {
    NurResult_i32 r_i32;
    NurResult_cstr r_cstr;

    r_i32 = safe_div(100, 5);
    if (!nur_result_is_ok(&r_i32)) return 0;
    if (nur_result_is_err(&r_i32)) return 0;

    r_cstr = nur_err_cstr(NUR_ERROR(NUR_E_NOT_FOUND, "missing"));
    if (nur_result_is_ok(&r_cstr)) return 0;
    if (!nur_result_is_err(&r_cstr)) return 0;

    /* NULL safety */
    if (nur_result_is_ok(NULL)) return 0;
    if (!nur_result_is_err(NULL)) return 0;

    return 1;
}

/* Test 3: Void result with pure functions */
static int test_void_pure_functions(void) {
    NurResult_void v_ok;
    NurResult_void v_err;
    nur_error_t err;

    v_ok = check_positive(10);
    if (!nur_is_ok_void(v_ok)) return 0;
    if (nur_is_err_void(v_ok)) return 0;

    v_err = check_positive(-5);
    if (nur_is_ok_void(v_err)) return 0;
    if (!nur_is_err_void(v_err)) return 0;

    err = nur_error_void_val(v_err);
    if (nur_error_code_of(&err) != NUR_E_INVALID_ARGUMENT) return 0;

    return 1;
}

/* Test 4: Resource leak-free cleanup with NUR_TRY_GOTO */
static int test_resource_cleanup_goto(void) {
    char *allocated_resource;
    int32_t final_val;
    nur_error_t err_out;
    int cleaned_up;

    allocated_resource = (char *)malloc(128);
    if (!allocated_resource) return 0;
    strcpy(allocated_resource, "allocated_memory");
    cleaned_up = 0;
    memset(&err_out, 0, sizeof(err_out));

    /* Attempt an operation that fails, ensuring resource is freed before return */
    NUR_TRY_GOTO(final_val, safe_div(10, 0), err_out, cleanup);

    /* Should not reach here */
    (void)final_val;
    free(allocated_resource);
    return 0;

cleanup:
    if (allocated_resource != NULL) {
        free(allocated_resource);
        allocated_resource = NULL;
        cleaned_up = 1;
    }

    if (!cleaned_up) return 0;
    if (err_out.code != NUR_E_INVALID_ARGUMENT) return 0;
    return 1;
}

int main(void) {
    printf("=== nur.h ISO C89 & C-Purist Test Suite ===\n\n");

    TEST_RUN(pure_function_api);
    TEST_RUN(universal_pointer_predicates);
    TEST_RUN(void_pure_functions);
    TEST_RUN(resource_cleanup_goto);

    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", g_tests_run);
    printf("Tests passed: %d\n", g_tests_passed);
    printf("Tests failed: %d\n", g_tests_run - g_tests_passed);

    if (g_tests_run == g_tests_passed) {
        printf("All C89/Purist tests PASSED! [OK]\n");
        return 0;
    }
    return 1;
}
