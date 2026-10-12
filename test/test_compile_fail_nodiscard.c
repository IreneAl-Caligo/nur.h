#include "nur.h"

/*
 * Compile-fail verification test:
 * This file is INTENTIONALLY designed to fail compilation when compiled with
 * -Werror=unused-result (or -Werror in C23 mode).
 * It proves that discarding a NurResult value triggers compiler diagnostic.
 */

NUR_NODISCARD static NurResult_i32 may_fail_operation(int x) {
    if (x < 0) return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "negative"));
    return NUR_OK(i32, x * 2);
}

int main(void) {
    /* Bug: Intentionally ignoring return value */
    may_fail_operation(42);
    return 0;
}
