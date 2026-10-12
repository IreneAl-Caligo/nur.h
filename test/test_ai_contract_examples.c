#include "../src/nur.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* AI contract: parse_port must use NurResult, check args, add context */
NUR_NODISCARD static NurResult_i32 parse_port(const char *s) {
    NUR_CHECK_AS(i32, s != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "port string is NULL"));
    int port = atoi(s);
    if (port <= 0 || port > 65535) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE,
                                           "port out of range", s));
    }
    return NUR_OK(i32, (int32_t)port);
}

static void test_parse_port_ok(void) {
    NurResult_i32 r = parse_port("8080");
    assert(NUR_IS_OK(r));
    assert(NUR_VALUE(r) == 8080);
    puts("test_parse_port_ok: PASS");
}

static void test_parse_port_null(void) {
    NurResult_i32 r = parse_port((const char *)0);
    assert(NUR_IS_ERR(r));
    assert(NUR_ERROR_OF(r).code == NUR_E_NULL_POINTER);
    puts("test_parse_port_null: PASS");
}

static void test_parse_port_out_of_range(void) {
    NurResult_i32 r = parse_port("99999");
    assert(NUR_IS_ERR(r));
    assert(NUR_ERROR_OF(r).code == NUR_E_OUT_OF_RANGE);
    puts("test_parse_port_out_of_range: PASS");
}

static void test_fallback_unwrap_or(void) {
    NurResult_i32 bad = parse_port("0");
    int port = NUR_UNWRAP_OR(bad, 80);
    assert(port == 80);
    puts("test_fallback_unwrap_or: PASS");
}

static void test_error_is_correct_code(void) {
    NurResult_i32 r = parse_port("99999");
    nur_error_t e = NUR_ERROR_OF(r);
    assert(NUR_ERROR_IS(e, NUR_E_OUT_OF_RANGE));
    puts("test_error_is_correct_code: PASS");
}

int main(void) {
    puts("=== test_ai_contract_examples ===");
    test_parse_port_ok();
    test_parse_port_null();
    test_parse_port_out_of_range();
    test_fallback_unwrap_or();
    test_error_is_correct_code();
    puts("All AI contract tests passed!");
    return 0;
}
