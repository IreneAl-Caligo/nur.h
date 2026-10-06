# Pull Request

## Description

**Brief summary of changes:**
Provide a clear and concise description of what this PR does.

**Type of change:**
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Code style/refactoring (no functional changes)
- [ ] Performance improvement
- [ ] Test improvements

## Related Issues

**Fixes/Closes:**
- Fixes #(issue number)
- Closes #(issue number)

**Related:**
- Related to #(issue number)

## Changes Made

**Detailed description of changes:**
- List the specific changes made
- Explain the reasoning behind the approach
- Highlight any design decisions

**Files modified:**
- `src/nur.h` - Brief description of changes
- `test/test_nur.c` - Added tests for new functionality
- `examples/example_name.c` - Added example demonstrating feature
- `README.md` - Updated documentation

## Testing

**Test coverage:**
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] Examples added/updated to demonstrate functionality

**Test results:**
```bash
# Paste output of make test-all here
$ make test-all
...
```

**Memory safety:**
- [ ] Valgrind clean (if applicable)
- [ ] No new memory leaks introduced

**Static analysis:**
- [ ] cppcheck clean (if applicable)
- [ ] No new static analysis warnings

## Compatibility

**C Standards:**
- [ ] Compiles with C99 (`-std=c99`)
- [ ] Compiles with C11 (`-std=c11`)
- [ ] Compiles with C17 (`-std=c17`)
- [ ] Compiles with C23 (`-std=c23`) (if available)

**Compilers:**
- [ ] GCC (tested version: ___)
- [ ] Clang (tested version: ___)
- [ ] MSVC (tested version: ___) [if applicable]

**Compiler warnings:**
- [ ] No new warnings with `-Wall -Wextra -Wpedantic`
- [ ] All functions returning `NurResult_*` have `NUR_NODISCARD` attribute

## Design Principles Compliance

**Core invariants maintained:**
- [ ] Single header only (no new .c files)
- [ ] Zero allocation in core (or properly guarded with `#ifndef NUR_NO_ALLOC`)
- [ ] No global mutable state introduced
- [ ] Multi-standard compatibility (C99-C23)
- [ ] No `goto` used for error handling

**Code style:**
- [ ] 4-space indentation, no tabs
- [ ] `snake_case` for functions and variables
- [ ] `SCREAMING_SNAKE_CASE` for macros
- [ ] Lines ≤ 100 characters
- [ ] Proper function documentation comments

## Documentation

**Documentation updates:**
- [ ] README.md updated (if user-facing changes)
- [ ] API documentation comments added/updated
- [ ] Examples added to demonstrate new features
- [ ] CONTRIBUTING.md updated (if development process changes)

**New error codes (if applicable):**
- [ ] Added to appropriate taxonomy range (1-9, 10-19, etc.)
- [ ] Added string mapping in `nur_error_code_name()`
- [ ] Documented in error code table

## Performance Impact

**Performance considerations:**
- [ ] No performance regression in existing functionality
- [ ] New code follows zero-cost abstraction principle
- [ ] Benchmarks run (if applicable): ___

## Breaking Changes

**Breaking changes (if any):**
- List any breaking changes
- Explain the necessity of breaking changes
- Provide migration guidance

**Deprecation notices:**
- Any deprecated functionality?
- Timeline for removal?

## Additional Context

**Screenshots/Output (if applicable):**
```
Paste relevant output, logs, or screenshots here
```

**Dependencies:**
- Any new dependencies added?
- Changes to build requirements?

**Future work:**
- Any follow-up work planned?
- Related features to implement?

## Reviewer Checklist

For reviewers to verify:

- [ ] Code follows nur.h design principles
- [ ] All tests pass
- [ ] No memory safety issues
- [ ] Proper error handling
- [ ] Documentation is complete and accurate
- [ ] Breaking changes are justified and documented
- [ ] Performance impact is acceptable

## Author Checklist

Before requesting review, I have:

- [ ] Read and followed [CONTRIBUTING.md](../CONTRIBUTING.md)
- [ ] Self-reviewed my code changes
- [ ] Ensured all tests pass locally
- [ ] Added/updated tests for new functionality
- [ ] Updated documentation as needed
- [ ] Verified compatibility across supported C standards
- [ ] Checked that no warnings are introduced
- [ ] Confirmed the change doesn't break existing functionality