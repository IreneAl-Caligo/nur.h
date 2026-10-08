# Getting Started with nur.h

Welcome to nur.h! This tutorial will walk you through the basics of using Rust-style error handling in C.

## What is nur.h?

nur.h is a single-header C library that brings Rust's `Result<T,E>` type to C, providing:
- **Safe error handling** - Forces you to check for errors before using values
- **Zero allocation** - No hidden malloc/free calls in the core library  
- **Header-only** - Just include the header, no linking required
- **Multi-standard** - Works with C99, C11, C17, and C23

## Installation

### Download and Include

1. Download `nur.h` from the repository
2. Place it in your project's include directory
3. Include it in your C files:

```c
#include "nur.h"
```

### System Installation (Linux/macOS)

```bash
# Clone and install
git clone https://github.com/yourusername/nur.h.git
cd nur.h
make install
```

### Windows Installation

Copy `src/nur.h` to your project's include directory.

## Your First nur.h Program

Let's start with a simple example that shows the basics:

```c
#include <stdio.h>
#include "nur.h"

// A function that can succeed or fail
NurResult_i32 divide(int a, int b) {
    if (b == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Division by zero"));
    }
    return NUR_OK(i32, a / b);
}

int main() {
    // Successful operation
    NurResult_i32 result = divide(10, 2);
    
    if (NUR_IS_OK(result)) {
        printf("10 / 2 = %d\\n", NUR_UNWRAP(result));
    } else {
        printf("Error: %s\\n", result.u.error.message);
    }
    
    // Error case
    result = divide(10, 0);
    
    if (NUR_IS_OK(result)) {
        printf("10 / 0 = %d\\n", NUR_UNWRAP(result));
    } else {
        printf("Error: %s\\n", result.u.error.message);
        printf("Error code: %d\\n", result.u.error.code);
        printf("Location: %s:%d in %s()\\n", 
               result.u.error.file, result.u.error.line, result.u.error.func);
    }
    
    return 0;
}
```

**Output:**
```
10 / 2 = 5
Error: Division by zero
Error code: 1
Location: example.c:8 in divide()
```

## Core Concepts

### 1. Result Types

Result types represent operations that can succeed (with a value) or fail (with an error). They use a tagged union structure:

```c
typedef struct NurResult_i32 {
    bool ok;                    // true = success, false = error
    union {
        int32_t     value;      // Valid when ok == true
        nur_error_t error;      // Valid when ok == false
    } u;
} NurResult_i32;
```

**Predefined Result Types:**
```c
NurResult_i32   // Result containing an int32_t
NurResult_i64   // Result containing an int64_t
NurResult_f64   // Result containing a double  
NurResult_cstr  // Result containing a char*
NurResult_ptr   // Result containing a void*
NurResult_void  // Result with no value (just success/failure)
```

### 2. Creating Results

**Success (requires type suffix):**
```c
NurResult_i32 success = NUR_OK(i32, 42);           // Success with value 42
NurResult_void success = NUR_OK_VOID();             // Success with no value
```

**Failure (requires type suffix):**
```c
// Create error with NUR_ERROR macro, then wrap in NUR_ERR
NurResult_i32 failure = NUR_ERR(i32, NUR_ERROR(NUR_E_NOT_FOUND, "File not found"));

// For void results
NurResult_void failure = NUR_ERR_VOID(NUR_ERROR(NUR_E_PERMISSION_DENIED, "Access denied"));
```

### 3. Checking Results

```c
NurResult_i32 result = some_function();

// Check for success
if (NUR_IS_OK(result)) {
    int value = NUR_UNWRAP(result);     // Extract the value from union
    printf("Success: %d\\n", value);
} else {
    // Access error through union
    printf("Error: %s\\n", result.u.error.message);
}

// Alternative: check for error first
if (NUR_IS_ERR(result)) {
    printf("Something went wrong: %s\\n", result.u.error.message);
    return -1;
}

// Use the value (only after checking!)
int value = NUR_UNWRAP(result);
```

## Error Propagation with NUR_TRY

One of the most powerful features is automatic error propagation:

```c
NurResult_i32 step1() {
    // This might fail
    return some_operation();
}

NurResult_i32 step2(int value) {
    // This might also fail  
    return another_operation(value);
}

// NUR_TRY automatically propagates errors
NurResult_i32 complex_operation() {
    // If step1 fails, the error is automatically returned
    int value1 = NUR_TRY(step1());
    
    // If step2 fails, its error is automatically returned
    int value2 = NUR_TRY(step2(value1));
    
    // If we get here, both steps succeeded
    return NUR_OK(i32, value1 + value2);
}
```

**Important:** `NUR_TRY` can only be used in functions that return a compatible Result type.

## Error Codes and Context

nur.h provides predefined error codes organized by category:

