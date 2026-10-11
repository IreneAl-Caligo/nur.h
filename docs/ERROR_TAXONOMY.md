# nur.h Error Taxonomy Specification

This document provides the canonical taxonomy, numerical categorization, errno/Win32 mappings, and machine-readable schema for all errors defined in `nur.h`.

---

## 1. Numerical Architecture

`nur.h` categorizes errors into deterministic numeric ranges to allow fast range-based switching and categorization:

| Range | Domain | Typical Operations |
|---|---|---|
| **0** | `NUR_E_NONE` | Successful completion |
| **1 – 9** | Input / Validation | Precondition checks, bounds, pointers, state |
| **10 – 19** | Data Processing | Parsing, decoding, encoding, hashing, format |
| **20 – 29** | Resource Management | Heap allocation, handles, concurrency, locks |
| **30 – 39** | Input / Output (I/O) | Filesystem, pipes, streams, EOF |
| **40 – 49** | Network & Async | Sockets, protocols, timeouts, cancellation |
| **50 – 59** | System & Internal | OS capabilities, assertions, bugs |
| **≥ 1000** | User / Application | Domain-specific application errors |

---

## 2. Complete Error Code Catalog

### Validation Errors (1–9)
- `NUR_E_INVALID_ARGUMENT (1)`: Invalid function argument or parameter.
- `NUR_E_OUT_OF_RANGE (2)`: Numeric or array index out of valid range.
- `NUR_E_NULL_POINTER (3)`: Disallowed NULL pointer received.
- `NUR_E_INVALID_STATE (4)`: Operation incompatible with current lifecycle state.
- `NUR_E_OVERFLOW (5)`: Arithmetic or buffer capacity overflow.
- `NUR_E_UNDERFLOW (6)`: Arithmetic underflow.

### Data Processing Errors (10–19)
- `NUR_E_PARSE (10)`: Syntax parsing failed (e.g. JSON, INI, CSV, XML).
- `NUR_E_FORMAT (11)`: Data structure or format constraint violation.
- `NUR_E_ENCODING (12)`: Character encoding error (UTF-8, ASCII).
- `NUR_E_CORRUPTED (13)`: Data structure damaged or internally inconsistent.
- `NUR_E_TRUNCATED (14)`: Input or stream unexpectedly ended.
- `NUR_E_CHECKSUM (15)`: CRC, MD5, SHA, or parity check mismatch.

### Resource Errors (20–29)
- `NUR_E_OUT_OF_MEMORY (20)`: System or pool allocator failed.
- `NUR_E_RESOURCE_LIMIT (21)`: OS or process limit hit (e.g. `RLIMIT_NOFILE`).
- `NUR_E_BUSY (22)`: Resource locked or unavailable temporarily.
- `NUR_E_QUOTA (23)`: Storage or bandwidth quota exceeded.
- `NUR_E_DEADLOCK (24)`: Deadlock detected or prevented.

### I/O Errors (30–39)
- `NUR_E_NOT_FOUND (30)`: File, directory, or key does not exist.
- `NUR_E_PERMISSION (31)`: Access denied / insufficient permissions.
- `NUR_E_ALREADY_EXISTS (32)`: File or entity already exists.
- `NUR_E_IO (33)`: Generic disk/device hardware I/O error.
- `NUR_E_EOF (34)`: End-of-file reached.
- `NUR_E_CLOSED (35)`: Stream or descriptor is closed.
- `NUR_E_WOULD_BLOCK (36)`: Non-blocking I/O operation would block.

### Network Errors (40–49)
- `NUR_E_TIMEOUT (40)`: Network or operation deadline expired.
- `NUR_E_CANCELLED (41)`: Async operation aborted by caller.
- `NUR_E_CONNECTION (42)`: Connection dropped, reset, or broken.
- `NUR_E_PROTOCOL (43)`: Wire protocol violation (HTTP, WebSocket).
- `NUR_E_AGAIN (44)`: Resource unavailable now, retry later.
- `NUR_E_REFUSED (45)`: Connection refused by target endpoint.

