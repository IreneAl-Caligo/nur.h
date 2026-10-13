/*
 * Multi-translation unit (ODR) safety test — Translation Unit 2 (Main).
 */
#include "../src/nur.h"
#include <stdio.h>

extern NurResult_i32 tu1_compute(int x);
extern NurResult_void tu1_validate(int x);

int main(void) {
    puts("=== Multi-TU (ODR) Safety Test ===");

    /* Call function exported from TU1 */
    NurResult_i32 r1 = tu1_compute(21);
    if (!NUR_IS_OK(r1)) {
        fprintf(stderr, "TU1 compute failed!\n");
        return 1;
    }
    if (NUR_VALUE(r1) != 42) {
        fprintf(stderr, "TU1 compute returned unexpected value: %d\n", NUR_VALUE(r1));
        return 1;
    }

    NurResult_void v1 = tu1_validate(50);
    if (!NUR_IS_OK(v1)) {
        fprintf(stderr, "TU1 validate failed!\n");
        return 1;
    }

    /* Local operations in TU2 */
    NurResult_i32 r2 = NUR_OK(i32, 100);
    if (!NUR_IS_OK(r2) || NUR_VALUE(r2) != 100) {
        return 1;
    }

    puts("Multi-TU linking check PASSED with 0 duplicate symbols! [OK]");
    return 0;
}
