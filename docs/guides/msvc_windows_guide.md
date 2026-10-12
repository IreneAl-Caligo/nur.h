# MSVC and Windows Integration Guide for `nur.h`

`nur.h` is designed for clean cross-platform compilation across Windows (MSVC `cl.exe`, MinGW GCC, Clang-cl) and POSIX systems.

---

## 1. Compiler Flags

For Visual Studio / MSVC:
- Use `/std:c11` or `/std:c17` (supported in VS 2019 version 16.8+ and VS 2022).
- Recommended warning flags:
  ```cmd
  cl /W4 /std:c11 /Isrc main.c /Fe:main.exe
  ```

---

## 2. Windows and Winsock Error Integration

`nur.h` provides automatic mapping of Win32 and Winsock error codes to typed semantic errors:

```c
#include "nur.h"
#include <winsock2.h>

NUR_NODISCARD NurResult_i32 send_data(SOCKET s, const char *data, int len) {
    int bytes = send(s, data, len, 0);
    if (bytes == SOCKET_ERROR) {
        int err = WSAGetLastError();
        return NUR_ERR(i32, NUR_ERROR_FROM_WIN32(err, "socket send failed"));
    }
    return NUR_OK(i32, bytes);
}
```

Key mappings:
- `WSAEWOULDBLOCK` (10035) -> `NUR_E_WOULD_BLOCK`
- `WSAETIMEDOUT` (10060) -> `NUR_E_TIMEOUT`
- `WSAECONNRESET` (10054) -> `NUR_E_CONNECTION`
- `ERROR_FILE_NOT_FOUND` (2) -> `NUR_E_NOT_FOUND`
- `ERROR_ACCESS_DENIED` (5) -> `NUR_E_PERMISSION`

---

## 3. MSVC Macro Portability Note

MSVC does not support GNU statement expressions (`({ ... })`).
- `nur.h` automatically uses standard C11 `_Generic` dispatch for `NUR_UNWRAP_OR(r, fallback)` on MSVC with `/std:c11` or `/std:c17`, guaranteeing single-evaluation.
- For `NUR_TRY_GOTO` on MSVC, assign results of functions with side effects to a named variable before calling `NUR_TRY_GOTO`:
  ```c
  NurResult_i32 r = read_value();
  NUR_TRY_GOTO(val, r, err, cleanup);
  ```