```c
// Input/Validation (1-9)
NUR_E_INVALID_ARGUMENT  // Invalid function argument
NUR_E_NULL_POINTER      // Unexpected null pointer
NUR_E_OUT_OF_RANGE      // Value outside valid range

// Data/Parsing (10-19)  
NUR_E_PARSE            // Parsing failed
NUR_E_FORMAT           // Invalid format

// Resources (20-29)
NUR_E_OUT_OF_MEMORY    // Memory allocation failed

// I/O (30-39)
NUR_E_NOT_FOUND        // File/resource not found
NUR_E_PERMISSION_DENIED // Permission denied
NUR_E_IO               // I/O operation failed
```

**Error Structure:**
```c
typedef struct {
    nur_error_code_t code;      // Error code (NUR_E_*)
    int              sys;       // System errno (for I/O errors)
    const char      *message;   // Human-readable message
    const char      *context;   // Additional context (optional)
    const char      *file;      // Source file (__FILE__)
    const char      *func;      // Function name (__func__)
    unsigned int     line;      // Line number (__LINE__)
} nur_error_t;
```

## Practical Example: File Processing

Here's a realistic example showing error handling patterns:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nur.h"

// Read file contents into a string
NurResult_cstr read_file(const char* filename) {
    if (!filename) {
        return NUR_ERR(cstr, NUR_ERROR(NUR_E_NULL_POINTER, "Filename cannot be null"));
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NUR_ERR(cstr, NUR_ERROR_ERRNO("Could not open file"));
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NUR_ERR(cstr, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "Could not allocate buffer"));
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, size, file);
    fclose(file);
    
    if (bytes_read != (size_t)size) {
        free(buffer);
        return NUR_ERR(cstr, NUR_ERROR(NUR_E_IO, "Could not read entire file"));
    }
    
    buffer[size] = '\\0';
    return NUR_OK(cstr, buffer);
}

// Count lines in text
NurResult_i32 count_lines(const char* text) {
    if (!text) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Text cannot be null"));
    }
    
    int lines = 1;
    for (const char* p = text; *p; p++) {
        if (*p == '\\n') {
            lines++;
        }
    }
    
    return NUR_OK(i32, lines);
}

// Process file and return line count  
NurResult_i32 process_file(const char* filename) {
    // Read file - if it fails, error propagates automatically
    char* content = NUR_TRY(read_file(filename));
    
    // Count lines - if it fails, error propagates automatically  
    int lines = NUR_TRY(count_lines(content));
    
    // Clean up
    free(content);
    
    return NUR_OK(i32, lines);
}

int main() {
    const char* filename = "example.txt";
    
    NurResult_i32 result = process_file(filename);
    
    if (NUR_IS_OK(result)) {
        printf("File '%s' has %d lines\\n", filename, NUR_UNWRAP(result));
    } else {
        printf("Failed to process '%s': %s\\n", filename, result.u.error.message);
        printf("Error occurred at %s:%d in %s()\\n", 
               result.u.error.file, result.u.error.line, result.u.error.func);
    }
    
    return 0;
}
```

## Best Practices

### 1. Always Check Results
```c
// ❌ Bad: ignoring potential errors
int value = NUR_UNWRAP(risky_operation());

// ✅ Good: check before unwrapping
NurResult_i32 result = risky_operation();
if (NUR_IS_ERR(result)) {
    // Handle error
    return NUR_ERR(i32, result.u.error);  // Propagate
}
int value = NUR_UNWRAP(result);
```

### 2. Use NUR_TRY for Clean Error Propagation
```c
// ❌ Bad: manual error checking
NurResult_i32 result1 = step1();
if (NUR_IS_ERR(result1)) return result1;
int val1 = NUR_UNWRAP(result1);

// ✅ Good: automatic propagation
int val1 = NUR_TRY(step1());
int val2 = NUR_TRY(step2(val1));
```

### 3. Provide Meaningful Error Messages
```c
// ❌ Bad: generic error
return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Bad argument"));

// ✅ Good: specific context
return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Buffer size must be positive"));
```

## Compilation

nur.h works with all modern C compilers:

```bash
# GCC
gcc -std=c11 -Wall -Wextra -I. your_program.c -o your_program

# Clang  
clang -std=c11 -Wall -Wextra -I. your_program.c -o your_program

# MSVC (Windows)
cl /std:c11 /I. your_program.c
```

## Key Differences from Original Tutorial

1. **Type Suffixes**: All `NUR_OK` and `NUR_ERR` calls require type suffixes (`i32`, `cstr`, etc.)
2. **Union Access**: Error fields accessed via `result.u.error.field`, not `result.error.field`
3. **Error Creation**: Use `NUR_ERROR()` macro to create errors, then wrap with `NUR_ERR()`
4. **Complete Context**: Error objects capture file, function, and line information automatically

## Next Steps

- [Error Propagation Tutorial](error_propagation.md) - Advanced error handling patterns
- [Custom Types Tutorial](custom_types.md) - Create Result types for your structs  
- [API Reference](../api/) - Complete function and macro reference

This corrected version now accurately reflects the actual nur.h API implementation.