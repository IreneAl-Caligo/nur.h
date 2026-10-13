/*
 * Multi-translation unit (ODR) safety test — Translation Unit 1.
 */
#include "../src/nur.h"

NUR_NODISCARD NurResult_i32 tu1_compute(int x) {
    if (x < 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "x must be non-negative"));
    }
    return NUR_OK(i32, x * 2);
}

NUR_NODISCARD NurResult_void tu1_validate(int x) {
    if (x > 1000) {
        return nur_err_void(NUR_ERROR(NUR_E_OUT_OF_RANGE, "x exceeds threshold"));
    }
    return nur_ok_void();
}
