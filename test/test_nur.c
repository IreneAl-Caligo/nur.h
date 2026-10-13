/*
 * Test Suite for nur.h Library
 * 
 * This file tests all major functionality of the nur.h error handling library
 * including Result types, error propagation, and utility functions.
 */

#if defined(_MSC_VER)
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS 1
#  endif
#  pragma warning(disable: 4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Include the library under test */
#include "../src/nur.h"

/* Test counter */
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

/* Test basic Result<i32> operations */
static int test_result_i32_basic(void) {
    NurResult_i32 ok_result = NUR_OK(i32, 42);
    NurResult_i32 err_result = NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Test error"));
    
    if (!NUR_IS_OK(ok_result)) return 0;
    if (NUR_IS_OK(err_result)) return 0;
    if (NUR_UNWRAP(ok_result) != 42) return 0;
    
    return 1;
}

/* Test error propagation with NUR_TRY */
static NurResult_i32 divide_safe(int a, int b) {
    if (b == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Division by zero"));
    }
    return NUR_OK(i32, a / b);
}

static NurResult_i32 test_propagation_helper(int a, int b) {
    int result;
    NUR_TRY_TO(i32, i32, result, divide_safe(a, b));
    return NUR_OK(i32, result * 2);
}

static int test_error_propagation(void) {
    NurResult_i32 ok_result = test_propagation_helper(10, 2);
    NurResult_i32 err_result = test_propagation_helper(10, 0);
    
    if (!NUR_IS_OK(ok_result)) return 0;
    if (NUR_UNWRAP(ok_result) != 10) return 0; /* (10/2)*2 = 10 */
    if (NUR_IS_OK(err_result)) return 0;
    
    return 1;
}

/* Test predefined types */
static int test_predefined_types(void) {
    NurResult_cstr str_ok = NUR_OK(cstr, "hello");
    NurResult_ptr ptr_ok = NUR_OK(ptr, (void*)0x12345);
    NurResult_f64 float_ok = NUR_OK(f64, 3.14159);
    NurResult_void void_ok = NUR_OK_VOID();
    
    if (!NUR_IS_OK(str_ok)) return 0;
    if (!NUR_IS_OK(ptr_ok)) return 0;
    if (!NUR_IS_OK(float_ok)) return 0;
    if (!NUR_IS_OK(void_ok)) return 0;
    
    if (strcmp(NUR_UNWRAP(str_ok), "hello") != 0) return 0;
    if (NUR_UNWRAP(ptr_ok) != (void*)0x12345) return 0;
    if (NUR_UNWRAP(float_ok) != 3.14159) return 0;
    
    return 1;
}

/* Test error categories */
static int test_error_categories(void) {
    NurResult_i32 input_err = NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Input error"));
    NurResult_i32 data_err = NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "Parse error"));
    NurResult_i32 resource_err = NUR_ERR(i32, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "Memory error"));
    NurResult_i32 io_err = NUR_ERR(i32, NUR_ERROR(NUR_E_NOT_FOUND, "File not found"));
    
    if (NUR_IS_OK(input_err) || NUR_IS_OK(data_err) || 
        NUR_IS_OK(resource_err) || NUR_IS_OK(io_err)) {
        return 0;
    }
    
    /* Check error codes are in correct ranges - access via union */
    if (input_err.u.error.code < 1 || input_err.u.error.code > 9) return 0;
    if (data_err.u.error.code < 10 || data_err.u.error.code > 19) return 0;
    if (resource_err.u.error.code < 20 || resource_err.u.error.code > 29) return 0;
    if (io_err.u.error.code < 30 || io_err.u.error.code > 39) return 0;
    
    return 1;
}

/* Test custom Result type generation */
typedef struct {
    int x, y;
} Point;

NUR_RESULT_TYPE(Point, point);

typedef struct {
    int app_code;
    const char *detail;
} CustomErr;

NUR_RESULT_TYPE_E(int, CustomErr, custom_e);

