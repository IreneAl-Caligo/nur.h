---
name: Feature request
about: Suggest an idea for improving nur.h
title: '[FEATURE] '
labels: ['enhancement']
assignees: ''
---

## Feature Summary

**Brief description of the feature:**
A clear and concise description of what you want to happen.

## Motivation and Use Case

**Is your feature request related to a problem? Please describe.**
A clear and concise description of what the problem is. Ex. I'm always frustrated when [...]

**Describe your use case:**
Explain the specific scenario where this feature would be helpful. Include:
- What you're trying to achieve
- Why current nur.h functionality is insufficient
- How this would improve your workflow

**Real-world example:**
```c
// Show how you would like to use this feature
NurResult_something result = nur_proposed_function(params);
if (NUR_IS_OK(result)) {
    // ...
}
```

## Proposed Solution

**Describe the solution you'd like:**
A clear and concise description of what you want to happen.

**Proposed API (if applicable):**
```c
// Suggest function signatures, macros, or types
NurResult_type nur_new_function(param1_t param1, param2_t param2);

#define NUR_NEW_MACRO(x) /* implementation */

typedef struct {
    // new type definition
} nur_new_type_t;
```

**Integration with existing API:**
Explain how this would work with existing nur.h features:
- Does it extend existing functionality?
- Would it require changes to existing APIs?
- How would it fit into the current error handling patterns?

## Design Considerations

**Compatibility:**
- [ ] Should be compatible with C99/C11/C17/C23
- [ ] Should work with GCC, Clang, and MSVC
- [ ] Should maintain zero-allocation guarantee (if applicable)
- [ ] Should follow existing naming conventions

**Error handling:**
- What error conditions should be considered?
- How should errors be represented?
- Should new error codes be added?

**Performance:**
- Should this be zero-cost/compile-time only?
- Are there runtime performance implications?
- Memory usage considerations?

## Alternatives Considered

**Describe alternatives you've considered:**
A clear and concise description of any alternative solutions or features you've considered.

**Workarounds:**
- Current workarounds you're using
- Why they're insufficient
- Limitations of existing approaches

**Other libraries:**
- How do other error handling libraries solve this?
- What can we learn from their approaches?
- Why can't you use those alternatives?

## Implementation Notes

**Technical approach (optional):**
If you have ideas about implementation:
- Macro-based vs function-based approach
- Required changes to core nur.h structure
- Potential challenges or complications

**Breaking changes:**
- Would this require breaking changes to existing APIs?
- How could breaking changes be minimized?
- Migration path for existing users?

## Additional Context

**Examples from other projects:**
Links or examples from other projects that implement similar functionality.

**Related issues:**
- Link any related GitHub issues
- Reference relevant discussions or RFCs

**Documentation impact:**
- What documentation would need to be updated?
- Would new examples be needed?
- Tutorial or guide implications?

**Testing considerations:**
- What test cases should be added?
- Edge cases to consider
- Integration test scenarios

## Checklist

Before submitting this feature request, please ensure:

- [ ] I have searched existing issues to make sure this isn't a duplicate
- [ ] I have clearly explained the motivation and use case
- [ ] I have considered compatibility with nur.h design principles
- [ ] I have thought about alternatives and workarounds
- [ ] This aligns with nur.h's goals of type safety and explicit error handling