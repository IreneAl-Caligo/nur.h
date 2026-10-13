/*
 * 05_error_context.c - Error context and source location tracking
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 05_error_context.c -o error_context
 * Run: ./error_context
 *
 * Demonstrates:
 * - NUR_ERROR_CTX for adding context to errors
 * - NUR_ERROR_ERRNO for capturing errno after system calls
 * - Source location tracking (file, line, function)
 * - Different error printing formats
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <nur.h>

/**
 * Simulate a file operation that sets errno.
 * In a real implementation, this would be fopen() or similar.
 */
static FILE *simulate_fopen(const char *filename, const char *mode) {
    (void)mode; /* Unused in simulation */
    
    /* Simulate different errno conditions based on filename */
    if (strcmp(filename, "nonexistent.txt") == 0) {
        errno = ENOENT; /* No such file or directory */
        return NULL;
    }
    if (strcmp(filename, "protected.txt") == 0) {
        errno = EACCES; /* Permission denied */
        return NULL;
    }
    if (strcmp(filename, "device.txt") == 0) {
        errno = EIO; /* I/O error */
        return NULL;
    }
    
    /* Success case - return a fake FILE pointer */
    return (FILE*)0x1234; /* Fake pointer for demo */
}

/**
 * Open a file with proper error handling using NUR_ERROR_ERRNO.
 */
NUR_NODISCARD
static NurResult_ptr open_file(const char *filename) {
    if (filename == NULL) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_NULL_POINTER, "Filename cannot be NULL"));
    }
    
    if (strlen(filename) == 0) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Filename cannot be empty"));
    }
    
    FILE *fp = simulate_fopen(filename, "r");
    if (fp == NULL) {
        /* Capture errno immediately after the failed system call */
        return NUR_ERR(ptr, NUR_ERROR_ERRNO("Failed to open file"));
    }
    
    return NUR_OK(ptr, fp);
}

/**
 * Open a file with additional context information.
 */
NUR_NODISCARD
static NurResult_ptr open_file_with_context(const char *filename, const char *operation) {
    if (filename == NULL) {
        return NUR_ERR(ptr, NUR_ERROR_CTX(NUR_E_NULL_POINTER, 
                                          "Filename cannot be NULL", 
                                          operation));
    }
    
    if (strlen(filename) == 0) {
        return NUR_ERR(ptr, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, 
                                          "Filename cannot be empty", 
                                          operation));
    }
    
    FILE *fp = simulate_fopen(filename, "r");
    if (fp == NULL) {
        return NUR_ERR(ptr, NUR_ERROR_ERRNO_CTX("Failed to open file", operation));
    }
    
    return NUR_OK(ptr, fp);
}

/**
 * Parse a number with detailed context about what's being parsed.
 */
NUR_NODISCARD
static NurResult_i32 parse_config_value(const char *str, const char *config_key) {
    if (str == NULL) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_NULL_POINTER, 
                                          "Configuration value string is NULL", 
                                          config_key));
    }
    
    if (strlen(str) == 0) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, 
                                          "Configuration value cannot be empty", 
                                          config_key));
    }
    
    /* Check for valid digits */
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_PARSE, 
                                              "Configuration value contains non-digit characters", 
                                              config_key));
        }
    }
    
    /* Simple conversion (no overflow checking for demo) */
    int32_t result = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
        int32_t digit = str[i] - '0';
        if (result > (INT32_MAX - digit) / 10) {
            return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OVERFLOW, 
                                              "Configuration value too large", 
                                              config_key));
        }
        result = result * 10 + digit;
    }
    
    return NUR_OK(i32, result);
}

/**
 * Validate a port number with context.
 */
NUR_NODISCARD
static NurResult_void validate_port(int32_t port, const char *service_name) {
    if (port <= 0) {
        return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, 
                                          "Port number must be positive", 
                                          service_name));
    }
    
    if (port > 65535) {
        return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, 
                                          "Port number exceeds maximum (65535)", 
                                          service_name));
    }
    
    if (port < 1024 && strcmp(service_name, "user-service") == 0) {
        return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_PERMISSION, 
                                          "User services cannot use privileged ports (<1024)", 
                                          service_name));
    }
    
    return NUR_OK_VOID();
}

/**
 * Demonstrate the difference between basic errors and contextual errors.
 */
