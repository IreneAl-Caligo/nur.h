# Error Handling Best Practices with `nur.h`

This guide outlines architectural patterns and safety practices for engineering robust C software using `nur.h`.

---

## 1. The 5 Safety Invariants

1. **Verify Before Access**: Always guard `NUR_VALUE(r)` with `if (NUR_IS_OK(r))` or use `NUR_UNWRAP_OR(r, default)`. Reading `.u.value` when `ok == false` is undefined behavior.
2. **Borrowed Lifetime Rule**: `nur_error_t` string pointers (`.message`, `.context`, `.cause`) are borrowed references. Always supply static string literals or heap pointers owned by the caller. Never pass pointers to ephemeral stack buffers.
3. **Leak-Free Resource Acquisition**: If memory, handles, sockets, or locks have been acquired, never use early-return macros (`NUR_TRY_TO`, `NUR_TRY_AUTO`). Always use `NUR_TRY_GOTO(lhs, expr, err, cleanup)` to route failures through a single cleanup section.
4. **Contextualize at Boundaries**: When propagating an error upward from a lower-level module (e.g. socket read), wrap it with caller context via `NUR_ERROR_WRAP(e, "higher_level_context")`. This captures both the origin and call flow without any heap allocation.
5. **No Silent Swallowing**: If an operation fails and you choose not to propagate it, explicitly document or log it. Never ignore a fallible result without either a fallback (`NUR_OR_ELSE`) or explicit ignoring (`NUR_IGNORE`).

---

## 2. Resource Management Pattern

```c
NUR_NODISCARD NurResult_i32 process_file(const char *path) {
    NUR_CHECK_AS_MSG(i32, path != NULL, NUR_E_NULL_POINTER, "path is NULL");

    FILE *f = fopen(path, "rb");
    if (!f) return NUR_ERR(i32, NUR_ERROR_ERRNO_CTX("fopen failed", path));

    char *buf = malloc(4096);
    if (!buf) {
        fclose(f);
        return NUR_ERR(i32, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "failed to allocate buffer"));
    }

    int bytes_processed = 0;
    nur_error_t err = {0};

    NUR_TRY_GOTO(bytes_processed, parse_stream(f, buf), err, cleanup);

cleanup:
    free(buf);
    if (f) fclose(f);
    if (err.code != NUR_E_NONE) return nur_err_i32(err);
    return NUR_OK(i32, bytes_processed);
}
```
