/*
 * 01_error_propagation.c - Error propagation through call chains
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 01_error_propagation.c -o error_propagation
 * Run: ./error_propagation (or error_propagation.exe on Windows)
 *
 * Demonstrates:
 * - Multi-layer error propagation with NUR_TRY
 * - Preserving original error source location through propagation
 * - Manual condition checking with early returns
 * - Building complex call chains with error handling
 * - Converting between different Result types
 * - Error context propagation
 *
 * Standards Compliance:
 * - ✅ Zero allocation
 * - ✅ C11/C23 compatible
 * - ✅ Proper NurResult usage
 * - ✅ Memory safe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../src/nur.h"

/* Custom result types for this example */
NUR_RESULT_TYPE(int, IntResult);
typedef NurResult_IntResult IntResult;

/**
 * Bottom layer: Input validation
 * Validates that input is a positive number in reasonable range.
 */
static IntResult validate_input(const char *input) {
    if (input == NULL) {
        return NUR_ERR(IntResult, NUR_ERROR(NUR_E_NULL_POINTER, "Input string cannot be NULL"));
    }
    
    if (strlen(input) == 0) {
        return NUR_ERR(IntResult, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Input string cannot be empty"));
    }
    
    if (strlen(input) > 10) {
        return NUR_ERR(IntResult, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Input string too long (max 10 chars)"));
    }
    
    /* Check all characters are digits */
    for (size_t i = 0; input[i] != '\0'; i++) {
        if (input[i] < '0' || input[i] > '9') {
            return NUR_ERR(IntResult, NUR_ERROR_CTX(NUR_E_PARSE, 
                                                     "Input contains non-digit characters", 
                                                     input));
        }
    }
    
    return NUR_OK(IntResult, 1); /* Validation passed - return dummy success value */
}

/**
 * Middle layer: Parse the validated input into a number
 * Depends on validate_input and propagates its errors using NUR_TRY_AS.
 */
static IntResult parse_value(const char *input) {
    /* If validate_input fails, propagate error immediately */
    NUR_TRY_AS(IntResult, validate_input(input));
    
    /* If we get here, validation passed - now do the actual parsing */
    long result = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        int digit = input[i] - '0';
        
        /* Check for overflow before multiplying */
        if (result > (INT_MAX - digit) / 10) {
            return NUR_ERR(IntResult, NUR_ERROR_CTX(NUR_E_OVERFLOW, 
                                                     "Number too large for int", 
                                                     input));
        }
        
        result = result * 10 + digit;
    }
    
    /* Additional validation: result must be positive */
    if (result <= 0) {
        return NUR_ERR(IntResult, NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, 
                                                 "Parsed value must be positive", 
                                                 input));
    }
    
    return NUR_OK(IntResult, (int)result);
}

/**
 * Top layer: Compute result using the parsed value
 * Depends on parse_value and propagates its errors.
 */
static IntResult compute_result(const char *input, int multiplier) {
    IntResult parse_result = parse_value(input);
    
    /* Check for parse error before using value */
    if (NUR_IS_ERR(parse_result)) {
        return parse_result; /* Propagate error */
    }
    
    int value = NUR_VALUE(parse_result);
    
    /* Additional validation on multiplier */
    if (multiplier <= 0) {
        return NUR_ERR(IntResult, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Multiplier must be positive"));
    }
    
    /* Check for overflow in multiplication */
    if (value > INT_MAX / multiplier) {
        return NUR_ERR(IntResult, NUR_ERROR(NUR_E_OVERFLOW, "Result would overflow"));
    }
    
    int result = value * multiplier;
    return NUR_OK(IntResult, result);
}

/**
 * Manual condition validation with early return.
 * Demonstrates explicit error creation without macros.
 */
static NurResult_void validate_configuration(int port, int timeout, const char *hostname) {
    /* Manual checks with early returns */
    if (port <= 0 || port > 65535) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Port must be in range [1, 65535]"));
    }
    
    if (timeout < 1 || timeout > 300) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Timeout must be in range [1, 300] seconds"));
    }
    
    if (hostname == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Hostname cannot be NULL"));
    }
    
    if (strlen(hostname) == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Hostname cannot be empty"));
    }
    
    if (strlen(hostname) > 253) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Hostname too long (max 253 chars)"));
    }
    
    /* All checks passed */
    return NUR_OK_VOID();
}

