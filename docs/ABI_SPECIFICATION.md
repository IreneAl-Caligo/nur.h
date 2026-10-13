# ABI Layout Freeze Specification (nur.h v1.4.x)

**Status**: FROZEN (Normative)  
**Applies to**: `src/nur.h`, `src/nur_core.h`  
**Specification Version**: 1.4.0  
**Authors**: Irene Al-Caligo  
**License**: MIT  

---

## 1. Overview & Stability Invariants

This document defines the strict, permanent **Application Binary Interface (ABI) specification** for all core data structures within `nur.h` and `nur_core.h`.

### The ABI Freeze Guarantee
1. **Semantic Versioning Promise**: Throughout the lifetime of `v1.x.y`, no modification to struct field order, member types, struct alignment, or total byte size will be made to any public type.
2. **Cross-Translation-Unit (TU) Interoperability**: Binary object files (`.o`, `.obj`, `.a`, `.so`, `.dll`) compiled against `src/nur.h` or `src/nur_core.h` under the same target architecture and compilation flags can exchange `NurResult` and `NurOption` instances across function boundaries without data corruption.
3. **C++ Standard Layout**: In C++11 and higher, all `NurResult_*` and `NurOption_*` types satisfy `std::is_standard_layout<T>::value == true`, ensuring safe pass-by-value and pass-by-pointer across C and C++ language boundaries (`extern "C"`).

---

## 2. Supported Architecture Data Models

The ABI specification accounts for the standard C data models across modern computing architectures:

| Data Model | Pointer (`sizeof(void*)`) | `int` | `long` | `long long` | Target Platforms |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **LP64** | 8 bytes (64-bit) | 4 | 8 | 8 | Linux, macOS, iOS, BSD (x86_64, aarch64, riscv64) |
| **LLP64** | 8 bytes (64-bit) | 4 | 4 | 8 | Windows (x86_64, aarch64) - MSVC, Clang-CL, MinGW |
| **ILP32** | 4 bytes (32-bit) | 4 | 4 | 8 | Linux 32-bit, Windows 32-bit, ARM Cortex-M, WebAssembly |

---

## 3. ABI Layout: `nur_error_t`

`nur_error_t` is the standard error payload carried in `NurResult` types.

### Standard Layout (Default, 64-bit LP64 & LLP64)
- **Total Size**: 152 bytes
- **Required Alignment**: 8 bytes

| Offset (Bytes) | Field Name | Type | Size | Alignment | Description |
| :--- | :--- | :--- | :---: | :---: | :--- |
| `0` | `code` | `nur_error_code_t` | 4 | 4 | 32-bit enumerated error code |
| `4` | `sys` | `int` | 4 | 4 | Underlying OS error (`errno` / `GetLastError`) |
| `8` | `message` | `const char *` | 8 | 8 | Borrowed primary error message |
| `16` | `context` | `const char *` | 8 | 8 | Borrowed contextual annotation |
| `24` | `cause` | `const char *` | 8 | 8 | Borrowed cause string (chaining) |
| `32` | `file` | `const char *` | 8 | 8 | Origin file name (`__FILE__`) |
| `40` | `func` | `const char *` | 8 | 8 | Origin function name (`__func__`) |
| `48` | `line` | `unsigned int` | 4 | 4 | Origin source line (`__LINE__`) |
| `52` | `context_count` | `unsigned char` | 1 | 1 | Active elements in `context_chain` (0..4) |
| `53..55` | *Padding* | — | 3 | — | Implicit alignment padding to 8-byte boundary |
| `56` | `context_chain[4]`| `const char *[4]` | 32 | 8 | Multi-layer contextual stack traces |
| `88` | `inline_msg[64]` | `char[64]` | 64 | 1 | Inline stack buffer preventing use-after-free |

### Standard Layout (32-bit ILP32)
- **Total Size**: 116 bytes (or 120 bytes with 8-byte boundary padding)
- **Required Alignment**: 4 bytes

| Offset (Bytes) | Field Name | Type | Size | Alignment |
| :--- | :--- | :--- | :---: | :---: |
| `0` | `code` | `nur_error_code_t` | 4 | 4 |
| `4` | `sys` | `int` | 4 | 4 |
| `8` | `message` | `const char *` | 4 | 4 |
| `12` | `context` | `const char *` | 4 | 4 |
| `16` | `cause` | `const char *` | 4 | 4 |
| `20` | `file` | `const char *` | 4 | 4 |
| `24` | `func` | `const char *` | 4 | 4 |
| `28` | `line` | `unsigned int` | 4 | 4 |
| `32` | `context_count` | `unsigned char` | 1 | 1 |
| `33..35` | *Padding* | — | 3 | — |
| `36` | `context_chain[4]`| `const char *[4]` | 16 | 4 |
| `52` | `inline_msg[64]` | `char[64]` | 64 | 1 |