static int test_custom_result_type(void) {
    Point p = {10, 20};
    NurResult_point ok_point = NUR_OK(point, p);
    NurResult_point err_point = NUR_ERR(point, NUR_ERROR(NUR_E_INTERNAL, "Point error"));
    
    if (!NUR_IS_OK(ok_point)) return 0;
    if (NUR_IS_OK(err_point)) return 0;
    
    Point unwrapped = NUR_UNWRAP(ok_point);
    if (unwrapped.x != 10 || unwrapped.y != 20) return 0;
    
    return 1;
}

/* Test error utilities */
static int test_error_utilities(void) {
    NurResult_i32 err = NUR_ERR(i32, NUR_ERROR(NUR_E_NOT_FOUND, "Test message"));
    NUR_IGNORE(err); /* Result constructed to verify macro compiles; value not inspected here */

    /* Test error code name function */
    const char* code_name = nur_error_code_name(NUR_E_NOT_FOUND);
    if (code_name == NULL) return 0;
    if (strstr(code_name, "NOT_FOUND") == NULL) return 0;

    return 1;
}

/* Test Option types */
static int test_option_types(void) {
    NurOption_i32 some_val = NUR_SOME(i32, 42);
    NurOption_i32 none_val = NUR_NONE(i32);
    
    if (!NUR_IS_SOME(some_val)) return 0;
    if (NUR_IS_SOME(none_val)) return 0;
    if (NUR_OPTION_UNWRAP_OR(some_val, 0) != 42) return 0;
    if (NUR_OPTION_UNWRAP_OR(none_val, 99) != 99) return 0;
    
    return 1;
}

/* Test macro hygiene (no double evaluation) */
static int counter = 0;
static int increment_counter(void) {
    return ++counter;
}

static NurResult_i32 make_counter_result(void) {
    return NUR_OK(i32, ++counter);
}

static int test_macro_hygiene(void) {
    counter = 0;
    NurResult_i32 result = NUR_OK(i32, increment_counter());
    
    /* Counter should only be incremented once during construction */
    if (counter != 1) return 0;
    if (NUR_UNWRAP(result) != 1) return 0;

    /* NUR_UNWRAP must evaluate its expression exactly once */
    counter = 0;
    int val = NUR_UNWRAP(make_counter_result());
    if (counter != 1 || val != 1) return 0;

    /* NUR_UNWRAP_OR must evaluate its expression exactly once */
    counter = 0;
    int val_or = NUR_UNWRAP_OR(make_counter_result(), -1);
    if (counter != 1 || val_or != 1) return 0;
    
    return 1;
}

/* Test NUR_CHECK and NUR_CHECK_AS guard macros */
static NurResult_void check_positive_void(int v) {
    NUR_CHECK(v > 0, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "value must be positive"));
    return NUR_OK_VOID();
}

static NurResult_i32 check_positive_i32(int v) {
    NUR_CHECK_AS(i32, v > 0, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "value must be positive"));
    return NUR_OK(i32, v * 2);
}

static int test_nur_check(void) {
    /* NUR_CHECK success path */
    NurResult_void ok = check_positive_void(5);
    if (!NUR_IS_OK(ok)) return 0;

    /* NUR_CHECK failure path */
    NurResult_void fail = check_positive_void(-1);
    if (NUR_IS_OK(fail)) return 0;
    if (fail.error.code != NUR_E_INVALID_ARGUMENT) return 0;

    /* NUR_CHECK_AS success path */
    NurResult_i32 ok2 = check_positive_i32(7);
    if (!NUR_IS_OK(ok2)) return 0;
    if (NUR_UNWRAP(ok2) != 14) return 0;

    /* NUR_CHECK_AS failure path */
    NurResult_i32 fail2 = check_positive_i32(0);
    if (NUR_IS_OK(fail2)) return 0;
    if (fail2.u.error.code != NUR_E_INVALID_ARGUMENT) return 0;

    return 1;
}

