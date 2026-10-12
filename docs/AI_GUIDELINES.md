# AI and LLM Usage Contract for nur.h

Strict contract for AI-generated C code using nur.h.

## 1. The 5 Safety Rules

1. Never access NUR_VALUE without checking NUR_IS_OK first.
2. Never create nur_error_t from stack-allocated string buffers.
3. Always add context with NUR_ERROR_CTX when relevant.
4. Use NUR_TRY_GOTO when the function holds resources.
5. Never silently ignore NurResult_* return values.

## 2. Allowed Patterns

```c
// Pattern A: check-then-use
NurResult_i32 r = compute();
if (NUR_IS_OK(r)) {
    int val = NUR_VALUE(r);
} else {
    nur_error_eprint(&NUR_ERROR_OF(r));
}

// Pattern B: fallback
int ms = NUR_UNWRAP_OR(read_timeout(), 5000);

// Pattern C: resource cleanup
FILE *f = fopen(path, "rb");
int val = 0; nur_error_t err = {0};
NurResult_i32 _r = parse_file(f);
NUR_TRY_GOTO(val, _r, err, cleanup);
cleanup:
    fclose(f);
    if (err.code != NUR_E_NONE) return nur_err_i32(err);
    return NUR_OK(i32, val);
```

## 3. Forbidden Patterns

```c
int x = NUR_VALUE(r);        // FORBIDDEN: no NUR_IS_OK guard
return -1;                   // FORBIDDEN: raw sentinel
return NULL;                 // FORBIDDEN: raw NULL
// NUR_TRY_TO after malloc   // FORBIDDEN: leaks memory
// stack buf in error ctx    // FORBIDDEN: dangling pointer
```

## 4. Generation Checklist

1. All fallible functions return NurResult_*
2. NUR_IS_OK checked before every NUR_VALUE call
3. NUR_ERROR_CTX used for contextual diagnostics
4. NUR_TRY_GOTO used when resources are held
5. Both success and failure paths demonstrated
6. NUR_RESULT_TYPE at file scope with trailing semicolon

## 5. Prompt Templates

Prompt 1 - Convert Legacy C:
Replace int/bool return codes with NurResult_<type>.
Use NUR_ERROR_ERRNO_CTX for system failures.
Use NUR_TRY_GOTO for resource cleanup.

Prompt 2 - Verification:
1. NUR_IS_OK before every NUR_VALUE?
2. Stack buffers in error context?
3. NUR_TRY_TO leaking resources?
4. NUR_RESULT_TYPE at file scope?
5. NurResult_* ignored silently?

## 6. When to Use

USE: I/O, parsing, network, allocation, public API boundaries.
SKIP: pure math, tight inner loops, ISRs, trivial getters.
