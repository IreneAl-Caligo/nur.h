/*
 * 01_hello_result.c - Introduction to NurResult types
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 01_hello_result.c -o hello_result
 * Run: ./hello_result
 *
 * Demonstrates:
 * - Creating success and error results
 * - Checking result status with NUR_IS_OK/NUR_IS_ERR
 * - Safe value extraction with NUR_VALUE
 * - Fallback values with NUR_UNWRAP_OR
 */

#include <stdio.h>
#include <nur.h>

/**
 * Safe division function that returns a result instead of crashing.
 * Returns NurResult_i32 containing either the quotient or an error.
 */
NUR_NODISCARD 
static NurResult_i32 safe_divide(int32_t a, int32_t b) {
    if (b == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Division by zero"));
    }
    
    return NUR_OK(i32, a / b);
}

int main(void) {
    printf("=== nur.h Basic Result Example ===\n\n");
    
    /* Example 1: Successful division */
    printf("1. Testing safe_divide(10, 2):\n");
    NurResult_i32 result1 = safe_divide(10, 2);
    
    if (NUR_IS_OK(result1)) {
        printf("   Success! Result: %d\n", NUR_VALUE(result1));
    } else {
        printf("   Error: %s\n", NUR_ERROR_OF(result1).message);
    }
    
    /* Example 2: Division by zero */
    printf("\n2. Testing safe_divide(42, 0):\n");
    NurResult_i32 result2 = safe_divide(42, 0);
    
    if (NUR_IS_OK(result2)) {
        printf("   Success! Result: %d\n", NUR_VALUE(result2));
    } else {
        printf("   Error: %s\n", NUR_ERROR_OF(result2).message);
        printf("   Error code: %s\n", nur_error_code_name(NUR_ERROR_OF(result2).code));
    }
    
    /* Example 3: Using NUR_UNWRAP_OR for fallbacks */
    printf("\n3. Using NUR_UNWRAP_OR for fallback values:\n");
    int32_t safe_result1 = NUR_UNWRAP_OR(safe_divide(20, 4), -1);
    int32_t safe_result2 = NUR_UNWRAP_OR(safe_divide(20, 0), -1);
    
    printf("   safe_divide(20, 4) with fallback -1: %d\n", safe_result1);
    printf("   safe_divide(20, 0) with fallback -1: %d\n", safe_result2);
    
    /* Example 4: Chaining operations */
    printf("\n4. Chaining multiple operations:\n");
    NurResult_i32 a = safe_divide(100, 5);  /* 20 */
    NurResult_i32 b = safe_divide(60, 3);   /* 20 */
    
    if (NUR_IS_OK(a) && NUR_IS_OK(b)) {
        NurResult_i32 final = safe_divide(NUR_VALUE(a), NUR_VALUE(b));
        if (NUR_IS_OK(final)) {
            printf("   (100/5) / (60/3) = %d\n", NUR_VALUE(final));
        } else {
            printf("   Final division failed: %s\n", NUR_ERROR_OF(final).message);
        }
    } else {
        printf("   One of the preliminary operations failed\n");
    }
    
    printf("\n=== Example completed successfully ===\n");
    return 0;
}