/* Test nur_malloc / nur_realloc allocation wrappers */
static int test_nur_malloc(void) {
    /* Successful allocation */
    NurResult_ptr r = nur_malloc(64);
    if (!NUR_IS_OK(r)) return 0;
    void *p = NUR_UNWRAP(r);
    if (p == NULL) return 0;

    /* Write and read back to confirm pointer is usable */
    ((char *)p)[0] = 'N';
    ((char *)p)[1] = 'U';
    ((char *)p)[2] = 'R';
    ((char *)p)[3] = '\0';
    if (strcmp((char *)p, "NUR") != 0) { free(p); return 0; }
    free(p);

    /* Zero-size allocation must return an error */
    NurResult_ptr zero = nur_malloc(0);
    if (NUR_IS_OK(zero)) return 0;
    if (zero.u.error.code != NUR_E_INVALID_ARGUMENT) return 0;

    /* Realloc success */
    NurResult_ptr r2 = nur_malloc(8);
    if (!NUR_IS_OK(r2)) return 0;
    void *p2 = NUR_UNWRAP(r2);
    NurResult_ptr r3 = nur_realloc(p2, 128);
    if (!NUR_IS_OK(r3)) { free(p2); return 0; }
    free(NUR_UNWRAP(r3));

    /* Realloc zero-size must return an error */
    NurResult_ptr r4 = nur_malloc(8);
    if (!NUR_IS_OK(r4)) return 0;
    void *p4 = NUR_UNWRAP(r4);
    NurResult_ptr bad = nur_realloc(p4, 0);
    if (NUR_IS_OK(bad)) { free(p4); return 0; }
    free(p4);

    return 1;
}

/* Test NUR_ERROR_ERRNO captures errno correctly */
static int test_error_errno(void) {
    /* Force errno to a known value then build an error from it */
    errno = ENOENT;
    nur_error_t e = NUR_ERROR_ERRNO("open failed");

    if (e.code != NUR_E_NOT_FOUND) return 0;
    if (e.sys  != ENOENT)          return 0;
    if (e.message == NULL)         return 0;
    if (strcmp(e.message, "open failed") != 0) return 0;
    /* Source location must be captured */
    if (e.file == NULL || e.func == NULL || e.line == 0) return 0;

    /* NUR_ERROR_ERRNO_CTX variant */
    errno = EACCES;
    nur_error_t e2 = NUR_ERROR_ERRNO_CTX("read failed", "/etc/shadow");
    if (e2.code    != NUR_E_PERMISSION)   return 0;
    if (e2.sys     != EACCES)             return 0;
    if (e2.context == NULL)               return 0;
    if (strcmp(e2.context, "/etc/shadow") != 0) return 0;

    errno = 0; /* restore */
    return 1;
}

/* Test NUR_ERROR_CTX preserves context field */
static int test_error_context(void) {
    nur_error_t e = NUR_ERROR_CTX(NUR_E_NOT_FOUND, "file missing", "config.toml");
    if (e.code != NUR_E_NOT_FOUND) return 0;
    if (e.context == NULL) return 0;
    if (strcmp(e.context, "config.toml") != 0) return 0;
    /* Plain NUR_ERROR must leave context NULL */
    nur_error_t e2 = NUR_ERROR(NUR_E_PARSE, "bad token");
    if (e2.context != NULL) return 0;
    return 1;
}

/* Test nur_error_print doesn't crash on valid/null inputs */
static int test_error_print(void) {
    nur_error_t e = NUR_ERROR_CTX(NUR_E_IO, "disk error", "sda1");
    e.sys = EIO;
    /* Print to /dev/null equivalent — use a temp file */
    FILE *f = tmpfile();
    if (f == NULL) return 0;
    nur_error_print(&e, f);
    /* Null-safety: must not crash */
    nur_error_print(NULL, f);
    nur_error_print(&e, NULL);
    fclose(f);
    return 1;
}

/* Test user-defined error code range */
static int test_user_error_codes(void) {
    nur_error_code_t my_code = (nur_error_code_t)(NUR_E_USER_BASE + 42);
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(my_code, "app-specific failure"));
    if (NUR_IS_OK(r)) return 0;
    if (r.u.error.code != my_code) return 0;
    /* nur_error_code_name must not return NULL for user codes */
    const char *name = nur_error_code_name(my_code);
    if (name == NULL) return 0;
    if (strstr(name, "USER") == NULL) return 0;
    return 1;
}