static void demonstrate_error_comparison(void) {
    printf("=== Error Comparison: Basic vs Contextual ===\n\n");
    
    printf("1. Basic error (NUR_ERROR):\n");
    nur_error_t basic_error = NUR_ERROR(NUR_E_PARSE, "Invalid number format");
    nur_error_print(&basic_error, stdout);
    
    printf("\n2. Contextual error (NUR_ERROR_CTX):\n");
    nur_error_t ctx_error = NUR_ERROR_CTX(NUR_E_PARSE, "Invalid number format", "parsing timeout_seconds");
    nur_error_print(&ctx_error, stdout);
    
    printf("\n3. System error (NUR_ERROR_ERRNO):\n");
    errno = ENOENT; /* Simulate errno */
    nur_error_t sys_error = NUR_ERROR_ERRNO("File operation failed");
    nur_error_print(&sys_error, stdout);
    
    printf("\n4. System error with context (NUR_ERROR_ERRNO_CTX):\n");
    errno = EACCES; /* Simulate errno */
    nur_error_t sys_ctx_error = NUR_ERROR_ERRNO_CTX("File operation failed", "loading configuration");
    nur_error_print(&sys_ctx_error, stdout);
    
    printf("\n");
}

int main(void) {
    printf("=== nur.h Error Context Demonstration ===\n\n");
    
    /* Show different error formats first */
    demonstrate_error_comparison();
    
    /* Example 1: File operations with errno capture */
    printf("1. File operations with errno capture:\n");
    
    const char *test_files[] = {"good.txt", "nonexistent.txt", "protected.txt", "device.txt"};
    
    for (size_t i = 0; i < 4; i++) {
        NurResult_ptr file_result = open_file(test_files[i]);
        if (NUR_IS_OK(file_result)) {
            printf("   %s: Opened successfully\n", test_files[i]);
        } else {
            printf("   %s: ", test_files[i]);
            nur_error_eprint(&NUR_ERROR_OF(file_result));
        }
    }
    
    /* Example 2: File operations with context */
    printf("\n2. File operations with context:\n");
    
    for (size_t i = 0; i < 4; i++) {
        NurResult_ptr ctx_result = open_file_with_context(test_files[i], "configuration loading");
        if (NUR_IS_OK(ctx_result)) {
            printf("   %s: Opened successfully\n", test_files[i]);
        } else {
            printf("   %s: ", test_files[i]);
            nur_error_eprint(&NUR_ERROR_OF(ctx_result));
        }
    }
    
    /* Example 3: Configuration parsing with context */
    printf("\n3. Configuration parsing with context:\n");
    
    const char *config_values[] = {"8080", "abc", "", "999999999"};
    const char *config_keys[] = {"server_port", "timeout_seconds", "max_connections", "buffer_size"};
    
    for (size_t i = 0; i < 4; i++) {
        NurResult_i32 parse_result = parse_config_value(config_values[i], config_keys[i]);
        if (NUR_IS_OK(parse_result)) {
            printf("   %s='%s': %d\n", config_keys[i], config_values[i], NUR_VALUE(parse_result));
        } else {
            printf("   %s='%s': ", config_keys[i], config_values[i]);
            nur_error_eprint(&NUR_ERROR_OF(parse_result));
        }
    }
    
    /* Example 4: Port validation with context */
    printf("\n4. Port validation with different services:\n");
    
    struct {
        int32_t port;
        const char *service;
    } port_tests[] = {
        {8080, "web-server"},
        {80, "user-service"},    /* Should fail - privileged port */
        {-1, "api-server"},      /* Should fail - negative */
        {99999, "database"},     /* Should fail - too large */
        {3000, "user-service"}   /* Should succeed */
    };
    
    for (size_t i = 0; i < 5; i++) {
        NurResult_void validation = validate_port(port_tests[i].port, port_tests[i].service);
        if (NUR_IS_OK(validation)) {
            printf("   %s on port %d: Valid\n", port_tests[i].service, port_tests[i].port);
        } else {
            printf("   %s on port %d: ", port_tests[i].service, port_tests[i].port);
            nur_error_eprint(&validation.error);
        }
    }
    
    /* Example 5: Source location information */
    printf("\n5. Source location tracking:\n");
    printf("   All errors above include file, line, and function information\n");
    printf("   This is automatically captured by NUR_ERROR, NUR_ERROR_CTX, etc.\n");
    
    NurResult_void demo_error = NUR_ERR_VOID(NUR_ERROR(NUR_E_INTERNAL, "This error shows source location"));
    printf("   Demo error from line %d: ", __LINE__ - 1);
    nur_error_eprint(&demo_error.error);
    
    printf("\n=== Error context demonstration completed ===\n");
    return 0;
}