/**
 * Complex operation that combines multiple validations.
 * Demonstrates converting between different result types.
 */
static NurResult_void setup_server(const char *port_str, const char *timeout_str, const char *hostname) {
    /* Parse port */
    IntResult port_result = parse_value(port_str);
    if (NUR_IS_ERR(port_result)) {
        /* Convert IntResult error to NurResult_void */
        return NUR_ERR_VOID(NUR_ERROR_OF(port_result));
    }
    int port = NUR_VALUE(port_result);
    
    /* Parse timeout */
    IntResult timeout_result = parse_value(timeout_str);
    if (NUR_IS_ERR(timeout_result)) {
        return NUR_ERR_VOID(NUR_ERROR_OF(timeout_result));
    }
    int timeout = NUR_VALUE(timeout_result);
    
    /* Validate the complete configuration */
    NurResult_void validation = validate_configuration(port, timeout, hostname);
    if (NUR_IS_ERR(validation)) {
        return validation; /* Propagate void to void */
    }
    
    /* If we get here, everything is valid */
    printf("   Server configured: %s:%d (timeout: %ds)\n", hostname, port, timeout);
    return NUR_OK_VOID();
}

/**
 * Function that always fails to demonstrate error propagation source tracking.
 */
static IntResult always_fails(void) {
    return NUR_ERR(IntResult, NUR_ERROR(NUR_E_INTERNAL, "This function always fails for demonstration"));
}

/**
 * Function that propagates the always-failing function's error.
 * Demonstrates that source location is preserved through propagation.
 */
static NurResult_void propagate_failure(void) {
    IntResult result = always_fails();
    
    if (NUR_IS_ERR(result)) {
        /* Convert error from IntResult to NurResult_void */
        return NUR_ERR_VOID(NUR_ERROR_OF(result));
    }
    
    /* This line is never reached */
    return NUR_OK_VOID();
}

/**
 * Demonstrates error chaining - catching an error and adding context.
 */
static IntResult parse_with_context(const char *input, const char *field_name) {
    IntResult result = parse_value(input);
    
    if (NUR_IS_ERR(result)) {
        /* Add context to the error before propagating */
        nur_error_t original = NUR_ERROR_OF(result);
        nur_error_t enriched = original;
        enriched.context = field_name;
        return NUR_ERR(IntResult, enriched);
    }
    
    return result;
}