/* Test NUR_UNWRAP_OR and NUR_OPTION_UNWRAP_OR fallback semantics */
static int test_unwrap_or(void) {
    NurResult_i32 ok  = NUR_OK(i32, 99);
    NurResult_i32 err = NUR_ERR(i32, NUR_ERROR(NUR_E_INTERNAL, "oops"));

    if (NUR_UNWRAP_OR(ok,  -1) != 99) return 0;
    if (NUR_UNWRAP_OR(err, -1) != -1) return 0;

    NurOption_i32 some = NUR_SOME(i32, 7);
    NurOption_i32 none = NUR_NONE(i32);

    if (NUR_OPTION_UNWRAP_OR(some, 0) != 7) return 0;
    if (NUR_OPTION_UNWRAP_OR(none, 0) != 0) return 0;

    return 1;
}

/* Test NUR_IGNORE suppresses nodiscard warnings at call sites */
NUR_NODISCARD static NurResult_i32 always_ok(void) {
    return NUR_OK(i32, 1);
}

static int test_nur_ignore(void) {
#if defined(NUR_GNU_EXTENSIONS) && !defined(NUR_C23)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
    NUR_IGNORE(always_ok());
#if defined(NUR_GNU_EXTENSIONS) && !defined(NUR_C23)
#pragma GCC diagnostic pop
#endif
    return 1;
}

/* Test NUR_TRY_GOTO and NUR_TRY_VOID_GOTO leak-free cleanup */
static NurResult_i32 may_fail(int val) {
    if (val < 0) return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "negative"));
    return NUR_OK(i32, val * 10);
}

static NurResult_void may_fail_void(int val) {
    if (val < 0) return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "negative"));
    return NUR_OK_VOID();
}

static int test_try_goto(void) {
    int resource_cleaned = 0;
    nur_error_t err = {0};
    int val = 0;

    /* Failure case: jump to cleanup */
    NUR_TRY_GOTO(val, may_fail(-5), err, cleanup1);
    val += 100; /* Should not be reached */

cleanup1:
    resource_cleaned = 1;
    if (!resource_cleaned || err.code != NUR_E_INVALID_ARGUMENT) return 0;

    /* Success case */
    resource_cleaned = 0;
    err = (nur_error_t){0};
    NUR_TRY_GOTO(val, may_fail(5), err, cleanup2);
    if (val != 50) return 0;

cleanup2:
    if (err.code != NUR_E_NONE) return 0;

    /* Void goto test */
    err = (nur_error_t){0};
    NUR_TRY_VOID_GOTO(may_fail_void(-1), err, cleanup_void);
    return 0; /* Should not be reached */

cleanup_void:
    if (err.code != NUR_E_INVALID_ARGUMENT) return 0;
    return 1;
}

/* Test NUR_TRY_AUTO in strict C mode */
static NurResult_i32 auto_pipeline(int x) {
    int v1 = 0;
    NUR_TRY_AUTO(i32, v1, may_fail(x));
    return NUR_OK(i32, v1 + 1);
}

static int test_try_auto(void) {
    NurResult_i32 ok_res = auto_pipeline(3);
    if (!NUR_IS_OK(ok_res) || NUR_UNWRAP(ok_res) != 31) return 0;

    NurResult_i32 err_res = auto_pipeline(-1);
    if (NUR_IS_OK(err_res) || NUR_ERROR_OF(err_res).code != NUR_E_INVALID_ARGUMENT) return 0;

    return 1;
}

