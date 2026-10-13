/*
 * Comprehensive End-to-End Test Suite for nur.h
 *
 * Tests:
 * 1. Single-include batteries-included standard headers (<stdio.h>, <stdlib.h>,
 *    <string.h>, <math.h>, <time.h>, <ctype.h>, <stdbool.h>, <stdint.h>, etc.)
 *    WITHOUT ANY SEPARATE STANDARD HEADER INCLUDES!
 * 2. All syntaxes and shortcut constructors (NUR_H, 1-arg NUR_OK, 2-arg NUR_OK, void)
 * 3. Intentional execution of multiple, varied error scenarios and verification
 * 4. Error chaining, JSON logging, control flow guards, and unwrap fallbacks
 */

/* NOTE: We ONLY include "nur.h" here to verify that nur.h provides
 * all standard library headers out-of-the-box! */
#include "../src/nur.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(name) \
    do { \
        printf("Running test: %s... ", #name); \
        tests_run++; \
        if (test_##name()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
        } \
    } while (0)

/* =========================================================================
 * 1. TEST BATTERIES-INCLUDED STANDARD LIBRARY HEADERS INCLUSION
 * ========================================================================= */
static int test_batteries_included_headers(void) {
    /* stdio.h */
    char buf[128];
    int snp_len = snprintf(buf, sizeof(buf), "%s-%d", "nur", 42);
    if (snp_len <= 0 || strcmp(buf, "nur-42") != 0) return 0;

    /* stdlib.h */
    int val_abs = abs(-100);
    if (val_abs != 100) return 0;
    int parsed_int = atoi("54321");
    if (parsed_int != 54321) return 0;

    /* string.h */
    if (strlen("hello") != 5) return 0;
    if (strcmp("abc", "abc") != 0) return 0;

    /* math.h */
    double sq = sqrt(16.0);
    if (fabs(sq - 4.0) > 0.0001) return 0;

    /* time.h */
    time_t t = time(NULL);
    if (t == (time_t)(-1)) return 0;

    /* ctype.h */
    if (!isdigit('5')) return 0;
    if (!isalpha('A')) return 0;
    if (toupper('b') != 'B') return 0;

    /* stdbool.h */
    bool flag = true;
    if (!flag) return 0;

    /* stdint.h */
    int32_t i32_val = 12345;
    uint64_t u64_val = 99999ULL;
    if (i32_val != 12345 || u64_val != 99999ULL) return 0;

    /* errno.h */
    if (errno != 0 && errno == -9999) return 0; /* Just testing errno symbol exists */

    return 1;
}

/* =========================================================================
 * 2. TEST ALL CONSTRUCTORS & SHORTCUT SYNTAXES
 * ========================================================================= */
static int test_all_constructors_and_shortcuts(void) {
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
    /* 1. Universal NUR_H single-token syntax */
    NurResult_i32 h_i32 = NUR_H(101);
    if (!NUR_IS_OK(h_i32) || NUR_VALUE(h_i32) != 101) return 0;

    NurResult_i64 h_i64 = NUR_H((int64_t)9999999999LL);
    if (!NUR_IS_OK(h_i64) || NUR_VALUE(h_i64) != 9999999999LL) return 0;

    NurResult_usize h_usize = NUR_H((size_t)2048);
    if (!NUR_IS_OK(h_usize) || NUR_VALUE(h_usize) != 2048) return 0;

    NurResult_f32 h_f32 = NUR_H(1.25f);
    if (!NUR_IS_OK(h_f32) || fabs(NUR_VALUE(h_f32) - 1.25f) > 0.001) return 0;

    NurResult_f64 h_f64 = NUR_H(3.14159);
    if (!NUR_IS_OK(h_f64) || fabs(NUR_VALUE(h_f64) - 3.14159) > 0.00001) return 0;

    NurResult_cstr h_cstr = NUR_H("Universal string");
    if (!NUR_IS_OK(h_cstr) || strcmp(NUR_VALUE(h_cstr), "Universal string") != 0) return 0;

    void *test_ptr = (void*)(uintptr_t)0x55aa55aa;
    NurResult_ptr h_ptr = NUR_H(test_ptr);
    if (!NUR_IS_OK(h_ptr) || NUR_VALUE(h_ptr) != test_ptr) return 0;

    bool b = true;
    NurResult_bool h_bool = NUR_H(b);
    if (!NUR_IS_OK(h_bool) || NUR_VALUE(h_bool) != true) return 0;

    /* 2. Modern overloaded 1-argument NUR_OK syntax */
    NurResult_i32 ok1_i32 = NUR_OK(202);
    if (!NUR_IS_OK(ok1_i32) || NUR_VALUE(ok1_i32) != 202) return 0;

    NurResult_cstr ok1_cstr = NUR_OK("1-arg ok");
    if (!NUR_IS_OK(ok1_cstr) || strcmp(NUR_VALUE(ok1_cstr), "1-arg ok") != 0) return 0;
#endif

    /* 3. Classical explicit 2-argument NUR_OK syntax (C99 through C23) */
    NurResult_i32 ok2_i32 = NUR_OK(i32, 303);
    if (!NUR_IS_OK(ok2_i32) || NUR_VALUE(ok2_i32) != 303) return 0;

    NurResult_cstr ok2_cstr = NUR_OK(cstr, "2-arg explicit");
    if (!NUR_IS_OK(ok2_cstr) || strcmp(NUR_VALUE(ok2_cstr), "2-arg explicit") != 0) return 0;

    /* 4. Void Result */
    NurResult_void ok_v = NUR_OK_VOID();
    if (!NUR_IS_OK(ok_v)) return 0;

    NurResult_void err_v = NUR_ERR_VOID(NUR_ERROR(NUR_E_INTERNAL, "void failure"));
    if (NUR_IS_OK(err_v) || NUR_VOID_ERROR_OF(err_v).code != NUR_E_INTERNAL) return 0;

    return 1;
}

/* =========================================================================
 * 3. TEST INTENTIONAL ERRORS: ARITHMETIC OVERFLOW, DIV-BY-ZERO, FILE I/O
 * ========================================================================= */
static int test_intentional_arithmetic_errors(void) {
    /* 1. Addition Overflow */
    NurResult_i32 add_ovf = nur_add_i32(INT32_MAX, 1);
    if (NUR_IS_OK(add_ovf)) return 0;
    if (NUR_ERROR_OF(add_ovf).code != NUR_E_OVERFLOW) return 0;

    /* 2. Subtraction Overflow / Underflow */
    NurResult_i32 sub_ovf = nur_sub_i32(INT32_MIN, 1);
    if (NUR_IS_OK(sub_ovf)) return 0;
    if (NUR_ERROR_OF(sub_ovf).code != NUR_E_OVERFLOW) return 0;

    /* 3. Multiplication Overflow */
    NurResult_i32 mul_ovf = nur_mul_i32(INT32_MAX / 2 + 10, 2);
    if (NUR_IS_OK(mul_ovf)) return 0;
    if (NUR_ERROR_OF(mul_ovf).code != NUR_E_OVERFLOW) return 0;

    /* 4. Division by Zero */
    NurResult_i32 div_zero = nur_div_i32(42, 0);
    if (NUR_IS_OK(div_zero)) return 0;
    if (NUR_ERROR_OF(div_zero).code != NUR_E_INVALID_ARGUMENT) return 0;

    /* 5. Division INT32_MIN by -1 Overflow */
    NurResult_i32 div_min = nur_div_i32(INT32_MIN, -1);
    if (NUR_IS_OK(div_min)) return 0;
    if (NUR_ERROR_OF(div_min).code != NUR_E_OVERFLOW) return 0;

    return 1;
}

static int test_intentional_file_errors(void) {
    char buf[128];

    /* 1. Non-existent file read */
    NurResult_cstr non_exist = nur_file_read_text("absolutely_missing_file_8877.txt", buf, sizeof(buf));
    if (NUR_IS_OK(non_exist)) return 0;
    if (NUR_ERROR_OF(non_exist).code != NUR_E_NOT_FOUND) return 0;

    /* 2. NULL path */
    NurResult_cstr null_path = nur_file_read_text(NULL, buf, sizeof(buf));
    if (NUR_IS_OK(null_path)) return 0;
    if (NUR_ERROR_OF(null_path).code != NUR_E_INVALID_ARGUMENT) return 0;

    /* 3. NULL buffer */
    NurResult_cstr null_buf = nur_file_read_text("some_file.txt", NULL, sizeof(buf));
    if (NUR_IS_OK(null_buf)) return 0;
    if (NUR_ERROR_OF(null_buf).code != NUR_E_INVALID_ARGUMENT) return 0;

    /* 4. Zero buffer capacity */
    NurResult_cstr zero_cap = nur_file_read_text("some_file.txt", buf, 0);
    if (NUR_IS_OK(zero_cap)) return 0;
    if (NUR_ERROR_OF(zero_cap).code != NUR_E_INVALID_ARGUMENT) return 0;

    /* 5. Valid file read and verify null termination */
    const char *tmp_name = "test_auto_io_valid.txt";
    FILE *f = fopen(tmp_name, "w");
    if (!f) return 0;
    fputs("Sample file content for testing", f);
    fclose(f);

    NurResult_cstr read_ok = nur_file_read_text(tmp_name, buf, sizeof(buf));
    remove(tmp_name);

    if (!NUR_IS_OK(read_ok)) return 0;
    if (strcmp(NUR_VALUE(read_ok), "Sample file content for testing") != 0) return 0;

    /* 6. Truncation guard test: buffer smaller than file content */
    f = fopen(tmp_name, "w");
    if (!f) return 0;
    fputs("This is a long text that will not fit in a small buffer", f);
    fclose(f);

    char small_buf[16];
    NurResult_cstr trunc_res = nur_file_read_text(tmp_name, small_buf, sizeof(small_buf));
    remove(tmp_name);

    if (NUR_IS_OK(trunc_res)) return 0; /* MUST NOT SUCCEED SILENTLY! */
    if (NUR_ERROR_OF(trunc_res).code != NUR_E_TRUNCATED) return 0;

    return 1;
}

/* =========================================================================
 * 4. TEST ERROR TAXONOMY, ROOT CAUSE CHAINING & JSON LOGGING
 * ========================================================================= */
static int test_error_chaining_and_json(void) {
    /* 1. Taxonomy code string */
    const char *name = nur_error_code_name(NUR_E_RESOURCE_LIMIT);
    if (strcmp(name, "NUR_E_RESOURCE_LIMIT") != 0) return 0;

    /* 2. Root cause chaining via NUR_ERROR_WRAP */
    nur_error_t root_err = NUR_ERROR_CTX(NUR_E_CONNECTION, "socket reset by peer", "tcp://localhost:8080");
    nur_error_t wrapped_err = NUR_ERROR_WRAP(root_err, "failed to send telemetry request");

    if (wrapped_err.code != NUR_E_CONNECTION) return 0;
#ifdef NUR_COMPACT_ERRORS
    if (wrapped_err.message == NULL) return 0;

    /* Verify compact size: sizeof(NurResult_i32) is <= 24 bytes on 64-bit */
    if (sizeof(nur_error_t) > 24) return 0;

    /* 3. JSON Serialization */
    char json_buffer[512];
    int written = (int)nur_error_json(&wrapped_err, json_buffer, sizeof(json_buffer));
    if (written <= 0) return 0;

    if (strstr(json_buffer, "\"code\":") == NULL) return 0;
    if (strstr(json_buffer, "\"name\":\"NUR_E_CONNECTION\"") == NULL) return 0;

    return 1;
#else
    if (strcmp(wrapped_err.context, "failed to send telemetry request") != 0) return 0;
    if (wrapped_err.cause == NULL || strcmp(wrapped_err.cause, "tcp://localhost:8080") != 0) return 0;

    /* 3. JSON Serialization */
    char json_buffer[512];
    int written = nur_error_json(&wrapped_err, json_buffer, sizeof(json_buffer));
    if (written <= 0) return 0;

    /* Verify JSON fields exist */
    if (strstr(json_buffer, "\"code\":") == NULL) return 0;
    if (strstr(json_buffer, "\"name\":\"NUR_E_CONNECTION\"") == NULL) return 0;
    if (strstr(json_buffer, "\"message\":\"socket reset by peer\"") == NULL) return 0;
    if (strstr(json_buffer, "\"context\":\"failed to send telemetry request\"") == NULL) return 0;
    if (strstr(json_buffer, "\"cause\":\"tcp://localhost:8080\"") == NULL) return 0;

    return 1;
#endif
}

/* =========================================================================
 * 5. TEST CONTROL FLOW MACROS (TRY_GOTO, CHECK, UNWRAP_OR, MATCH)
 * ========================================================================= */
static NurResult_i32 mock_failing_function(void) {
    return NUR_ERR(i32, NUR_ERROR(NUR_E_PERMISSION, "access denied"));
}

static NurResult_i32 mock_succeeding_function(void) {
    return NUR_OK(i32, 888);
}

static int g_check_val = 0;
static NurResult_void check_fn(int c) {
    NUR_CHECK(c > 0, NUR_ERROR(NUR_E_OUT_OF_RANGE, "must be positive"));
    g_check_val = c;
    return NUR_OK_VOID();
}

static int test_control_flow_and_guards(void) {
    /* 1. NUR_UNWRAP_OR fallback */
    int val1 = NUR_UNWRAP_OR(mock_failing_function(), -99);
    if (val1 != -99) return 0;

    int val2 = NUR_UNWRAP_OR(mock_succeeding_function(), -99);
    if (val2 != 888) return 0;

    /* 2. NUR_TRY_GOTO leak-free cleanup test */
    int cleanup_reached = 0;
    int received_val = 0;
    nur_error_t captured_err = {0};

    NUR_TRY_GOTO(received_val, mock_failing_function(), captured_err, cleanup_step);

    /* Should not be reached */
    (void)received_val;
    return 0;

cleanup_step:
    cleanup_reached = 1;
    (void)received_val;
    if (cleanup_reached != 1 || captured_err.code != NUR_E_PERMISSION) return 0;

    /* 3. NUR_CHECK test */
    NurResult_void cr1 = check_fn(-5);
    if (NUR_IS_OK(cr1) || NUR_VOID_ERROR_OF(cr1).code != NUR_E_OUT_OF_RANGE) return 0;

    NurResult_void cr2 = check_fn(10);
    if (!NUR_IS_OK(cr2) || g_check_val != 10) return 0;

    return 1;
}

int main(void) {
    printf("============================================================\n");
    printf("nur.h Complete Feature & Error Verification Suite (A to Z)\n");
    printf("============================================================\n\n");

    RUN_TEST(batteries_included_headers);
    RUN_TEST(all_constructors_and_shortcuts);
    RUN_TEST(intentional_arithmetic_errors);
    RUN_TEST(intentional_file_errors);
    RUN_TEST(error_chaining_and_json);
    RUN_TEST(control_flow_and_guards);

    printf("\n============================================================\n");
    printf("Summary: %d / %d tests passed.\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("ALL A-TO-Z TESTS AND ERROR SCENARIOS PASSED CLEANLY! [OK]\n");
        printf("============================================================\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED! [FAIL]\n");
        printf("============================================================\n");
        return 1;
    }
}
