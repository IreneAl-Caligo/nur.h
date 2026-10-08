# nur.h Examples

This directory contains comprehensive examples demonstrating the nur.h library's error handling capabilities. All examples are designed to compile and run independently with a single GCC command.

## Directory Structure

```
examples/
├── basic/           # Fundamental concepts and API introduction
├── advanced/        # Complex patterns and real-world techniques  
├── real-world/      # Complete applications and practical use cases
└── README.md        # This file
```

## Compilation Requirements

- **C Standard**: C11 minimum (examples are compatible with C11, C17, and C23)
- **Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **Flags**: `-std=c11 -Wall -Wextra -Wpedantic` recommended

## Basic Examples

### 01_hello_result.c
**Purpose**: Introduction to NurResult types  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src basic/01_hello_result.c -o hello_result`  
**Demonstrates**: 
- Creating success and error results
- Safe value extraction with NUR_IS_OK/NUR_VALUE  
- Fallback values with NUR_UNWRAP_OR
- Basic error handling patterns

### 02_error_codes.c  
**Purpose**: Comprehensive error code taxonomy  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src basic/02_error_codes.c -o error_codes`  
**Demonstrates**:
- All major error categories (input, data, resources, I/O, network, system)
- Error printing with nur_error_eprint
- Error code name resolution
- User-defined error codes

### 03_option_types.c
**Purpose**: NurOption types for nullable values  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src basic/03_option_types.c -o option_types`  
**Demonstrates**:
- NUR_SOME and NUR_NONE constructors
- Safe optional value handling
- NUR_OPTION_UNWRAP_OR for fallbacks  
- Array searching and environment variable simulation

### 04_custom_types.c
**Purpose**: Custom data structures with NurResult  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src basic/04_custom_types.c -o custom_types`  
**Demonstrates**:
- NUR_RESULT_TYPE macro usage
- Validation functions for custom structs
- Error propagation with custom types
- Configuration parsing patterns

### 05_error_context.c
**Purpose**: Error context and source location tracking  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src basic/05_error_context.c -o error_context`  
**Demonstrates**:
- NUR_ERROR_CTX for adding operation context
- NUR_ERROR_ERRNO for system error capture
- Source location tracking (file, line, function)
- Different error printing formats

## Advanced Examples

### 01_error_propagation.c
**Purpose**: Multi-layer error propagation  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src advanced/01_error_propagation.c -o error_propagation`  
**Demonstrates**:
- NUR_TRY_TO for portable error propagation
- NUR_CHECK for condition validation  
- Three-layer call stacks with error preservation
- Complex validation pipelines

### 02_file_io.c
**Purpose**: File operations with comprehensive error handling  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src advanced/02_file_io.c -o file_io`  
**Demonstrates**:
- File I/O returning NurResult types
- Resource cleanup on error paths
- Chained file operations
- Cross-platform temporary file handling

### 03_allocation.c ⚠️
**Purpose**: Dynamic memory allocation adapters  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src advanced/03_allocation.c -o allocation`  
**⚠️ Note**: This example demonstrates **optional allocation adapters** that violate nur.h's core zero-allocation principle. It shows nur_malloc/nur_realloc wrappers but should be used cautiously in production.  
**Demonstrates**:
- nur_malloc and nur_realloc wrappers
- Dynamic string buffer implementation
- Out-of-memory simulation and handling

### 04_parsing.c
**Purpose**: Zero-allocation CSV parsing  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src advanced/04_parsing.c -o parsing`  
**Demonstrates**:
- Complex parsing without dynamic allocation
- Buffer size validation and bounds checking
- Field extraction with comprehensive error handling
- CSV record validation and processing

## Real-World Examples

### 01_config_parser.c
**Purpose**: Complete INI-style configuration parser  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src real-world/01_config_parser.c -o config_parser`  
**Demonstrates**:
- File I/O combined with string processing
- Comment and whitespace handling
- NurOption for missing configuration values
- Integer parsing with validation
- Line-by-line error reporting

### 02_string_processing.c  
**Purpose**: Text processing pipeline  
**Compile**: `gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src real-world/02_string_processing.c -o string_processing`  
**Demonstrates**:
- Multi-stage string processing pipelines
- Whitespace normalization and trimming
- Character replacement and case conversion
- Word extraction and counting
- Text validation patterns

## Design Principles