/* Test semantic Win32 mapping */
static int test_win32_mapping(void) {
    /* 10035 = WSAEWOULDBLOCK -> NUR_E_WOULD_BLOCK */
    if (nur_error_code_from_win32(10035UL) != NUR_E_WOULD_BLOCK) return 0;
    /* 38 = ERROR_HANDLE_EOF -> NUR_E_EOF */
    if (nur_error_code_from_win32(38UL) != NUR_E_EOF) return 0;
    /* 10053 = WSAECONNABORTED -> NUR_E_CONNECTION */
    if (nur_error_code_from_win32(10053UL) != NUR_E_CONNECTION) return 0;
    /* 2 = ERROR_FILE_NOT_FOUND -> NUR_E_NOT_FOUND */
    if (nur_error_code_from_win32(2UL) != NUR_E_NOT_FOUND) return 0;

    nur_error_t win_err = NUR_ERROR_FROM_WIN32(10035UL, "socket would block");
    if (win_err.code != NUR_E_WOULD_BLOCK) return 0;
    if (win_err.sys != 10035) return 0;

    return 1;
}

/* Test errno mapping */
static int test_errno_mapping(void) {
    if (nur_error_code_from_errno(ENOENT) != NUR_E_NOT_FOUND) return 0;
    if (nur_error_code_from_errno(EACCES) != NUR_E_PERMISSION) return 0;
    if (nur_error_code_from_errno(ENOMEM) != NUR_E_OUT_OF_MEMORY) return 0;
    if (nur_error_code_from_errno(EEXIST) != NUR_E_ALREADY_EXISTS) return 0;
    return 1;
}

/* Test NUR_WRAP_ERROR preserves root error attributes */
static int test_wrap_error_preserves_trace(void) {
    nur_error_t root = NUR_ERROR_CTX(NUR_E_NOT_FOUND, "root msg", "root ctx");
    nur_error_t wrapped = NUR_WRAP_ERROR(root, "outer ctx");

    if (wrapped.code != root.code) return 0;
    if (wrapped.message != root.message) return 0;
    if (wrapped.file != root.file) return 0;
    if (wrapped.func != root.func) return 0;
    if (wrapped.line != root.line) return 0;
    if (strcmp(wrapped.context, "outer ctx") != 0) return 0;
    return 1;
}

/* Test null safety and buffer formatting */
static int test_error_format_null_safety(void) {
    char buf[64] = "garbage";
    size_t len = nur_error_format(NULL, buf, sizeof(buf));
    if (len != 0) return 0;
    if (buf[0] != '\0') return 0;

    /* Format with valid error */
    nur_error_t err = NUR_ERROR(NUR_E_TIMEOUT, "operation timed out");
    len = nur_error_format(&err, buf, sizeof(buf));
    if (len == 0 || strstr(buf, "TIMEOUT") == NULL) return 0;
    return 1;
}

/* Test JSON error serialization */
static int test_error_json(void) {
    char buf[256];
    size_t len = nur_error_json(NULL, buf, sizeof(buf));
    if (len == 0 || strcmp(buf, "{\"ok\":true}") != 0) return 0;

    nur_error_t err = NUR_ERROR_CTX(NUR_E_NOT_FOUND, "missing file", "cfg.json");
    err.sys = 2;
    len = nur_error_json(&err, buf, sizeof(buf));
    if (len == 0) return 0;
    if (strstr(buf, "\"ok\":false") == NULL) return 0;
    if (strstr(buf, "\"code\":30") == NULL) return 0;
    if (strstr(buf, "\"name\":\"NUR_E_NOT_FOUND\"") == NULL) return 0;
    if (strstr(buf, "\"context\":\"cfg.json\"") == NULL) return 0;
    if (strstr(buf, "\"sys\":2") == NULL) return 0;
    return 1;
}

/* Test panic handler registration */
static void dummy_panic_handler(const char *msg, const nur_error_t *e, const char *file, unsigned int line) {
    (void)msg; (void)e; (void)file; (void)line;
}

static int test_panic_handler(void) {
    nur_set_panic_handler(dummy_panic_handler);
    nur_set_panic_handler(NULL);
    return 1;
}

