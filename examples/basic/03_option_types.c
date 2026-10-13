/*
 * 03_option_types.c - Demonstration of NurOption types
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 03_option_types.c -o option_types
 * Run: ./option_types
 *
 * Demonstrates:
 * - NurOption types for nullable values
 * - NUR_IS_SOME and NUR_IS_NONE checks
 * - NUR_OPTION_VALUE for safe extraction
 * - NUR_OPTION_UNWRAP_OR for fallbacks
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <nur.h>

/**
 * Search for a value in an array, returning the index if found.
 * Returns NurOption_i32 - Some(index) if found, None if not found.
 */
static NurOption_i32 find_in_array(const int32_t *arr, size_t len, int32_t target) {
    if (arr == NULL) {
        return NUR_NONE(i32);
    }
    
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target) {
            /* Ensure safe conversion from size_t to int32_t */
            if (i > (size_t)INT32_MAX) {
                return NUR_NONE(i32); /* Index too large for int32_t */
            }
            return NUR_SOME(i32, (int32_t)i);
        }
    }
    
    return NUR_NONE(i32);
}

/**
 * Get environment variable value.
 * Returns NurOption_cstr - Some(value) if exists, None if not found.
 * Note: In a real implementation, this would use getenv()
 */
static NurOption_cstr get_env_var(const char *name) {
    if (name == NULL) {
        return NUR_NONE(cstr);
    }
    
    /* Simulate environment variables for demonstration */
    if (strcmp(name, "HOME") == 0) {
        return NUR_SOME(cstr, "/home/user");
    }
    if (strcmp(name, "PATH") == 0) {
        return NUR_SOME(cstr, "/usr/bin:/bin");
    }
    if (strcmp(name, "USER") == 0) {
        return NUR_SOME(cstr, "testuser");
    }
    
    return NUR_NONE(cstr);
}

/**
 * Find the maximum value in an array.
 * Returns NurOption_i32 - Some(max_value) if array is non-empty, None if empty.
 */
static NurOption_i32 find_max(const int32_t *arr, size_t len) {
    if (arr == NULL || len == 0) {
        return NUR_NONE(i32);
    }
    
    int32_t max_val = arr[0];
    for (size_t i = 1; i < len; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    return NUR_SOME(i32, max_val);
}

/**
 * Parse a positive integer from string.
 * Returns NurOption_i32 - Some(value) if valid positive integer, None otherwise.
 */
static NurOption_i32 parse_positive_int(const char *str) {
    if (str == NULL || str[0] == '\0') {
        return NUR_NONE(i32);
    }
    
    /* Simple validation - all characters must be digits */
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return NUR_NONE(i32);
        }
    }
    
    /* Convert to integer (simplified - no overflow checking) */
    int32_t result = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
        result = result * 10 + (str[i] - '0');
        /* Prevent overflow in demo */
        if (result > 100000) {
            return NUR_NONE(i32);
        }
    }
    
    return result > 0 ? NUR_SOME(i32, result) : NUR_NONE(i32);
}

int main(void) {
    printf("=== nur.h Option Types Demonstration ===\n\n");
    
    /* Example 1: Array searching */
    printf("1. Array searching with find_in_array:\n");
    int32_t numbers[] = {10, 20, 30, 40, 50};
    size_t len = sizeof(numbers) / sizeof(numbers[0]);
    
    NurOption_i32 index1 = find_in_array(numbers, len, 30);
    if (NUR_IS_SOME(index1)) {
        printf("   Found 30 at index: %d\n", NUR_OPTION_VALUE(index1));
    } else {
        printf("   Value 30 not found\n");
    }
    
    NurOption_i32 index2 = find_in_array(numbers, len, 99);
    if (NUR_IS_SOME(index2)) {
        printf("   Found 99 at index: %d\n", NUR_OPTION_VALUE(index2));
    } else {
        printf("   Value 99 not found\n");
    }
    
    /* Example 2: Environment variables */
    printf("\n2. Environment variable lookup:\n");
    const char *env_vars[] = {"HOME", "PATH", "NONEXISTENT", "USER"};
    
    for (size_t i = 0; i < 4; i++) {
        NurOption_cstr env_val = get_env_var(env_vars[i]);
        if (NUR_IS_SOME(env_val)) {
            printf("   %s = %s\n", env_vars[i], NUR_OPTION_VALUE(env_val));
        } else {
            printf("   %s = (not set)\n", env_vars[i]);
        }
    }
    
    /* Example 3: Using NUR_OPTION_UNWRAP_OR for fallbacks */
    printf("\n3. Using NUR_OPTION_UNWRAP_OR for fallbacks:\n");
    char *home_fallback = NUR_OPTION_UNWRAP_OR(get_env_var("HOME"), "/default/home");
    char *shell_fallback = NUR_OPTION_UNWRAP_OR(get_env_var("SHELL"), "/bin/sh");
    
    printf("   HOME (with fallback): %s\n", home_fallback);
    printf("   SHELL (with fallback): %s\n", shell_fallback);
    
    /* Example 4: Finding maximum in arrays */
    printf("\n4. Finding maximum values:\n");
    
    int32_t data1[] = {5, 2, 8, 1, 9, 3};
    NurOption_i32 max1 = find_max(data1, 6);
    printf("   Max of [5,2,8,1,9,3]: %d\n", NUR_OPTION_UNWRAP_OR(max1, -1));
    
    int32_t *empty_array = NULL;
    NurOption_i32 max2 = find_max(empty_array, 0);
    printf("   Max of empty array: %d\n", NUR_OPTION_UNWRAP_OR(max2, -1));
    
    /* Example 5: String parsing */
    printf("\n5. Parsing positive integers from strings:\n");
    const char *test_strings[] = {"123", "0", "-5", "abc", "999999999", ""};
    
    for (size_t i = 0; i < 6; i++) {
        NurOption_i32 parsed = parse_positive_int(test_strings[i]);
        if (NUR_IS_SOME(parsed)) {
            printf("   \"%s\" -> %d\n", test_strings[i], NUR_OPTION_VALUE(parsed));
        } else {
            printf("   \"%s\" -> (invalid)\n", test_strings[i]);
        }
    }
    
    /* Example 6: Chaining operations with options */
    printf("\n6. Chaining optional operations:\n");
    
    /* Find index of 40, then use that index to get another value */
    NurOption_i32 found_index = find_in_array(numbers, len, 40);
    if (NUR_IS_SOME(found_index)) {
        int32_t idx = NUR_OPTION_VALUE(found_index);
        printf("   Found 40 at index %d\n", idx);
        
        /* Use the index to access adjacent element if it exists */
        if (idx + 1 < (int32_t)len) {
            printf("   Next element is: %d\n", numbers[idx + 1]);
        } else {
            printf("   No next element (at end of array)\n");
        }
    } else {
        printf("   Could not find 40 in array\n");
    }
    
    printf("\n=== Option types demonstration completed ===\n");
    return 0;
}
