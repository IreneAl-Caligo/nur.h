# Contributing to nur.h

Thank you for your interest in contributing to nur.h! This document provides guidelines for contributors to ensure high-quality, consistent contributions.

## Prerequisites

Before contributing, make sure you have:

- **GCC ≥ 9** or **Clang ≥ 10** (MSVC 2019+ also supported)
- **make** build system
- Basic familiarity with C programming and error handling patterns

## Getting Started

1. **Clone and Test**
   ```bash
   git clone https://github.com/yourusername/nur.h.git
   cd nur.h
   
   # Run the full test suite
   make test
   
   # Test across all supported C standards
   make test-all
   ```

2. **Verify Your Environment**
   ```bash
   # Basic compilation check
   make check
   
   # Memory safety check (requires Valgrind)
   make test-valgrind
   
   # Static analysis (requires cppcheck)
   make analyze
   ```

## Code Style Guidelines

nur.h follows strict coding conventions to ensure consistency and maintainability:

### Formatting
- **Indentation**: 4 spaces, no tabs
- **Line length**: ≤ 100 characters
- **Braces**: K&R style with opening brace on same line

### Naming Conventions
- **Variables and functions**: `snake_case`
- **Types**: `snake_case` with `_t` suffix for primitive types
- **Macros and constants**: `SCREAMING_SNAKE_CASE`
- **Public API prefix**: All public symbols start with `NUR_` or `nur_`

### Examples
```c
// Good
typedef struct nur_point {
    int32_t x;
    int32_t y;
} nur_point_t;

#define NUR_MAX_BUFFER_SIZE 1024

NurResult_i32 calculate_distance(const nur_point_t *p1, const nur_point_t *p2);

// Bad
typedef struct Point {
    int X, Y;
} point;

#define maxBufferSize 1024
int calculateDistance(Point* p1, Point* p2);
```

## Non-Negotiable Invariants

These rules are fundamental to nur.h's design and **must never be violated**:

### 1. Single Header Only
- Everything lives in `src/nur.h`
- No `.c` source files in the core library
- No linking step required for users

### 2. Zero Allocation in Core
- `nur.h` itself never calls `malloc`, `calloc`, `realloc`
- Optional adapter functions (like `nur_malloc`) wrap allocation and return `NurResult`
- Use `#ifndef NUR_NO_ALLOC` guards for allocation-related code

### 3. No Global Mutable State
- All result objects are value types on the stack
- Thread safety by design through immutability
- No global variables or static mutable data

### 4. Compiler Warnings Required
- Every function returning `NurResult_*` must have `NUR_NODISCARD` attribute
- Code must compile without warnings using `-Wall -Wextra -Wpedantic`
- Support C99/C11/C17/C23 standards

### 5. No `goto` for Error Handling
- Use `NUR_TRY` macros instead of `goto cleanup` patterns
- Structured error propagation only

## Error Code Guidelines

When adding new error codes, follow the systematic taxonomy:

| Range | Category | Examples |
|-------|----------|----------|
| 1-9 | Input/Validation | `NUR_E_INVALID_ARGUMENT`, `NUR_E_NULL_POINTER` |
| 10-19 | Data | `NUR_E_PARSE`, `NUR_E_FORMAT` |
| 20-29 | Resources | `NUR_E_OUT_OF_MEMORY`, `NUR_E_BUSY` |
| 30-39 | I/O | `NUR_E_NOT_FOUND`, `NUR_E_IO` |
| 40-49 | Network/Async | `NUR_E_TIMEOUT`, `NUR_E_CANCELLED` |
| 50-59 | System | `NUR_E_UNSUPPORTED`, `NUR_E_INTERNAL` |
| ≥1000 | User-defined | `NUR_E_USER_BASE + n` |

### Adding a New Error Code

1. Choose the appropriate range based on the error category
2. Add the enum value to `nur_error_code_t` in `src/nur.h`
3. Add the string mapping in `nur_error_code_name()`
4. Document the error code in relevant examples

Example:
```c
// In enum nur_error_code_t
NUR_E_BUFFER_FULL = 25,    // Resources category (20-29)

// In nur_error_code_name()
case NUR_E_BUFFER_FULL: return "NUR_E_BUFFER_FULL";
```

## Adding New Predefined Result Types

To add a new `NurResult_<suffix>` type:

1. **Add the type declaration**:
   ```c
   NUR_RESULT_TYPE(your_type_t, suffix)
   ```

2. **Add convenience macros** (if needed):
   ```c
   #define NUR_OK_SUFFIX(val)  nur_ok_suffix(val)
   #define NUR_ERR_SUFFIX(err) nur_err_suffix(err)
   ```

3. **Add to predefined types section** in the header
4. **Update documentation** in README.md
5. **Add tests** in `test/test_nur.c`
6. **Add example** in `examples/`

## Testing Requirements

All contributions must include appropriate tests:

### Test Organization
- **Unit tests**: `test/test_nur.c`
- **Integration tests**: `test/integration/`
- **Examples**: `examples/` (these serve as integration tests)

### Test Coverage Requirements
- Every new function must have a unit test
- Both success and failure cases must be tested
- Error propagation must be verified
- Memory safety must be validated (Valgrind clean)

### Running Tests
```bash
# Basic test suite
make test

# All C standards
make test-all

# Memory safety
make test-valgrind

# Static analysis
make analyze

# Complete validation
make release
```

## Pull Request Checklist

Before submitting your pull request, ensure:

- [ ] **All tests pass**: `make test-all` succeeds
- [ ] **No compiler warnings**: Clean compilation with `-Wall -Wextra -Wpedantic`
- [ ] **C99/C11/C17 compatibility**: Code compiles with all supported standards
- [ ] **Memory safety**: `make test-valgrind` passes (if available)
- [ ] **Static analysis clean**: `make analyze` passes (if available)
- [ ] **New features have tests**: Added to `test/test_nur.c`
- [ ] **New features have examples**: Added to `examples/`
- [ ] **Documentation updated**: README.md updated if needed
- [ ] **Code style followed**: Matches existing style guidelines
- [ ] **Commit messages follow format**: See below

## Commit Message Format

Use conventional commit format:

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

### Types
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code changes that neither fix bugs nor add features
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

### Examples
```
feat(error): add NUR_E_BUFFER_FULL error code

Add new error code for buffer overflow conditions in the 
resources category (20-29 range).

Closes #42

fix(macro): prevent double evaluation in NUR_TRY

Use temporary variable to avoid evaluating expression twice
in NUR_TRY macro, which could cause side effects.

docs(readme): update installation instructions

Add Windows-specific build instructions and MSVC compatibility notes.
```

## Development Workflow

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/your-feature-name`
3. **Make your changes** following the guidelines above
4. **Run tests**: `make test-all`
5. **Commit your changes** with proper commit messages
6. **Push to your fork**: `git push origin feature/your-feature-name`
7. **Create a pull request** with description of changes

## Code Review Process

All pull requests will be reviewed for:

1. **Correctness**: Does the code work as intended?
2. **Safety**: No memory leaks, buffer overflows, or undefined behavior
3. **Style**: Follows project conventions
4. **Tests**: Adequate test coverage
5. **Documentation**: Changes are documented appropriately
6. **Compatibility**: Works across supported compilers and standards

## Questions?

If you have questions about contributing:

1. Check existing [issues](https://github.com/yourusername/nur.h/issues)
2. Read the [documentation](docs/)
3. Create a new issue with the "question" label

## Recognition

Contributors are recognized in:
- Git commit history
- `CONTRIBUTORS.md` file (for significant contributions)
- Release notes

Thank you for helping make nur.h better!