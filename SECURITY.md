# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 1.x     | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

We take security vulnerabilities in nur.h seriously. If you discover a security issue, please follow these guidelines for responsible disclosure.

### How to Report

**DO NOT** open a public GitHub issue for security vulnerabilities.

Instead, please email us directly at: **irenealcaligo@gmail.com**

### What to Include

When reporting a security vulnerability, please provide the following information:

1. **Description**: A clear description of the vulnerability
2. **Impact**: What an attacker could achieve by exploiting this vulnerability
3. **Reproduction**: Step-by-step instructions to reproduce the issue
4. **Environment**: 
   - Compiler version (GCC, Clang, MSVC)
   - Operating system
   - C standard used (-std=c99, -std=c11, etc.)
5. **Proof of Concept**: A minimal code example demonstrating the vulnerability
6. **Suggested Fix**: If you have ideas for how to fix the issue (optional)

### Example Report

```
Subject: Security Vulnerability in nur.h - Buffer Overflow in NUR_TRY macro

Description:
The NUR_TRY macro is vulnerable to buffer overflow when processing 
specially crafted error messages longer than 256 characters.

Impact:
An attacker could cause a stack buffer overflow leading to potential 
code execution or denial of service.

Reproduction:
1. Create an error message > 256 characters
2. Use NUR_TRY with this error
3. Observe stack corruption

Environment:
- GCC 11.2.0 on Ubuntu 22.04
- Compiled with -std=c11 -O2

Proof of Concept:
[Include minimal reproducing code]
```

### Response Timeline

We aim to respond to security reports according to the following timeline:

- **48 hours**: Initial acknowledgment of your report
- **7 days**: Assessment of the vulnerability and impact analysis
- **14 days**: Development of a fix (for confirmed vulnerabilities)
- **30 days**: Public disclosure after fix is released

### What's In Scope

Security issues we consider in scope for nur.h include:

#### Memory Safety Issues
- Buffer overflows/underflows in macros or inline functions
- Use-after-free vulnerabilities
- Double-free vulnerabilities
- Stack buffer overflows
- Integer overflows leading to memory corruption

#### Type Safety Issues
- Type confusion vulnerabilities
- Unsafe casts that could lead to memory corruption
- Macro parameter injection leading to undefined behavior

#### Error Handling Issues
- Vulnerabilities where error conditions are not properly handled
- Information disclosure through error messages
- Denial of service through error handling paths

### What's Out of Scope

The following are generally **not** considered security vulnerabilities:

#### Example Code Issues
- Security issues in example programs (`examples/` directory)
- Vulnerabilities in test code (`test/` directory)
- Issues in build scripts or documentation

#### Expected Behavior
- Crashes when using `NUR_UNWRAP` on error results (this is intentional)
- Crashes when using `NUR_EXPECT` on error results (this is intentional)
- Performance issues or denial of service through excessive computation
- Issues requiring an attacker to modify the source code

#### External Dependencies
- Vulnerabilities in compilers (GCC, Clang, MSVC)
- Issues in the C standard library
- Platform-specific vulnerabilities

#### Misuse of the Library
- Incorrect usage not following documented API
- Issues arising from disabling safety features (e.g., `NUR_NO_STDIO`)
- Problems when users ignore compiler warnings

### Disclosure Policy

- We will coordinate with you on the timing of public disclosure
- We prefer coordinated disclosure after a fix is available
- We will credit you in the security advisory (unless you prefer anonymity)
- We may request a CVE if the vulnerability is severe enough

### Security Best Practices

When using nur.h in your applications:

1. **Enable Compiler Warnings**: Always compile with `-Wall -Wextra -Wpedantic`
2. **Use Static Analysis**: Run tools like cppcheck on your code
3. **Memory Safety Tools**: Use Valgrind, AddressSanitizer, or similar tools
4. **Keep Updated**: Use the latest version of nur.h
5. **Review Error Handling**: Ensure all `NurResult` values are properly checked

### Security Features

nur.h includes several built-in security features:

- **Type Safety**: Tagged unions prevent type confusion
- **Explicit Error Handling**: `NUR_NODISCARD` prevents ignored errors
- **Memory Safety**: Zero-allocation core prevents heap-related vulnerabilities
- **Source Location**: Error tracking aids in debugging security issues

### Contact

For security-related questions or concerns:

- **Security reports**: irenealcaligo@gmail.com
- **General security questions**: Create a GitHub issue with "security" label
- **Emergency contact**: For critical vulnerabilities affecting production systems

### Recognition

We maintain a list of security researchers who have responsibly disclosed vulnerabilities:

- [Security Hall of Fame will be listed here]

Thank you for helping keep nur.h secure!