---
name: Bug report
about: Create a report to help us improve nur.h
title: '[BUG] '
labels: ['bug']
assignees: ''
---

## Bug Description

**Describe the bug**
A clear and concise description of what the bug is.

**Steps to Reproduce**
Steps to reproduce the behavior:
1. Include the code you're using
2. Compile with specific flags
3. Run the program
4. See error

**Expected Behavior**
A clear and concise description of what you expected to happen.

**Actual Behavior**
A clear and concise description of what actually happened.

## Environment

**Compiler Information:**
- Compiler: [e.g., GCC 11.2.0, Clang 14.0.0, MSVC 2019]
- C Standard: [e.g., -std=c99, -std=c11, -std=c17]
- Compilation flags: [e.g., -Wall -Wextra -O2]

**Operating System:**
- OS: [e.g., Ubuntu 22.04, Windows 11, macOS 13.0]
- Architecture: [e.g., x86_64, ARM64]

**nur.h Version:**
- Version: [e.g., 1.0.0, commit hash if using development version]

## Minimal Reproducer

**Complete code that demonstrates the issue:**
```c
#include "nur.h"

// Paste your minimal reproducing code here
int main() {
    // ...
    return 0;
}
```

**Compilation command:**
```bash
gcc -std=c11 -Wall -Wextra -o test test.c
```

**Output/Error:**
```
Paste the compiler output, runtime output, or error messages here
```

## Additional Context

**Stack trace (if applicable):**
```
Paste stack trace here if the program crashes
```

**Valgrind output (if applicable):**
```
Paste Valgrind output here if there are memory issues
```

**Additional context:**
Add any other context about the problem here. This could include:
- Related issues or discussions
- Workarounds you've tried
- Whether this is a regression from a previous version
- Performance impact (if any)

## Checklist

Before submitting this bug report, please ensure:

- [ ] I have searched existing issues to make sure this isn't a duplicate
- [ ] I have included a minimal reproducer
- [ ] I have tested with the latest version of nur.h
- [ ] I have included all relevant environment information
- [ ] This is a bug in nur.h itself, not in my application code