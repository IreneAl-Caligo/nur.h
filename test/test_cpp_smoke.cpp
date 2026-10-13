/*
 * C++ Compatibility & Integration Test for nur.h.
 * Verifies standard compliance across C++11, C++14, C++17, C++20, and C++23.
 */
#include "../src/nur.h"
#include <iostream>
#include <string>
#include <cassert>

namespace {

NurResult_i32 divide(int numerator, int denominator) {
    if (denominator == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "division by zero in C++"));
    }
    return NUR_OK(i32, numerator / denominator);
}

NurResult_void validate_port(int port) {
    if (port <= 0 || port > 65535) {
        return nur_err_void(NUR_ERROR(NUR_E_OUT_OF_RANGE, "port out of range"));
    }
    return nur_ok_void();
}

} // namespace

int main() {
    std::cout << "=== C++ Compatibility Test ===\n";

    // 1. Success case
    NurResult_i32 ok_res = divide(100, 4);
    assert(NUR_IS_OK(ok_res));
    assert(NUR_VALUE(ok_res) == 25);

    // 2. Error case
    NurResult_i32 err_res = divide(100, 0);
    assert(NUR_IS_ERR(err_res));
    assert(NUR_ERROR_OF(err_res).code == NUR_E_INVALID_ARGUMENT);

    // 3. Fallback unwrap
    int val = NUR_UNWRAP_OR(err_res, -1);
    assert(val == -1);

    // 4. Void result
    NurResult_void v_ok = validate_port(8080);
    assert(NUR_IS_OK(v_ok));

    NurResult_void v_err = validate_port(99999);
    assert(NUR_IS_ERR(v_err));

    // 5. Option types
    NurOption_i32 opt_some = NUR_SOME(i32, 999);
    assert(NUR_IS_SOME(opt_some));
    assert(opt_some.value == 999);

    NurOption_i32 opt_none = NUR_NONE(i32);
    assert(NUR_IS_NONE(opt_none));

    // 6. C++11 Universal single-token constructor & overloaded NUR_OK
    NurResult_i32 r_h = NUR_H(42);
    assert(NUR_IS_OK(r_h) && NUR_VALUE(r_h) == 42);

    NurResult_cstr_const r_str = NUR_H("hello cpp");
    assert(NUR_IS_OK(r_str) && std::string(NUR_VALUE(r_str)) == "hello cpp");

    NurResult_i32 r_ok_single = NUR_OK(777);
    assert(NUR_IS_OK(r_ok_single) && NUR_VALUE(r_ok_single) == 777);

    // 7. Modern C++ namespace nur { ... } usage
    nur::Result_i32 r_ns = nur::ok(1234);
    assert(nur::is_ok(r_ns));
    assert(nur::value(r_ns) == 1234);

    nur::Result_void v_ns = nur::ok();
    assert(nur::is_ok(v_ns));

    // 8. Built-in drop-in helpers in C++
    nur::Result_f64 r_sqrt = nur::sqrt(49.0);
    assert(nur::is_ok(r_sqrt) && nur::value(r_sqrt) == 7.0);

    nur::Result_usize r_len = nur::strlen("hello cpp");
    assert(nur::is_ok(r_len) && nur::value(r_len) == 9);

    std::cout << "All C++ compatibility tests PASSED! [OK]\n";
    return 0;
}
