#include "../src/nur.h"
#include <assert.h>
#include <stdio.h>

static int32_t square(int32_t v) { return v * v; }

#if defined(NUR_GNU_EXTENSIONS)
static NurResult_i32 square_result(int32_t v) {
    return NUR_OK(i32, v * v);
}
#endif

static void test_is_ok_err(void) {
    NurResult_i32 ok = NUR_OK(i32, 42);
    NurResult_i32 err = NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "bad"));
    assert(NUR_IS_OK(ok));
    assert(NUR_IS_ERR(err));
    assert(!NUR_IS_ERR(ok));
    puts("test_is_ok_err: PASS");
}

static void test_unwrap_or_ok(void) {
    NurResult_i32 r = NUR_OK(i32, 7);
    int val = NUR_UNWRAP_OR(r, 99);
    assert(val == 7);
    puts("test_unwrap_or_ok: PASS");
}

static void test_unwrap_or_err(void) {
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "bad"));
    int val = NUR_UNWRAP_OR(r, 99);
    assert(val == 99);
    puts("test_unwrap_or_err: PASS");
}

static void test_value_or(void) {
    NurResult_i32 r = NUR_OK(i32, 5);
    int val = NUR_VALUE_OR(r, 0);
    assert(val == 5);
    puts("test_value_or: PASS");
}

static void test_error_is(void) {
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(NUR_E_NOT_FOUND, "missing"));
    nur_error_t e = NUR_ERROR_OF(r);
    assert(NUR_ERROR_IS(e, NUR_E_NOT_FOUND));
    assert(!NUR_ERROR_IS(e, NUR_E_PARSE));
    puts("test_error_is: PASS");
}

static void test_error_wrap(void) {
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "bad data"));
    nur_error_t wrapped = NUR_ERROR_WRAP(NUR_ERROR_OF(r), "while loading config");
    assert(wrapped.code == NUR_E_PARSE);
    assert(wrapped.context != (const char *)0);
    puts("test_error_wrap: PASS");
}

#if defined(NUR_GNU_EXTENSIONS)
static void test_map_ok(void) {
    NurResult_i32 r = NUR_OK(i32, 4);
    NurResult_i32 mapped = NUR_MAP(r, square);
    assert(NUR_IS_OK(mapped));
    assert(NUR_VALUE(mapped) == 16);
    puts("test_map_ok: PASS");
}

static void test_map_err(void) {
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "bad"));
    NurResult_i32 mapped = NUR_MAP(r, square);
    assert(NUR_IS_ERR(mapped));
    assert(NUR_ERROR_OF(mapped).code == NUR_E_PARSE);
    puts("test_map_err: PASS");
}

static void test_and_then_ok(void) {
    NurResult_i32 r = NUR_OK(i32, 5);
    NurResult_i32 r2 = NUR_AND_THEN(r, square_result);
    assert(NUR_IS_OK(r2));
    assert(NUR_VALUE(r2) == 25);
    puts("test_and_then_ok: PASS");
}

static void test_and_then_err(void) {
    NurResult_i32 r = NUR_ERR(i32, NUR_ERROR(NUR_E_IO, "io fail"));
    NurResult_i32 r2 = NUR_AND_THEN(r, square_result);
    assert(NUR_IS_ERR(r2));
    assert(NUR_ERROR_OF(r2).code == NUR_E_IO);
    puts("test_and_then_err: PASS");
}
#endif /* NUR_GNU_EXTENSIONS */

int main(void) {
    puts("=== test_result_helpers ===");
    test_is_ok_err();
    test_unwrap_or_ok();
    test_unwrap_or_err();
    test_value_or();
    test_error_is();
    test_error_wrap();
#if defined(NUR_GNU_EXTENSIONS)
    test_map_ok();
    test_map_err();
    test_and_then_ok();
    test_and_then_err();
#endif
    puts("All tests passed!");
    return 0;
}
