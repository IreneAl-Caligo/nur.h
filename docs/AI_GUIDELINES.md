# AI & LLM Code Generation Guidelines for nur.h

This guide defines the explicit usage contract, safety invariants, and prompt templates for AI coding assistants (Claude, GPT-4, Gemini, Cursor, GitHub Copilot) generating C code with `nur.h`.

---

## 1. When to Use vs. When NOT to Use

### ✅ When to Use nur.h
- **I/O & Filesystem**: Opening files, sockets, device streams, pipes.
- **Parsing & Serialization**: JSON, INI, CSV, protocol buffers, command-line arguments.
- **Network Operations**: Socket connect, bind, read, write, HTTP/TLS requests.
- **Dynamic Allocation**: Boundary checks around heap allocation (`malloc`, `calloc`).
- **Public API Boundaries**: Exposing fallible functions in shared libraries and modular components.
- **Hardware & Microcontroller Drivers**: Sensor readings, CRC validation, bus operations.

### ❌ When NOT to Use nur.h
- **Pure Arithmetic Operations**: Math functions (e.g. `sin()`, `cos()`, fast matrix multiplication) where failure is impossible or handled via IEEE 754 NaN/Inf.
- **Tight Inner Loops**: Critical performance sections where returning a struct on each iteration adds register pressure and failure cannot occur.
- **Trivial Getters / Setters**: Reading or assigning pre-validated struct members.
- **Interrupt Service Routines (ISRs)**: Bare-metal interrupt handlers with strict non-standard ABI and zero cycle latency requirements.

---

## 2. Invariant Rules for AI Agents

LLMs generating C code MUST adhere to the following invariants:

### Invariant 1: Check `NUR_IS_OK` Before `NUR_VALUE`
Accessing `.u.value` or calling `NUR_VALUE(r)` without verifying `NUR_IS_OK(r)` produces **Undefined Behavior** on error.
```c
// [CORRECT]
NurResult_i32 r = compute();
if (NUR_IS_OK(r)) {
    int val = NUR_VALUE(r);
    use_val(val);
} else {
    nur_error_eprint(&NUR_ERROR_OF(r));
}

// [FORBIDDEN - AI MUST NEVER GENERATE THIS]
int val = NUR_VALUE(r); // CRASH / UB IF r IS AN ERROR!
```

### Invariant 2: Error Strings Must Have Static Duration
The string pointers (`.message`, `.context`, `.file`, `.func`) in `nur_error_t` are **borrowed references**.
- **DO**: Pass string literals (`"failed to open"`) or pointers with static/global duration.
- **DO NOT**: Pass pointers to local stack buffers (e.g., `char err_buf[64]; snprintf(err_buf, ...); return NUR_ERR(..., NUR_ERROR(..., err_buf))`).

### Invariant 3: Use `NUR_TRY_GOTO` for Resource Cleanup
`NUR_TRY_TO` executes an immediate `return` from the enclosing function. If heap memory, file handles, or mutex locks were acquired prior to the macro, an early return **leaks memory**.
- Use `NUR_TRY_GOTO` whenever resources require cleanup.

### Invariant 4: `NUR_RESULT_TYPE` Placement
- Declare `NUR_RESULT_TYPE(Type, suffix);` once at **file scope** in a header file.
- Do NOT declare it inside a function body.
- Always terminate with a semicolon `;`.

---

## 3. LLM Prompt Pack

### Prompt 1: Converting Legacy C to NurResult
```text
Refactor the following C function to use the `nur.h` library:
- Replace integer/boolean return codes with a type-safe `NurResult_<type>` or `NurResult_void`.
- Eliminate out-pointers (e.g., `int *out`) by returning the value inside `NurResult`.
- Map system failures using `NUR_ERROR_ERRNO_CTX("description", context)`.
- If resources (file handles, heap buffers, sockets) are held, use `NUR_TRY_GOTO` to avoid leaks.
- Ensure all error messages are string literals with static lifetime.
- Mark the function declaration with `NUR_NODISCARD`.
```

### Prompt 2: Wrapping Standard I/O Safely
```text
Write a robust C file-reading function using `nur.h`:
- Take `const char *path` and return `NurResult_usize` indicating bytes read into a caller buffer.
- Validate arguments with `NUR_CHECK`.
- Capture `errno` with `NUR_ERROR_ERRNO_CTX` on `fopen`, `fread`, and `fclose` failures.
- Ensure the `FILE *` handle is closed on every error exit path using `NUR_TRY_GOTO`.
```

### Prompt 3: Self-Healing / Verification Prompt
```text
Review the generated C code against `nur.h` safety rules:
1. Is `NUR_IS_OK` checked before every invocation of `NUR_VALUE`?
2. Are any local stack-allocated char buffers passed into `NUR_ERROR` or `NUR_ERROR_CTX`?
3. Could an early return from `NUR_TRY_TO` leak any allocated memory or open handles?
4. Is `NUR_RESULT_TYPE` placed at file scope with a trailing semicolon?
If any violations are found, fix them immediately.
```
