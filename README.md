# nur.h — Rust-Style Error Handling for Modern C

A **single-header, zero-allocation, pedantic warning-free** C library that brings Rust-style `Result<T,E>` and `Option<T>` patterns to ISO C with compiler-assisted safety via `[[nodiscard]]`.

**Author**: Irene Al-Caligo | **License**: MIT | **Compatibility**: ISO C99, C11, C17, C23 & C++11+

[![C Standard](https://img.shields.io/badge/Standards-C99%20%7C%20C11%20%7C%20C17%20%7C%20C23-blue.svg)](src/nur.h)
[![Pedantic Warnings](https://img.shields.io/badge/Warnings-Zero%20Pedantic-brightgreen.svg)](src/nur.h)
[![Zero Heap Allocation](https://img.shields.io/badge/Memory-0%20Bytes%20Heap-brightgreen.svg)](docs/BENCHMARKS.md)
[![Sanitizers](https://img.shields.io/badge/Sanitizers-ASan%20%7C%20UBSan%20Clean-success.svg)](.ci/ci.yml)
[![CI Matrix](https://img.shields.io/badge/CI-Passing%20(All%20Standards)-brightgreen.svg)](.ci/ci.yml)
[![AI / LLM Ready](https://img.shields.io/badge/AI%20DX-LLM%20Safe%20Contract-purple.svg)](docs/AI_GUIDELINES.md)

---

## 📑 Table of Contents

- [Why nur.h?](#-why-nurh)
- [Quick Start](#-quick-start)
- [Common Patterns Cheat Sheet](#-common-patterns-cheat-sheet)
- [Bad Patterns vs. Good Patterns](#-bad-patterns-vs-good-patterns)
- [When to Use & When NOT to Use](#-when-to-use--when-not-to-use)
- [Real-World Production Examples](#-real-world-production-examples)
- [Ergonomic Developer APIs](#-ergonomic-developer-apis)
- [Leak-Free Resource Cleanup (`NUR_TRY_GOTO`)](#-leak-free-resource-cleanup-nur_try_goto)
- [Performance & Zero-Allocation Benchmarks](#-performance--zero-allocation-benchmarks)
- [AI / LLM Autonomous Code Generation](#-ai--llm-autonomous-code-generation)
- [Build & Package Ecosystem](#-build--package-ecosystem)
- [Verification & Quality Gates](#-verification--quality-gates)
- [Documentation Index](#-documentation-index)

---

## 💡 Why nur.h?

C APIs communicate failure through at least four incompatible conventions (`-1`, `NULL`, `errno`, out-parameters) — none of which the compiler enforces you to check. `nur.h` unifies them under a single, typed `NurResult<T>` value that carries either a success payload or structured diagnostic context.

```c
// ❌ Old C — sentinel return, easily ignored, diagnostics lost
int val = parse_port("99999");
// Did you check validity? Did you check errno? Out of range?

// ✅ Modern nur.h — type-safe, nodiscard enforced, rich diagnostics
NurResult_i32 r = parse_port("99999");
if (NUR_IS_ERR(r)) {
    nur_error_eprint(&NUR_ERROR_OF(r));
    // Output: [NUR_E_OUT_OF_RANGE] Port exceeds 65535 (context: 99999) at net.c:42 in parse_port()
    return 1;
}
int port = NUR_VALUE(r);
```

---

## ⚡ Quick Start

### 1. Drop the single header into your project
```bash
# Direct download:
curl -fsSL https://raw.githubusercontent.com/IreneAl-Caligo/nur.h/master/src/nur.h -o include/nur.h
```

### 2. Basic Example
```c
#include "nur.h"
#include <stdio.h>

NUR_NODISCARD NurResult_i32 safe_divide(int32_t a, int32_t b) {
    if (b == 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "division by zero"));
    }
    return NUR_OK(i32, a / b);
}

int main(void) {
    NurResult_i32 r = safe_divide(10, 2);

    if (NUR_IS_OK(r)) {
        printf("Result: %d\n", NUR_VALUE(r)); // Result: 5
    } else {
        nur_error_eprint(&NUR_ERROR_OF(r));
    }

    /* Fallback value without branching */
    int32_t val = NUR_OR_ELSE(safe_divide(10, 0), -1); // -1
    printf("Fallback: %d\n", val);
    return 0;
}
```

### 3. Compile cleanly across any standard
```bash
gcc -std=c99 -Wall -Wextra -Wpedantic -Iinclude main.c -o main
```

---

## 📋 Common Patterns Cheat Sheet

| Task | Recommended Macro | Example |
|---|---|---|
| **Return Success** | `NUR_OK(suffix, val)` | `return NUR_OK(i32, 42);` |
| **Return Error** | `NUR_ERR(suffix, err)` | `return NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "msg"));` |
| **Precondition Guard** | `NUR_CHECK(cond, err)` | `NUR_CHECK(ptr != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "null"));` |
| **Safe Error Propagation** | `NUR_TRY_AUTO(Dest, lhs, expr)` | `NUR_TRY_AUTO(i32, val, parse_int(s));` |
| **Leak-Free Propagation** | `NUR_TRY_GOTO(lhs, expr, err, lbl)` | `NUR_TRY_GOTO(fd, open_fd(), err, cleanup);` |
| **Branch on Error** | `NUR_IF_ERR(res, err, body)` | `NUR_IF_ERR(r, err, { nur_error_eprint(&err); return -1; });` |
| **Pattern Match** | `NUR_MATCH_AS(Type, r, v, ok, e, err)` | `NUR_MATCH_AS(int, r, v, { use(v); }, e, { log(e); });` |
| **Fallback Value** | `NUR_OR_ELSE(r, fallback)` | `int port = NUR_OR_ELSE(parse_port(s), 8080);` |
| **Transform Value** | `NUR_MAP(r, fn)` | `NurResult_i32 r2 = NUR_MAP(r, double_it);` |
| **Result Chaining** | `NUR_AND_THEN(r, fn)` | `NurResult_i32 r2 = NUR_AND_THEN(r, parse_uid);` |
| **Wrap Error Context** | `NUR_ERROR_WRAP(err, ctx)` | `NUR_ERROR_WRAP(NUR_ERROR_OF(r), "config parse");` |
| **Test Error Code** | `NUR_ERROR_IS(err, code)` | `if (NUR_ERROR_IS(err, NUR_E_NOT_FOUND)) { ... }` |
| **Early Return on Err**| `NUR_RETURN_IF_ERR(expr)` | `NUR_RETURN_IF_ERR(validate_header(h));` |
| **Require Ok (Assert)** | `NUR_REQUIRE_OK(r)` | `NUR_REQUIRE_OK(init_subsystem());` |
| **Capture `errno`** | `NUR_ERROR_ERRNO_CTX(msg, ctx)` | `NUR_ERROR_ERRNO_CTX("fopen failed", path)` |

---

## ⚠️ Bad Patterns vs. Good Patterns

| Anti-Pattern (Bad) | Recommended Pattern (Good) | Rationale |
|---|---|---|
| `int val = NUR_VALUE(r);` without `NUR_IS_OK` | `if (NUR_IS_OK(r)) { int val = NUR_VALUE(r); }` | Avoids reading garbage memory / undefined behavior if result is an error. |
| `char buf[64]; snprintf(buf, ...); return NUR_ERR(i32, NUR_ERROR(..., buf));` | `return NUR_ERR(i32, NUR_ERROR_CTX(..., "msg", static_str));` | Pointers in `nur_error_t` are borrowed; stack buffers become dangling upon return. |
| `NUR_TRY_TO(...)` after allocating memory | `NUR_TRY_GOTO(val, call(), err, cleanup);` | Early `return` in `NUR_TRY_TO` leaks memory or open file descriptors. |
| Duplicate `NUR_RESULT_TYPE` inside `.c` files or function bodies | Declare `NUR_RESULT_TYPE(Type, sfx);` once in a shared header at file scope | Avoids type redeclaration warnings and maintains compilation symmetry. |

---

## 🎯 When to Use & When NOT to Use

### ✅ When to Use `nur.h`:
- File I/O, sockets, network communication, hardware buses.
- Parsing text formats (JSON, INI, CSV, CLI arguments).
- Dynamic memory allocations and resource pools.
- Public library boundaries where failure is an explicit possibility.
- Multi-layered services where errors must propagate with origin line/file metadata.

### ❌ When NOT to Use `nur.h`:
- Pure numeric arithmetic (e.g. matrix multiplication, trigonometric functions).
- Tight inner loops where failure is mathematically impossible and cycles are critical.
- Trivial member getters and setters.
- Hardware Interrupt Service Routines (ISRs) requiring bare register execution.

---

## 🚀 Real-World Production Examples

All examples compile cleanly under `-Wall -Wextra -Wpedantic` and demonstrate both success and failure handling:

| Directory | Example | Concepts Demonstrated |
|---|---|---|
| `examples/basic/` | [`01_hello_result.c`](examples/basic/01_hello_result.c) | Result creation, `NUR_IS_OK`, fallback values |
| `examples/basic/` | [`02_error_codes.c`](examples/basic/02_error_codes.c) | 36 semantic codes, categorization, user error codes |
| `examples/basic/` | [`03_option_types.c`](examples/basic/03_option_types.c) | `NurOption`, nullable values, array searching |
| `examples/basic/` | [`04_custom_types.c`](examples/basic/04_custom_types.c) | `NUR_RESULT_TYPE` with custom application structs |
| `examples/basic/` | [`05_error_context.c`](examples/basic/05_error_context.c) | Source location capture (`__FILE__`, `__LINE__`, `__func__`) |
| `examples/advanced/` | [`01_error_propagation.c`](examples/advanced/01_error_propagation.c) | 3-layer call stack propagation without boilerplate |
| `examples/advanced/` | [`02_file_io.c`](examples/advanced/02_file_io.c) | File operations, `errno` capture, resource cleanup |
| `examples/advanced/` | [`03_allocation.c`](examples/advanced/03_allocation.c) | `nur_malloc` / `nur_realloc` safe allocation wrappers |
| `examples/advanced/` | [`04_parsing.c`](examples/advanced/04_parsing.c) | Zero-allocation streaming CSV parser |
| `examples/real-world/` | [`01_config_parser.c`](examples/real-world/01_config_parser.c) | INI configuration parser with line-by-line error reporting |
| `examples/real-world/` | [`02_string_processing.c`](examples/real-world/02_string_processing.c) | Text processing pipeline with bounds verification |
| `examples/real-world/` | [`03_cli_arg_parser.c`](examples/real-world/03_cli_arg_parser.c) | CLI flags, key-value options, port range validation |
| `examples/real-world/` | [`04_json_parser.c`](examples/real-world/04_json_parser.c) | Zero-allocation streaming JSON tokenizer and extractor |
| `examples/real-world/` | [`05_socket_server.c`](examples/real-world/05_socket_server.c) | Cross-platform socket server with `NUR_TRY_GOTO` cleanup |
| `examples/real-world/` | [`06_http_client.c`](examples/real-world/06_http_client.c) | Wire-format HTTP 1.1 request serializer & response parser |
| `examples/real-world/` | [`07_thread_safe_logging.c`](examples/real-world/07_thread_safe_logging.c) | Thread-safe logging with RFC 8259 JSON serialization |
| `examples/real-world/` | [`08_embedded_sensor.c`](examples/real-world/08_embedded_sensor.c) | Embedded sensor driver with CRC-8 checksum verification |

Run `make examples` to compile and validate all 17 examples simultaneously.

---

## 🛡️ Leak-Free Resource Cleanup (`NUR_TRY_GOTO`)

When handling heap buffers, file descriptors, or mutexes, early returns lead to memory leaks. Use `NUR_TRY_GOTO` for clean, single-exit cleanup:

```c
NUR_NODISCARD NurResult_i32 process_stream(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NUR_ERR(i32, NUR_ERROR_ERRNO_CTX("fopen failed", path));

    char *buffer = malloc(4096);
    if (!buffer) {
        fclose(f);
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "malloc failed"));
    }

    int record_count = 0;
    nur_error_t err = {0};

    /* Evaluates parse_records(); if Err, assigns err and jumps to cleanup */
    NUR_TRY_GOTO(record_count, parse_records(f, buffer), err, cleanup);

    printf("Successfully processed %d records\n", record_count);

cleanup:
    free(buffer);
    if (f) fclose(f);
    if (err.code != NUR_E_NONE) return nur_err_i32(err);
    return NUR_OK(i32, record_count);
}
```

---

## 📊 Performance & Zero-Allocation Benchmarks

Measured with GCC `-O2` on AMD/Intel x86_64:

```
Benchmark                             Ops   Total (ms)        ns/op  vs Baseline
----------                            ---   ----------        -----  -----------
Baseline arithmetic              10000000       14.941        1.494        0.0%
Result<i32> ok construction      10000000        4.017        0.402      -73.1%
Result<i32> err construction     10000000        0.000        0.000     -100.0%
Result<i32> is_ok check          10000000        2.013        0.201      -86.5%
Error propagation (3-level)       1000000        1.473        1.473       -1.4%
```

- **Heap Allocations**: Exactly **0 bytes**.
- **Runtime Overhead**: Under **0.4 nanoseconds** per result construction; fully inlined into registers.
- **Freestanding**: Compatible with bare-metal microcontrollers via `-DNUR_FREESTANDING`.

See [docs/BENCHMARKS.md](docs/BENCHMARKS.md) for detailed memory layout and code-size comparisons.

---

## 🤖 AI / LLM Autonomous Code Generation

`nur.h` provides an explicit contract for AI assistants (ChatGPT, Claude, Copilot, Gemini):
- Read the [AI Guidelines & Prompt Pack](docs/AI_GUIDELINES.md).
- Automated C signature migration script: [`scripts/wrap_legacy_c.py`](scripts/wrap_legacy_c.py).
- Machine-readable RFC 8259 Error Taxonomy: [`docs/ERROR_TAXONOMY.md`](docs/ERROR_TAXONOMY.md) and [`docs/errors.json`](docs/errors.json).

---

## 📦 Build & Package Ecosystem

`nur.h` supports CMake, pkg-config, Meson, and vcpkg.

### CMake
```cmake
find_package(nur CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE nur::nur)
```

### pkg-config
```bash
gcc $(pkg-config --cflags nur) main.c -o main
```

### Meson
```meson
nur_dep = dependency('nur', required : true)
executable('app', 'main.c', dependencies : nur_dep)
```

Detailed installation steps available in [docs/PACKAGING.md](docs/PACKAGING.md).

---

## 🧪 Verification & Quality Gates

Run all quality checks locally:

```bash
# Run tests across all ISO C standards (C99, C11, C17, C23)
make test-all

# Check zero pedantic warnings
make check

# Validate all 17 real-world examples
make examples

# Run memory and undefined behavior sanitizers
make test-asan
```

---

## 📚 Documentation Index

- [Migration Guide (Legacy C -> nur.h)](docs/MIGRATION.md)
- [AI / LLM Usage Contract & Prompt Pack](docs/AI_GUIDELINES.md)
- [Error Taxonomy & POSIX/Win32 Mappings](docs/ERROR_TAXONOMY.md)
- [Performance & Latency Benchmarks](docs/BENCHMARKS.md)
- [Packaging Guide (CMake, pkg-config, Meson, vcpkg)](docs/PACKAGING.md)
- [Project Roadmap](ROADMAP.md)
- [Contribution Guidelines](CONTRIBUTING.md)
- [Security Disclosure Policy](SECURITY.md)
- [API Reference Manual](docs/api/api_ref.md)

---

## ⚖️ License

MIT License. Designed and maintained by Irene Al-Caligo.