int main(void) {
    printf("=============================================================\n");
    printf("      nur.h Error Propagation Demonstration                \n");
    printf("=============================================================\n\n");
    
    /* Example 1: Successful propagation through all layers */
    printf("1. Successful three-layer operation:\n");
    IntResult result1 = compute_result("42", 3);
    if (NUR_IS_OK(result1)) {
        printf("   Success: 42 * 3 = %d\n", NUR_VALUE(result1));
    } else {
        printf("   Error: ");
        nur_error_print(&NUR_ERROR_OF(result1), stderr);
    }
    
    /* Example 2: Error at bottom layer (validation) */
    printf("\n2. Error propagation from validation layer:\n");
    IntResult result2 = compute_result("abc", 2);
    if (NUR_IS_OK(result2)) {
        printf("   Success: %d\n", NUR_VALUE(result2));
    } else {
        printf("   Error propagated from validation:\n      ");
        nur_error_print(&NUR_ERROR_OF(result2), stderr);
    }
    
    /* Example 3: Error at middle layer (parsing) */
    printf("\n3. Error propagation from parsing layer (overflow):\n");
    IntResult result3 = compute_result("999999999999", 1);
    if (NUR_IS_OK(result3)) {
        printf("   Success: %d\n", NUR_VALUE(result3));
    } else {
        printf("   Error propagated from parsing:\n      ");
        nur_error_print(&NUR_ERROR_OF(result3), stderr);
    }
    
    /* Example 4: Error at top layer (computation) */
    printf("\n4. Error at final computation layer:\n");
    IntResult result4 = compute_result("1000000", 3000);
    if (NUR_IS_OK(result4)) {
        printf("   Success: %d\n", NUR_VALUE(result4));
    } else {
        printf("   Error at computation layer:\n      ");
        nur_error_print(&NUR_ERROR_OF(result4), stderr);
    }
    
    /* Example 5: Manual validation demonstrations */
    printf("\n5. Configuration validation with manual checks:\n");
    
    struct {
        int port;
        int timeout;
        const char *hostname;
        const char *description;
    } configs[] = {
        {8080, 30, "example.com", "valid config"},
        {0, 30, "example.com", "invalid port (0)"},
        {8080, 500, "example.com", "timeout too large"},
        {8080, 30, NULL, "null hostname"},
        {8080, 30, "", "empty hostname"}
    };
    
    for (size_t i = 0; i < sizeof(configs) / sizeof(configs[0]); i++) {
        NurResult_void validation = validate_configuration(configs[i].port, 
                                                           configs[i].timeout, 
                                                           configs[i].hostname);
        if (NUR_IS_OK(validation)) {
            printf("   %s: Valid\n", configs[i].description);
        } else {
            printf("   %s:\n      ", configs[i].description);
            nur_error_print(&validation.error, stderr);
        }
    }
    
    /* Example 6: Complex server setup with multiple propagation points */
    printf("\n6. Complex server setup with error propagation:\n");
    
    struct {
        const char *port;
        const char *timeout;
        const char *hostname;
        const char *description;
    } server_configs[] = {
        {"8080", "30", "localhost", "valid configuration"},
        {"abc", "30", "localhost", "invalid port string"},
        {"8080", "xyz", "localhost", "invalid timeout string"},
        {"8080", "30", "", "empty hostname"},
    };
    
    for (size_t i = 0; i < sizeof(server_configs) / sizeof(server_configs[0]); i++) {
        printf("   Config %zu (%s):\n", i + 1, server_configs[i].description);
        NurResult_void setup = setup_server(server_configs[i].port, 
                                            server_configs[i].timeout, 
                                            server_configs[i].hostname);
        if (NUR_IS_ERR(setup)) {
            printf("      ");
            nur_error_print(&setup.error, stderr);
        }
    }
    
    /* Example 7: Source location preservation */
    printf("\n7. Source location preservation through propagation:\n");
    NurResult_void propagated = propagate_failure();
    if (NUR_IS_ERR(propagated)) {
        printf("   Error originated from always_fails() but propagated through propagate_failure():\n");
        printf("   ");
        nur_error_print(&propagated.error, stderr);
        printf("   Notice: Source location shows original error site (always_fails)\n");
    }
    
    /* Example 8: Error context enrichment */
    printf("\n8. Error context enrichment:\n");
    const char *test_inputs[] = {
        "123",
        "abc",
        "999999999999"
    };
    const char *field_names[] = {
        "user_id",
        "session_id", 
        "timestamp"
    };
    
    for (size_t i = 0; i < sizeof(test_inputs) / sizeof(test_inputs[0]); i++) {
        IntResult contextual = parse_with_context(test_inputs[i], field_names[i]);
        if (NUR_IS_OK(contextual)) {
            printf("   Parsed %s = %d\n", field_names[i], NUR_VALUE(contextual));
        } else {
            printf("   Failed to parse %s:\n      ", field_names[i]);
            nur_error_print(&NUR_ERROR_OF(contextual), stderr);
        }
    }
    
    /* Example 9: Unwrap or default */
    printf("\n9. Using NUR_UNWRAP_OR for default values:\n");
    IntResult maybe_valid = parse_value("42");
    IntResult definitely_invalid = parse_value("not_a_number");
    
    int value1 = NUR_UNWRAP_OR(maybe_valid, -1);
    int value2 = NUR_UNWRAP_OR(definitely_invalid, -1);
    
    printf("   Valid input \"42\" unwraps to: %d\n", value1);
    printf("   Invalid input unwraps to default: %d\n", value2);
    
    printf("\n=============================================================\n");
    printf("      Error propagation demonstration completed             \n");
    printf("=============================================================\n");
    
    return 0;
}
