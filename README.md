# nur.h - Rust-Style Error Handling for C

A **single-header, zero-allocation** C library that brings Rust's `Result<T,E>` and `Option<T>` patterns to ISO C with compile-time safety guarantees.

## 🎯 Features

- **🦺 Type-Safe Error Handling** - `NurResult<T>` forces acknowledgment of failures
- **🚀 Zero Runtime Cost** - Header-only, no allocations, stack-based value types  
- **📘 Multi-Standard** - Compiles cleanly with C99/C11/C17/C23
- **🔧 Compiler Warnings** - `[[nodiscard]]` attributes catch ignored errors
- **🎪 Macro Magic** - Error propagation with `NUR_TRY` 
- **🧵 Thread-Safe** - No global mutable state

## 📦 Quick Start

```c
#include "nur.h"

// Simple success/failure
NurResult_i32 divide(int a, int b) {
    if (b == 0) {
        return NUR_ERR(NUR_E_INVALID_ARGUMENT, "Division by zero", __FILE__, __LINE__);
    }
    return NUR_OK(a / b);
}

int main() {
    NurResult_i32 result = divide(10, 2);
    
    if (NUR_IS_OK(result)) {
        printf("Result: %d\n", NUR_UNWRAP(result));  // Prints: Result: 5
    } else {
        nur_error_print(result.error);  // Prints error details
    }
    
    return 0;
}
```

## 🚀 Error Propagation

The `NUR_TRY` macro enables Rust-style `?` operator behavior:

```c
NurResult_i32 complex_calculation(int a, int b, int c) {
    int step1 = NUR_TRY(divide(a, b));    // Returns early on error
    int step2 = NUR_TRY(divide(step1, c)); // Continues only if step1 succeeded
    return NUR_OK(step2 * 2);
}
```

## 📊 Predefined Types

| Type | C Type | Usage |
|------|--------|-------|
| `NurResult_i32` | `int` | `NUR_OK(42)` |
| `NurResult_i64` | `long` | `NUR_OK(1000000L)` |  
| `NurResult_usize` | `size_t` | `NUR_OK(strlen(str))` |
| `NurResult_f64` | `double` | `NUR_OK(3.14159)` |
| `NurResult_cstr` | `char *` | `NUR_OK("hello")` |
| `NurResult_ptr` | `void *` | `NUR_OK(malloc(100))` |
| `NurResult_void` | (no value) | `NUR_OK_VOID()` |

## 🛠️ Custom Types

```c
typedef struct { int x, y; } Point;
NUR_RESULT_TYPE(Point, point);  // Creates NurResult_point

Point p = {10, 20};
NurResult_point result = NUR_OK(p);
```

## ⚠️ Error Categories

| Range | Category | Examples |
|-------|----------|----------|
| 1-9 | Input/Validation | `NUR_E_INVALID_ARGUMENT`, `NUR_E_NULL_POINTER` |
| 10-19 | Data | `NUR_E_PARSE`, `NUR_E_FORMAT` |
| 20-29 | Resources | `NUR_E_OUT_OF_MEMORY`, `NUR_E_RESOURCE_BUSY` |
| 30-39 | I/O | `NUR_E_NOT_FOUND`, `NUR_E_IO`, `NUR_E_EOF` |
| 40-49 | Network/Async | `NUR_E_TIMEOUT`, `NUR_E_CANCELLED` |
| 50-59 | System | `NUR_E_UNSUPPORTED`, `NUR_E_INTERNAL` |
| ≥1000 | User-defined | `NUR_E_USER_BASE + n` |

## 🔄 Option Types

For nullable values without errors:

```c
NurOption_i32 maybe_parse(const char* str) {
    if (!str || !*str) return NUR_NONE_I32();
    return NUR_SOME(atoi(str));
}

NurOption_i32 opt = maybe_parse("42");
int value = NUR_UNWRAP_OR(opt, 0);  // 42, or 0 if none
```

## 🏗️ Building

### Requirements
- GCC ≥ 9, Clang ≥ 10, or MSVC 2019+
- C99/C11/C17/C23 support

### Build Commands
```bash
# Basic compilation check
make check

# Run test suite  
make test

# Test all C standards
make test-all

# Memory safety check (requires Valgrind)
make test-valgrind  

# Static analysis (requires cppcheck)
make analyze

# Development workflow
make dev

# Release validation
make release
```

## 📋 API Reference

### Result Constructors
- `NUR_OK(value)` - Create success result
- `NUR_ERR(code, msg, file, line)` - Create error result
- `NUR_ERROR(code, msg)` - Shorthand error with auto location
- `NUR_ERROR_ERRNO(msg)` - Error from current errno

### Result Inspection  
- `NUR_IS_OK(result)` - Check if result is success
- `NUR_IS_ERR(result)` - Check if result is error
- `NUR_UNWRAP(result)` - Extract value (panics on error)
- `NUR_UNWRAP_OR(result, default)` - Extract value or default

### Error Propagation
- `NUR_TRY(expr)` - Return early on error, unwrap on success
- `NUR_TRY_AS(expr, type)` - TRY with explicit type conversion
- `NUR_CHECK(cond, error)` - Assert condition or return error

### Option Types
- `NUR_SOME(value)` - Create Some option
- `NUR_NONE_<type>()` - Create None option  
- `NUR_IS_SOME(opt)` / `NUR_IS_NONE(opt)` - Check option state

### Utilities
- `nur_error_code_name(code)` - Get error code name
- `nur_error_print(error)` - Print formatted error

## 🎨 Design Principles

1. **Single Header** - Everything in `nur.h`, no `.c` files
2. **Zero Allocation** - Core never calls `malloc`
3. **No Global State** - All value types on the stack  
4. **Multi-Standard** - C99 to C23 compatibility
5. **Compiler Warnings** - `[[nodiscard]]` prevents ignored errors

## 🤝 Integration

Simply copy `src/nur.h` to your project and:

```c
#include "nur.h"
```

No build configuration, no linking, no dependencies.

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## 🔗 Links

- **Documentation**: [API Reference](docs/)
- **Examples**: [examples/](examples/)
- **Contributing**: [CONTRIBUTING.md](CONTRIBUTING.md)
- **Issues**: [GitHub Issues](https://github.com/yourusername/nur.h/issues)

---

*"Explicit is better than implicit. Errors should never pass silently."*