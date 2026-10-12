/*
 * Test Suite for nur.h Universal Type Engine, Overloaded NUR_OK, Safe Math & Safe File I/O
 * (Pillars 1, 2, and 4)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "../src/nur.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("Running test: %s... ", #name); \
        tests_run++; \
        if (test_##name()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
        } \
    } while(0)

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)

/* Test Pillar 1: NUR_H single-token generic constructor */
static int test_nur_h_single_token(void) {
    NurResult_i32 r_i32 = NUR_H(42);
    if (!NUR_IS_OK(r_i32) || NUR_VALUE(r_i32) != 42) return 0;

    NurResult_i64 r_i64 = NUR_H((int64_t)9876543210LL);
    if (!NUR_IS_OK(r_i64) || NUR_VALUE(r_i64) != 9876543210LL) return 0;

    NurResult_usize r_size = NUR_H((size_t)1024);
    if (!NUR_IS_OK(r_size) || NUR_VALUE(r_size) != 1024) return 0;

    NurResult_f32 r_f32 = NUR_H(3.14f);
    if (!NUR_IS_OK(r_f32) || NUR_VALUE(r_f32) < 3.13f || NUR_VALUE(r_f32) > 3.15f) return 0;

    NurResult_f64 r_f64 = NUR_H(2.71828);
    if (!NUR_IS_OK(r_f64) || NUR_VALUE(r_f64) < 2.71 || NUR_VALUE(r_f64) > 2.72) return 0;

    NurResult_cstr r_cstr = NUR_H("hello universal");
    if (!NUR_IS_OK(r_cstr) || strcmp(NUR_VALUE(r_cstr), "hello universal") != 0) return 0;

    void *dummy_ptr = (void*)(uintptr_t)0xdeadbeef;
    NurResult_ptr r_ptr = NUR_H(dummy_ptr);
    if (!NUR_IS_OK(r_ptr) || NUR_VALUE(r_ptr) != dummy_ptr) return 0;

    bool b = true;
    NurResult_bool r_bool = NUR_H(b);
    if (!NUR_IS_OK(r_bool) || NUR_VALUE(r_bool) != true) return 0;

    return 1;
}

/* Test Pillar 1: Overloaded NUR_OK (1-arg generic vs 2-arg explicit) */
static int test_overloaded_nur_ok(void) {
    /* 1-arg modern generic usage */
    NurResult_i32 r1 = NUR_OK(77);
    if (!NUR_IS_OK(r1) || NUR_VALUE(r1) != 77) return 0;

    NurResult_cstr r2 = NUR_OK("modern ok string");
    if (!NUR_IS_OK(r2) || strcmp(NUR_VALUE(r2), "modern ok string") != 0) return 0;

    /* 2-arg legacy/explicit usage */
    NurResult_i32 r3 = NUR_OK(i32, 88);
    if (!NUR_IS_OK(r3) || NUR_VALUE(r3) != 88) return 0;

    NurResult_cstr r4 = NUR_OK(cstr, "classic ok string");
    if (!NUR_IS_OK(r4) || strcmp(NUR_VALUE(r4), "classic ok string") != 0) return 0;

    return 1;
}

#endif /* C11+ */

/* Test Pillar 4: Checked arithmetic */
static int test_safe_math(void) {
    /* Addition */
    NurResult_i32 add_ok = nur_add_i32(15, 25);
    if (!NUR_IS_OK(add_ok) || NUR_VALUE(add_ok) != 40) return 0;

    NurResult_i32 add_ovf = nur_add_i32(INT32_MAX, 1);
    if (NUR_IS_OK(add_ovf) || NUR_ERROR_OF(add_ovf).code != NUR_E_OVERFLOW) return 0;

    /* Subtraction */
    NurResult_i32 sub_ok = nur_sub_i32(100, 30);
    if (!NUR_IS_OK(sub_ok) || NUR_VALUE(sub_ok) != 70) return 0;

    NurResult_i32 sub_ovf = nur_sub_i32(INT32_MIN, 1);
    if (NUR_IS_OK(sub_ovf) || NUR_ERROR_OF(sub_ovf).code != NUR_E_OVERFLOW) return 0;

    /* Multiplication */
    NurResult_i32 mul_ok = nur_mul_i32(6, 7);
    if (!NUR_IS_OK(mul_ok) || NUR_VALUE(mul_ok) != 42) return 0;

    NurResult_i32 mul_ovf = nur_mul_i32(INT32_MAX / 2 + 1, 2);
    if (NUR_IS_OK(mul_ovf) || NUR_ERROR_OF(mul_ovf).code != NUR_E_OVERFLOW) return 0;

    /* Division */
    NurResult_i32 div_ok = nur_div_i32(100, 4);
    if (!NUR_IS_OK(div_ok) || NUR_VALUE(div_ok) != 25) return 0;

    NurResult_i32 div_zero = nur_div_i32(50, 0);
    if (NUR_IS_OK(div_zero) || NUR_ERROR_OF(div_zero).code != NUR_E_INVALID_ARGUMENT) return 0;

    NurResult_i32 div_ovf = nur_div_i32(INT32_MIN, -1);
    if (NUR_IS_OK(div_ovf) || NUR_ERROR_OF(div_ovf).code != NUR_E_OVERFLOW) return 0;

    return 1;
}

/* Test Pillar 4: Safe file reader */
static int test_safe_file_reader(void) {
    char buf[256];

    /* Test non-existent file */
    NurResult_cstr missing = nur_file_read_text("non_existent_file_9999.txt", buf, sizeof(buf));
    if (NUR_IS_OK(missing)) return 0;
    if (NUR_ERROR_OF(missing).code != NUR_E_NOT_FOUND) return 0;

    /* Test invalid argument */
    NurResult_cstr inval = nur_file_read_text(NULL, buf, sizeof(buf));
    if (NUR_IS_OK(inval) || NUR_ERROR_OF(inval).code != NUR_E_INVALID_ARGUMENT) return 0;

    /* Create temporary test file */
    const char *tmp_filename = "test_safe_read_tmp.txt";
    FILE *f = fopen(tmp_filename, "w");
    if (!f) return 0;
    fputs("Hello, nur.h Batteries-Included!", f);
    fclose(f);

    /* Read back with nur_file_read_text */
    NurResult_cstr read_res = nur_file_read_text(tmp_filename, buf, sizeof(buf));
    remove(tmp_filename);

    if (!NUR_IS_OK(read_res)) return 0;
    if (strcmp(NUR_VALUE(read_res), "Hello, nur.h Batteries-Included!") != 0) return 0;

    return 1;
}

int main(void) {
    printf("=== nur.h Universal Type & Batteries-Included Test Suite ===\n\n");

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || defined(NUR_C11)
    TEST(nur_h_single_token);
    TEST(overloaded_nur_ok);
#endif

    TEST(safe_math);
    TEST(safe_file_reader);

    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_run == tests_passed) {
        printf("All tests PASSED! ✓\n");
        return 0;
    } else {
        printf("Some tests FAILED! ✗\n");
        return 1;
    }
}
