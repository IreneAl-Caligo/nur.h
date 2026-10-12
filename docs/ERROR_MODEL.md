# Error Model for nur.h

## 1. Error Code Taxonomy

| Range  | Category         |
|--------|------------------|
| 1-9    | Input/Validation |
| 10-19  | Data Processing  |
| 20-29  | Resource         |
| 30-39  | IO               |
| 40-49  | Network/Async    |
| 50-59  | System           |
| >=1000 | User-defined     |

## 2. Memory Safety Contract

All string pointers in nur_error_t are BORROWED references.
Never pass stack-local buffer pointers. Use string literals only.

## 3. Error Wrapping

Use NUR_WRAP_ERROR / NUR_ERROR_WRAP to add caller context.
Preserves original code, sys, message, source location; replaces context.

## 4. Error Checking Predicate

Use NUR_ERROR_IS(err, code) to check for specific error codes.

## 5. Output Formats

- nur_error_format() -> text
- nur_error_json()   -> RFC 8259 JSON
- nur_error_eprint() -> stderr shortcut

## 6. Best Practices

- Use the most specific error code available
- Add context at each architectural layer
- Log errors ONCE at the top-level boundary
- Use NUR_TRY_GOTO whenever resources need cleanup
- Prefer NUR_ERROR_IS for recovery logic over raw .code comparison