/* Test void result flat layout */
static int test_void_result_flat_layout(void) {
    NurResult_void ok_v = nur_ok_void();
    if (!NUR_IS_OK(ok_v)) return 0;
    NUR_UNWRAP_VOID(ok_v);
    NUR_EXPECT_VOID(ok_v, "expected ok void");

    nur_error_t my_err = NUR_ERROR(NUR_E_BUSY, "resource locked");
    NurResult_void err_v = nur_err_void(my_err);
    if (NUR_IS_OK(err_v)) return 0;
    if (NUR_ERROR_OF(err_v).code != NUR_E_BUSY) return 0;
    if (err_v.error.code != NUR_E_BUSY) return 0;
    if (strcmp(err_v.error.message, "resource locked") != 0) return 0;
    if (strcmp(NUR_ERROR_OF(err_v).message, "resource locked") != 0) return 0;

    return 1;
}

/* Test assertion helpers */
static int test_assert_helpers(void) {
    NurResult_i32 r = NUR_OK(i32, 100);
    NUR_ASSERT_OK(r);

    NurResult_i32 err = NUR_ERR(i32, NUR_ERROR(NUR_E_TIMEOUT, "timeout"));
    NUR_ASSERT_ERR(err, NUR_E_TIMEOUT);
    return 1;
}

/* Test ergonomic helpers */
static int test_ergonomic_helpers(void) {
    /* 1. NUR_OR_ELSE */
    NurResult_i32 r_ok = NUR_OK(i32, 42);
    NurResult_i32 r_err = NUR_ERR(i32, NUR_ERROR(NUR_E_NOT_FOUND, "missing"));
    if (NUR_OR_ELSE(r_ok, 10) != 42) return 0;
    if (NUR_OR_ELSE(r_err, 10) != 10) return 0;

    /* 2. NUR_IF_ERR */
    int if_err_triggered = 0;
    NUR_IF_ERR(r_err, err, {
        if (err.code == NUR_E_NOT_FOUND) if_err_triggered = 1;
    });
    if (!if_err_triggered) return 0;

    /* 3. NUR_VOID_IF_ERR & NUR_VOID_ERROR_OF */
    NurResult_void v_err = nur_err_void(NUR_ERROR(NUR_E_BUSY, "busy"));
    if (NUR_VOID_ERROR_OF(v_err).code != NUR_E_BUSY) return 0;
    int void_if_err_triggered = 0;
    NUR_VOID_IF_ERR(v_err, v_e, {
        if (v_e.code == NUR_E_BUSY) void_if_err_triggered = 1;
    });
    if (!void_if_err_triggered) return 0;

    /* 4. NUR_MATCH_AS */
    int match_ok_val = 0;
    NUR_MATCH_AS(int, r_ok, v, { match_ok_val = v; }, err, { (void)err; return 0; });
    if (match_ok_val != 42) return 0;

    int match_err_code = 0;
    NUR_MATCH_AS(int, r_err, v, { (void)v; return 0; }, err, { match_err_code = (int)err.code; });
    if (match_err_code != NUR_E_NOT_FOUND) return 0;

#if defined(NUR_GNU_EXTENSIONS) || defined(NUR_C23)
    /* 5. NUR_MATCH */
    int match_val = 0;
    NUR_MATCH(r_ok, val, { match_val = val; }, err, { (void)err; return 0; });
    if (match_val != 42) return 0;
#endif

    return 1;
}

static int test_custom_error_type_e(void) {
    CustomErr ce;
    ce.app_code = 404;
    ce.detail = "Not found in cache";
    NurResult_custom_e res_err = nur_err_custom_e(ce);
    if (NUR_IS_OK(res_err)) return 0;
    if (nur_error_custom_e(res_err).app_code != 404) return 0;
    if (strcmp(nur_error_custom_e(res_err).detail, "Not found in cache") != 0) return 0;

    NurResult_custom_e res_ok = nur_ok_custom_e(1234);
    if (!NUR_IS_OK(res_ok)) return 0;
    if (NUR_VALUE(res_ok) != 1234) return 0;
    if (nur_unwrap_or_custom_e(res_err, 999) != 999) return 0;
    return 1;
}