All examples follow nur.h's core design principles:

### ✅ Zero-Allocation Core
- No dynamic memory allocation in main logic (except 03_allocation.c which demonstrates optional adapters)
- All data structures use fixed-size stack arrays  
- Processing done in-place or with stack buffers

### ✅ Explicit Error Handling
- Every fallible operation returns NurResult_T or NurResult_void
- No exceptions, errno checking, or NULL return patterns
- Errors must be explicitly handled or acknowledged

### ✅ Source Location Tracking  
- All errors automatically capture file, line, and function
- Error context preserved through propagation chains
- Rich debugging information without overhead

### ✅ Type Safety
- Tagged union prevents accessing error data as values
- NUR_NODISCARD prevents ignoring results
- Compiler warnings for unsafe patterns

### ✅ Multi-Standard Compatibility
- Clean compilation on C11, C17, and C23
- No compiler-specific extensions required
- Portable across GCC, Clang, and MSVC

## Common Patterns

### Error Construction
```c
// Basic error
return NUR_ERR(suffix, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Message"));

// Error with context  
return NUR_ERR(suffix, NUR_ERROR_CTX(NUR_E_PARSE, "Parse failed", "field_name"));

// System error
return NUR_ERR(suffix, NUR_ERROR_ERRNO("fopen failed"));
```

### Error Propagation
```c
// Portable propagation (works on all compilers)
int32_t value;
NUR_TRY_TO(DestSuffix, SrcSuffix, value, fallible_operation());

// Void function propagation
NUR_TRY_VOID(SrcSuffix, fallible_operation());

// Condition checking  
NUR_CHECK(condition, NUR_ERROR(NUR_E_INVALID_STATE, "Bad state"));
```

### Safe Value Access
```c
NurResult_i32 result = operation();
if (NUR_IS_OK(result)) {
    int32_t value = NUR_VALUE(result);
    // Use value safely
} else {
    nur_error_eprint(&NUR_ERROR_OF(result));
}
```

### Option Types
```c  
NurOption_cstr maybe_value = find_value();
if (NUR_IS_SOME(maybe_value)) {
    const char *value = NUR_OPTION_VALUE(maybe_value);
    // Use value
} else {
    // Handle missing value
}

// Or use fallback
const char *value = NUR_OPTION_UNWRAP_OR(maybe_value, "default");
```

## Running Examples

Each example can be compiled and run independently:

```bash
# Basic examples
cd examples/basic
gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 01_hello_result.c -o hello_result
./hello_result

# Advanced examples  
cd ../advanced
gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 04_parsing.c -o parsing
./parsing

# Real-world examples
cd ../real-world  
gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 01_config_parser.c -o config_parser
./config_parser
```

## Learning Path

**Recommended order for learning nur.h**:

1. **Start with basics**: `01_hello_result.c` → `02_error_codes.c` → `03_option_types.c`
2. **Learn custom types**: `04_custom_types.c` → `05_error_context.c`  
3. **Master propagation**: `advanced/01_error_propagation.c`
4. **Apply to I/O**: `advanced/02_file_io.c` → `advanced/04_parsing.c`
5. **Build complete apps**: `real-world/01_config_parser.c` → `real-world/02_string_processing.c`
6. **Optional**: Study `advanced/03_allocation.c` for allocation patterns (use cautiously)

## Integration Tips

- **Start small**: Begin with `NurResult_void` for functions that don't return values
- **Be consistent**: Use NurResult for all fallible operations in your codebase
- **Handle errors locally**: Check results immediately rather than propagating everything
- **Use context**: Add operation context to errors for better debugging
- **Test error paths**: Explicitly test failure conditions in your code
- **Document ownership**: Clearly document who owns/frees resources in comments

## Performance Notes

- **Zero overhead**: nur.h adds no runtime overhead compared to manual error checking
- **Stack allocation**: All examples use stack allocation for optimal performance  
- **Single pass**: Most algorithms are designed for single-pass processing
- **Minimal copying**: String operations minimize data copying where possible
- **Compiler optimization**: Tagged unions optimize to efficient machine code

## Platform Notes

- **Windows**: Examples handle Windows-specific paths and conventions
- **Unix/Linux**: Portable across Unix-like systems
- **Cross-platform**: Conditional compilation handles platform differences
- **Embedded**: Stack-only design suitable for resource-constrained environments