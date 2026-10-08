/*
 * 02_error_codes.c - Comprehensive error code demonstration
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 02_error_codes.c -o error_codes
 * Run: ./error_codes
 *
 * Demonstrates:
 * - All major error code categories
 * - Error printing with nur_error_eprint
 * - Error code name resolution with nur_error_code_name
 * - User-defined error codes
 */

#include <stdio.h>
#include <string.h>
#include <nur.h>

/* User-defined error code */
#define MY_CUSTOM_ERROR (NUR_E_USER_BASE + 1)

/**
 * Function that demonstrates input validation errors
 */
NUR_NODISCARD
static NurResult_void validate_input(int32_t value) {
    if (value < 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Value cannot be negative"));
    }
    if (value > 1000) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Value exceeds maximum limit of 1000"));
    }
    return NUR_OK_VOID();
}

/**
 * Function that demonstrates parsing errors
 */
NUR_NODISCARD
static NurResult_i32 parse_number(const char *str) {
    if (str == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Input string is NULL"));
    }
    
    /* Simulate parsing failure */
    if (str[0] == 'x') {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "Cannot parse string starting with 'x'"));
    }
    
    /* Simulate format error */
    if (str[0] == '#') {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_FORMAT, "Invalid format: strings cannot start with '#'"));
    }
    
    return NUR_OK(i32, 42); /* Simplified parsing result */
}

/**
 * Function that demonstrates resource errors
 */
NUR_NODISCARD
static NurResult_ptr allocate_buffer(size_t size) {
    if (size > 1024 * 1024) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "Requested size exceeds memory limit"));
    }
    
    if (size == 777) { /* Simulate resource limit */
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_RESOURCE_LIMIT, "System resource limit reached"));
    }
    
    return NUR_OK(ptr, (void*)0x1000); /* Fake pointer for demo */
}

/**
 * Function that demonstrates I/O errors
 */
NUR_NODISCARD
static NurResult_void access_file(const char *filename) {
    if (filename == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Filename is NULL"));
    }
    
    /* Simulate various file access errors */
    if (strcmp(filename, "nonexistent.txt") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NOT_FOUND, "File does not exist"));
    }
    
    if (strcmp(filename, "protected.txt") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_PERMISSION, "Access denied: insufficient permissions"));
    }
    
    if (strcmp(filename, "existing.txt") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_ALREADY_EXISTS, "File already exists"));
    }
    
    return NUR_OK_VOID();
}

/**
 * Function that demonstrates network/async errors
 */
NUR_NODISCARD
static NurResult_void connect_service(const char *service) {
    if (service == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Service name is NULL"));
    }
    
    if (strcmp(service, "slow-service") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_TIMEOUT, "Connection timed out after 30 seconds"));
    }
    
    if (strcmp(service, "cancelled-service") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_CANCELLED, "Operation was cancelled by user"));
    }
    
    return NUR_OK_VOID();
}

/**
 * Function that demonstrates system errors
 */
NUR_NODISCARD
static NurResult_void check_system_feature(const char *feature) {
    if (feature == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Feature name is NULL"));
    }
    
    if (strcmp(feature, "advanced-crypto") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_UNSUPPORTED, "Advanced crypto not supported on this platform"));
    }
    
    if (strcmp(feature, "internal-debug") == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INTERNAL, "Internal system error occurred"));
    }
    
    return NUR_OK_VOID();
}

/**
 * Function that demonstrates user-defined errors
 */
NUR_NODISCARD
static NurResult_void custom_operation(int32_t mode) {
    if (mode == 999) {
        return NUR_ERR_VOID(NUR_ERROR(MY_CUSTOM_ERROR, "Custom error: special mode 999 not implemented"));
    }
    
    return NUR_OK_VOID();
}

int main(void) {
    printf("=== nur.h Error Codes Demonstration ===\n\n");
    
    /* Test each error category */
    
    printf("1. Input/Validation Errors:\n");
    NurResult_void r1 = validate_input(-5);
    if (NUR_IS_ERR(r1)) {
        printf("   Code: %s\n", nur_error_code_name(r1.error.code));
        nur_error_eprint(&r1.error);
    }
    
    NurResult_void r2 = validate_input(2000);
    if (NUR_IS_ERR(r2)) {
        printf("   Code: %s\n", nur_error_code_name(r2.error.code));
        nur_error_eprint(&r2.error);
    }
    
    printf("\n2. Data/Parsing Errors:\n");
    NurResult_i32 r3 = parse_number("x123");
    if (NUR_IS_ERR(r3)) {
        printf("   Code: %s\n", nur_error_code_name(NUR_ERROR_OF(r3).code));
        nur_error_eprint(&NUR_ERROR_OF(r3));
    }
    
    NurResult_i32 r4 = parse_number("#invalid");
    if (NUR_IS_ERR(r4)) {
        printf("   Code: %s\n", nur_error_code_name(NUR_ERROR_OF(r4).code));
        nur_error_eprint(&NUR_ERROR_OF(r4));
    }
    
    printf("\n3. Resource Management Errors:\n");
    NurResult_ptr r5 = allocate_buffer(2 * 1024 * 1024);
    if (NUR_IS_ERR(r5)) {
        printf("   Code: %s\n", nur_error_code_name(NUR_ERROR_OF(r5).code));
        nur_error_eprint(&NUR_ERROR_OF(r5));
    }
    
    NurResult_ptr r6 = allocate_buffer(777);
    if (NUR_IS_ERR(r6)) {
        printf("   Code: %s\n", nur_error_code_name(NUR_ERROR_OF(r6).code));
        nur_error_eprint(&NUR_ERROR_OF(r6));
    }
    
    printf("\n4. I/O Errors:\n");
    NurResult_void r7 = access_file("nonexistent.txt");
    if (NUR_IS_ERR(r7)) {
        printf("   Code: %s\n", nur_error_code_name(r7.error.code));
        nur_error_eprint(&r7.error);
    }
    
    NurResult_void r8 = access_file("protected.txt");
    if (NUR_IS_ERR(r8)) {
        printf("   Code: %s\n", nur_error_code_name(r8.error.code));
        nur_error_eprint(&r8.error);
    }
    
    printf("\n5. Network/Async Errors:\n");
    NurResult_void r9 = connect_service("slow-service");
    if (NUR_IS_ERR(r9)) {
        printf("   Code: %s\n", nur_error_code_name(r9.error.code));
        nur_error_eprint(&r9.error);
    }
    
    printf("\n6. System Errors:\n");
    NurResult_void r10 = check_system_feature("advanced-crypto");
    if (NUR_IS_ERR(r10)) {
        printf("   Code: %s\n", nur_error_code_name(r10.error.code));
        nur_error_eprint(&r10.error);
    }
    
    printf("\n7. User-Defined Errors:\n");
    NurResult_void r11 = custom_operation(999);
    if (NUR_IS_ERR(r11)) {
        printf("   Code: %s (numeric: %d)\n", nur_error_code_name(r11.error.code), r11.error.code);
        nur_error_eprint(&r11.error);
    }
    
    printf("\n=== All error categories demonstrated ===\n");
    return 0;
}