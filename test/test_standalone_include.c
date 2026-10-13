/*
 * Standalone include smoke test for nur.h.
 * Proves that nur.h is 100% self-contained and does not rely on
 * any pre-included standard headers.
 */
#include "../src/nur.h"

int main(void) {
    NurResult_i32 res = nur_ok_i32(42);
    if (!NUR_IS_OK(res)) {
        return 1;
    }
    return 0;
}