static int test_option_combinators_and_types(void) {
    NurOption_i32 opt_some = NUR_SOME(i32, 77);
    NurOption_i32 opt_none = NUR_NONE(i32);

    /* 1. NUR_OK_OR */
    NurResult_i32 r_from_some = NUR_OK_OR(i32, opt_some, NUR_ERROR(NUR_E_NOT_FOUND, "missing"));
    if (!NUR_IS_OK(r_from_some) || NUR_VALUE(r_from_some) != 77) return 0;

    NurResult_i32 r_from_none = NUR_OK_OR(i32, opt_none, NUR_ERROR(NUR_E_NOT_FOUND, "missing"));
    if (NUR_IS_OK(r_from_none) || NUR_ERROR_OF(r_from_none).code != NUR_E_NOT_FOUND) return 0;

    /* 2. NUR_UNWRAP_OPTION */
    int unwrapped = NUR_UNWRAP_OPTION(opt_some);
    if (unwrapped != 77) return 0;

    /* 3. Predefined extended types */
    NurResult_i8 r_i8 = NUR_OK(i8, (int8_t)-8);
    if (!NUR_IS_OK(r_i8) || NUR_VALUE(r_i8) != -8) return 0;
    NurResult_u8 r_u8 = NUR_OK(u8, (uint8_t)250);
    if (!NUR_IS_OK(r_u8) || NUR_VALUE(r_u8) != 250) return 0;
    NurResult_i16 r_i16 = NUR_OK(i16, (int16_t)-1600);
    if (!NUR_IS_OK(r_i16) || NUR_VALUE(r_i16) != -1600) return 0;
    NurResult_u16 r_u16 = NUR_OK(u16, (uint16_t)65000);
    if (!NUR_IS_OK(r_u16) || NUR_VALUE(r_u16) != 65000) return 0;
    NurResult_cstr_const r_cc = NUR_OK(cstr_const, "const string literal");
    if (!NUR_IS_OK(r_cc) || strcmp(NUR_VALUE(r_cc), "const string literal") != 0) return 0;

    return 1;
}

static int test_inline_message_and_formatting(void) {
    /* 1. Dynamic formatted error directly into inline buffer */
    nur_error_t err_fmt = NUR_ERROR_FMT(NUR_E_OUT_OF_RANGE, "port %d exceeds %d", 70000, 65535);
    if (err_fmt.code != NUR_E_OUT_OF_RANGE) return 0;
    if (strcmp(nur_error_message(&err_fmt), "port 70000 exceeds 65535") != 0) return 0;
    if (strcmp(err_fmt.inline_msg, "port 70000 exceeds 65535") != 0) return 0;

    /* 2. Dynamic copy into inline buffer */
    char ephemeral_buf[32];
    sprintf(ephemeral_buf, "id-%d", 42);
    nur_error_t err_copy = NUR_ERROR_COPY(NUR_E_INVALID_ARGUMENT, ephemeral_buf);
    if (strcmp(nur_error_message(&err_copy), "id-42") != 0) return 0;
    if (strcmp(err_copy.inline_msg, "id-42") != 0) return 0;

    return 1;
}

int main(void) {
    printf("=== nur.h Library Test Suite ===\n\n");
    
    TEST(result_i32_basic);
    TEST(error_propagation);
    TEST(predefined_types);
    TEST(error_categories);
    TEST(custom_result_type);
    TEST(error_utilities);
    TEST(option_types);
    TEST(macro_hygiene);
    /* Extended tests */
    TEST(nur_check);
    TEST(nur_malloc);
    TEST(error_errno);
    TEST(error_context);
    TEST(error_print);
    TEST(user_error_codes);
    TEST(unwrap_or);
    TEST(nur_ignore);
    /* Comprehensive enterprise tests */
    TEST(try_goto);
    TEST(try_auto);
    TEST(win32_mapping);
    TEST(errno_mapping);
    TEST(wrap_error_preserves_trace);
    TEST(error_format_null_safety);
    TEST(error_json);
    TEST(panic_handler);
    TEST(void_result_flat_layout);
    TEST(assert_helpers);
    TEST(ergonomic_helpers);
    TEST(custom_error_type_e);
    TEST(option_combinators_and_types);
    TEST(inline_message_and_formatting);
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("All tests PASSED! ✓\n");
        return 0;
    } else {
        printf("Some tests FAILED! ✗\n");
        return 1;
    }
}