### Compact Error Mode (`#define NUR_COMPACT_ERRORS 1`)
For constrained embedded devices or microcontrollers:
- **Total Size (64-bit)**: 16 bytes (align 8). Fields: `code` (offset 0), `line` (offset 4), `message` (offset 8).
- **Total Size (32-bit)**: 12 bytes (align 4). Fields: `code` (offset 0), `line` (offset 4), `message` (offset 8).

---

## 4. ABI Layout: Predefined Result Types (`NurResult_*`)

All `NurResult` types are tagged unions structured as:
```c
typedef struct {
    bool ok;          /* offset 0, size 1 */
    /* padding */     /* offset 1..7 (on 64-bit systems) */
    union {
        T           value;
        nur_error_t error;
    } u;              /* offset 8 (on 64-bit systems) */
} NurResult_T;
```

### 64-bit Architecture Specification (LP64 / LLP64)

| Type | Target Type `T` | `sizeof(T)` | Offset `ok` | Offset `u` | Total Size | Alignment |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| `NurResult_i8` | `int8_t` | 1 | 0 | 8 | 160 | 8 |
| `NurResult_u8` | `uint8_t` | 1 | 0 | 8 | 160 | 8 |
| `NurResult_i16` | `int16_t` | 2 | 0 | 8 | 160 | 8 |
| `NurResult_u16` | `uint16_t` | 2 | 0 | 8 | 160 | 8 |
| `NurResult_i32` | `int32_t` | 4 | 0 | 8 | 160 | 8 |
| `NurResult_u32` | `uint32_t` | 4 | 0 | 8 | 160 | 8 |
| `NurResult_i64` | `int64_t` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_u64` | `uint64_t` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_usize`| `size_t` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_f32` | `float` | 4 | 0 | 8 | 160 | 8 |
| `NurResult_f64` | `double` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_bool` | `bool` | 1 | 0 | 8 | 160 | 8 |
| `NurResult_cstr` | `char *` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_ptr` | `void *` | 8 | 0 | 8 | 160 | 8 |
| `NurResult_void` | *none* | 0 | 0 | 8 | 160 | 8 |

*Note*: Because the union `u` contains `nur_error_t` (size 152 bytes, alignment 8), the union size is `max(sizeof(T), 152) = 152` bytes, and the total struct size is `8 (offset of u) + 152 = 160` bytes.

---

## 5. ABI Layout: Predefined Option Types (`NurOption_*`)

`NurOption` types represent nullable/optional values without error payload overhead:
```c
typedef struct {
    bool some;   /* offset 0 */
    T    value;  /* offset aligned to sizeof(T) */
} NurOption_T;
```

### 64-bit Architecture Specification (LP64 / LLP64)

| Type | Inner Type `T` | `sizeof(T)` | Offset `some` | Offset `value` | Total Size | Alignment |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| `NurOption_i8` | `int8_t` | 1 | 0 | 1 | 2 | 1 |
| `NurOption_u8` | `uint8_t` | 1 | 0 | 1 | 2 | 1 |
| `NurOption_i16` | `int16_t` | 2 | 0 | 2 | 4 | 2 |
| `NurOption_u16` | `uint16_t` | 2 | 0 | 2 | 4 | 2 |
| `NurOption_i32` | `int32_t` | 4 | 0 | 4 | 8 | 4 |
| `NurOption_u32` | `uint32_t` | 4 | 0 | 4 | 8 | 4 |
| `NurOption_i64` | `int64_t` | 8 | 0 | 8 | 16 | 8 |
| `NurOption_u64` | `uint64_t` | 8 | 0 | 8 | 16 | 8 |
| `NurOption_usize`| `size_t` | 8 | 0 | 8 | 16 | 8 |
| `NurOption_f32` | `float` | 4 | 0 | 4 | 8 | 4 |
| `NurOption_f64` | `double` | 8 | 0 | 8 | 16 | 8 |
| `NurOption_bool` | `bool` | 1 | 0 | 1 | 2 | 1 |
| `NurOption_cstr` | `char *` | 8 | 0 | 8 | 16 | 8 |

---

## 6. Automated Conformance & CI Verification

Conformance to this ABI specification is automatically verified on every Pull Request and commit via:
- [test/test_abi_layout.c](file:///test/test_abi_layout.c): Contains compile-time static assertions (`_Static_assert`) and runtime verification.
- Continuous Integration jobs enforcing zero layout drift on:
  - GCC (Linux x86_64)
  - Clang (Linux & macOS AArch64)
  - MSVC (Windows x86_64)
  - 32-bit cross-compilation environments
