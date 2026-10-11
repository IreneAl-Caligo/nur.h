# Migrating Legacy C Code to nur.h

This guide explains how to convert defensive, errno-based, sentinel-based, and out-pointer legacy C code into type-safe, leak-free, zero-allocation `NurResult` APIs.

---

## 1. Core Conversion Patterns

### Pattern 1: Return Code + Out-Pointer `int fn(..., T *out)`

In legacy C, functions frequently return an integer status code (0 for success, -1 for error) and write results into a caller-supplied pointer.

#### Legacy C (Before):
```c
int parse_port(const char *str, int *out_port) {
    if (!str || !out_port) return -1;
    int val = atoi(str);
    if (val <= 0 || val > 65535) return -2;
    *out_port = val;
    return 0; /* success */
}

/* Call site: verbose, prone to uninitialized reads */
int port;
if (parse_port(argv[1], &port) != 0) {
    fprintf(stderr, "Invalid port\n");
    return 1;
}
printf("Listening on port %d\n", port);
```

#### Modern nur.h (After):
```c
#include "nur.h"

NUR_NODISCARD NurResult_i32 parse_port(const char *str) {
    if (!str) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Port string is NULL"));
    }
    int val = atoi(str);
    if (val <= 0 || val > 65535) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, "Port out of range 1-65535", str));
    }
    return NUR_OK(i32, val);
}

/* Call site: compiler enforces checking, no uninitialized port variable */
NurResult_i32 res = parse_port(argv[1]);
if (!NUR_IS_OK(res)) {
    nur_error_eprint(&NUR_ERROR_OF(res));
    return 1;
}
int port = NUR_VALUE(res);
printf("Listening on port %d\n", port);
```

---

### Pattern 2: NULL-Returning Functions

Legacy C functions returning pointers often return `NULL` to signify failure, losing all diagnostics regarding *why* the failure occurred.

#### Legacy C (Before):
```c
FILE *open_config(const char *path) {
    if (!path) return NULL;
    return fopen(path, "r"); /* caller cannot distinguish ENOENT vs EACCES */
}
```

#### Modern nur.h (After):
```c
/* Declare typed result at file scope */
NUR_RESULT_TYPE(FILE *, file);

NUR_NODISCARD NurResult_file open_config(const char *path) {
    if (!path) {
        return NUR_ERR(file, NUR_ERROR(NUR_E_NULL_POINTER, "Path is NULL"));
    }
    FILE *f = fopen(path, "r");
    if (!f) {
        /* Automatically maps ENOENT -> NUR_E_NOT_FOUND, EACCES -> NUR_E_PERMISSION */
        return NUR_ERR(file, NUR_ERROR_ERRNO_CTX("Failed to open config file", path));
    }
    return NUR_OK(file, f);
}
```

---

### Pattern 3: Sentinel Values (-1 or EOF)

Sentinel returns conflate valid numeric domains with error states.

#### Legacy C (Before):
```c
int find_user_index(const char *username) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(users[i].name, username) == 0) return i;
    }
    return -1; /* Sentinel: not found */
}
```

#### Modern nur.h (After):
Use `NurOption` when absence is a normal, non-exceptional outcome:
```c
NUR_NODISCARD NurOption_usize find_user_index(const char *username) {
    for (size_t i = 0; i < user_count; ++i) {
        if (strcmp(users[i].name, username) == 0) {
            return NUR_SOME(usize, i);
        }
    }
    return NUR_NONE(usize);
}

/* Call site */
NurOption_usize opt = find_user_index("alice");
if (NUR_IS_SOME(opt)) {
    size_t idx = NUR_OPTION_VALUE(opt);
    printf("Found at index %zu\n", idx);
}
```

---

### Pattern 4: Resource Cleanup & Leak Prevention

Early returns via simple `NUR_TRY_TO` can leak allocated heap memory, file descriptors, or mutexes. When resources must be released, use `NUR_TRY_GOTO`.

#### Legacy C (Before):
```c
int process_data(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char *buf = malloc(4096);
    if (!buf) { fclose(f); return -2; }

    int res = parse_records(f, buf);
    if (res != 0) {
        free(buf);
        fclose(f);
        return -3;
    }

    free(buf);
    fclose(f);
    return 0;
}
```

#### Modern nur.h (After):
```c
NUR_NODISCARD NurResult_void process_data(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return nur_err_void(NUR_ERROR_ERRNO_CTX("fopen failed", path));

    char *buf = malloc(4096);
    if (!buf) {
        fclose(f);
        return nur_err_void(NUR_ERROR(NUR_E_OUT_OF_MEMORY, "malloc failed"));
    }

    nur_error_t err = {0};
    int record_count;

    /* If parse_records fails, err is assigned and execution jumps to cleanup */
    NUR_TRY_GOTO(record_count, parse_records(f, buf), err, cleanup);

    printf("Processed %d records\n", record_count);

cleanup:
    free(buf);
    fclose(f);
    if (err.code != NUR_E_NONE) return nur_err_void(err);
    return nur_ok_void();
}
```

---

## 2. Refactoring Checklist

1. [ ] **Verify Return Codes**: Replace `int (0/-1)` returns with `NurResult_<type>` or `NurResult_void`.
2. [ ] **Eliminate Out-Pointers**: Return data directly in the `NurResult` rather than via `T *out`.
3. [ ] **Audit String Lifetimes**: Ensure error strings in `NUR_ERROR` and `NUR_ERROR_CTX` are static literals or caller-persisted buffers (no stack arrays).
4. [ ] **Use NUR_TRY_GOTO for Resources**: Anywhere dynamic allocation or OS handles exist, use `NUR_TRY_GOTO` instead of early returns.
5. [ ] **Enforce Checked Returns**: Mark all fallible functions with `NUR_NODISCARD`.