### System & Internal Errors (50–59)
- `NUR_E_UNSUPPORTED (50)`: Operation not implemented on this platform.
- `NUR_E_INTERNAL (51)`: Internal subsystem unexpected condition.
- `NUR_E_BUG (52)`: Contract violation or logical bug detected.
- `NUR_E_VERSION (53)`: Protocol or schema version mismatch.
- `NUR_E_UNINITIALIZED (54)`: Subsystem invoked prior to initialization.
- `NUR_E_SYSTEM (55)`: Operating system kernel call failed.

---

## 3. Standard POSIX `errno` Mapping

`NUR_ERROR_ERRNO(msg)` automatically translates standard POSIX `errno` codes into semantic `nur_error_t`:

| `errno` | Semantic Code | Description |
|---|---|---|
| `ENOENT` | `NUR_E_NOT_FOUND` | No such file or directory |
| `EACCES`, `EPERM` | `NUR_E_PERMISSION` | Permission denied |
| `EEXIST` | `NUR_E_ALREADY_EXISTS` | File exists |
| `ENOMEM` | `NUR_E_OUT_OF_MEMORY` | Cannot allocate memory |
| `EINVAL` | `NUR_E_INVALID_ARGUMENT` | Invalid argument |
| `EBUSY` | `NUR_E_BUSY` | Device or resource busy |
| `ETIMEDOUT` | `NUR_E_TIMEOUT` | Connection timed out |
| `ECONNREFUSED` | `NUR_E_REFUSED` | Connection refused |
| `ECONNRESET` | `NUR_E_CONNECTION` | Connection reset by peer |
| `EAGAIN`, `EWOULDBLOCK`| `NUR_E_WOULD_BLOCK` | Resource temporarily unavailable |
| `ENOSPC` | `NUR_E_RESOURCE_LIMIT` | No space left on device |
| `ENOSYS` | `NUR_E_UNSUPPORTED` | Function not implemented |

---

## 4. Win32 `GetLastError()` Mapping

`NUR_ERROR_FROM_WIN32(code, msg)` maps Windows errors:

| Win32 Code | Semantic Code |
|---|---|
| `ERROR_FILE_NOT_FOUND`, `ERROR_PATH_NOT_FOUND` | `NUR_E_NOT_FOUND` |
| `ERROR_ACCESS_DENIED` | `NUR_E_PERMISSION` |
| `ERROR_ALREADY_EXISTS`, `ERROR_FILE_EXISTS` | `NUR_E_ALREADY_EXISTS` |
| `ERROR_NOT_ENOUGH_MEMORY`, `ERROR_OUTOFMEMORY` | `NUR_E_OUT_OF_MEMORY` |
| `ERROR_INVALID_PARAMETER` | `NUR_E_INVALID_ARGUMENT` |
| `WAIT_TIMEOUT`, `ERROR_TIMEOUT` | `NUR_E_TIMEOUT` |
| `WSAECONNREFUSED` | `NUR_E_REFUSED` |
| `WSAECONNRESET` | `NUR_E_CONNECTION` |

---

## 5. JSON Machine-Readable Schema (RFC 8259)

`nur_error_json(&err, buf, cap)` emits zero-allocation JSON conforming to this schema:

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "type": "object",
  "properties": {
    "ok": { "type": "boolean", "const": false },
    "code": { "type": "integer", "minimum": 0 },
    "name": { "type": "string" },
    "message": { "type": ["string", "null"] },
    "context": { "type": ["string", "null"] },
    "sys": { "type": "integer" },
    "file": { "type": ["string", "null"] },
    "func": { "type": ["string", "null"] },
    "line": { "type": "integer", "minimum": 0 }
  },
  "required": ["ok", "code", "name"]
}
```

Example JSON output:
```json
{"ok":false,"code":30,"name":"NUR_E_NOT_FOUND","message":"config file missing","context":"/etc/app.conf","sys":2,"file":"main.c","func":"load_cfg","line":42}
```
