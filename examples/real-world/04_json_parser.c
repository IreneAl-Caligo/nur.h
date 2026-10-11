/**
 * 04_json_parser.c - Zero-Allocation JSON Tokenizer & Field Reader
 *
 * Demonstrates:
 * - Zero-allocation JSON parsing and validation
 * - Semantic error codes: NUR_E_PARSE, NUR_E_FORMAT, NUR_E_NOT_FOUND, NUR_E_TYPE_MISMATCH
 * - Exact token location / context reporting
 * - Demonstrates success extraction and corrupted/malformed failure paths
 */

#include "nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Simple parsed string slice referencing the original JSON buffer */
typedef struct JsonSlice {
    const char *ptr;
    size_t len;
} JsonSlice;

NUR_RESULT_TYPE(JsonSlice, json_slice);

/**
 * Skip whitespace characters.
 */
static const char *skip_ws(const char *p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        p++;
    }
    return p;
}

/**
 * Parse a JSON quoted string starting at '"'.
 * Returns slice pointing inside the quotes (excluding surrounding quotes).
 */
static NurResult_json_slice parse_json_string(const char **p_inout) {
    const char *p = *p_inout;
    if (*p != '"') {
        return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_FORMAT, "Expected opening quote for string", p));
    }
    p++; /* skip opening quote */
    const char *start = p;

    while (*p && *p != '"') {
        if (*p == '\\') {
            p++; /* skip escaped character */
            if (*p == '\0') {
                return NUR_ERR(json_slice, NUR_ERROR(NUR_E_TRUNCATED, "Unterminated escape sequence in string"));
            }
        }
        p++;
    }

    if (*p != '"') {
        return NUR_ERR(json_slice, NUR_ERROR(NUR_E_PARSE, "Unterminated JSON string literal"));
    }

    JsonSlice slice;
    slice.ptr = start;
    slice.len = (size_t)(p - start);

    *p_inout = p + 1; /* advance past closing quote */
    return NUR_OK(json_slice, slice);
}

/**
 * Validate JSON object syntax and locate a top-level string field by key.
 * Zero heap allocation.
 */
static NurResult_json_slice json_get_string(const char *json, const char *target_key) {
    if (!json || !target_key) {
        return NUR_ERR(json_slice, NUR_ERROR(NUR_E_NULL_POINTER, "JSON buffer or target_key is NULL"));
    }

    const char *p = skip_ws(json);
    if (*p != '{') {
        return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_PARSE, "JSON must begin with '{'", p));
    }
    p++; /* skip '{' */

    size_t key_len = strlen(target_key);

    while (1) {
        p = skip_ws(p);
        if (*p == '}') {
            /* End of object reached without finding key */
            return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_NOT_FOUND, "Key not found in JSON object", target_key));
        }

        /* Parse key */
        NurResult_json_slice key_res = parse_json_string(&p);
        if (NUR_IS_ERR(key_res)) {
            return NUR_ERR(json_slice, NUR_ERROR_OF(key_res));
        }
        JsonSlice key = NUR_VALUE(key_res);

        p = skip_ws(p);
        if (*p != ':') {
            return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_PARSE, "Expected ':' after object key", p));
        }
        p++; /* skip ':' */

        p = skip_ws(p);
        /* Check if this is the key we want */
        int is_match = (key.len == key_len && strncmp(key.ptr, target_key, key_len) == 0);

        if (*p == '"') {
            NurResult_json_slice val_res = parse_json_string(&p);
            if (NUR_IS_ERR(val_res)) {
                return NUR_ERR(json_slice, NUR_ERROR_OF(val_res));
            }
            if (is_match) {
                return val_res;
            }
        } else {
            /* Skip non-string primitive */
            if (is_match) {
                return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_FORMAT, "Target field is not a string", target_key));
            }
            while (*p && *p != ',' && *p != '}' && !isspace((unsigned char)*p)) {
                p++;
            }
        }

        p = skip_ws(p);
        if (*p == ',') {
            p++;
            continue;
        } else if (*p == '}') {
            p++;
            break;
        } else {
            return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_PARSE, "Expected ',' or '}' in object", p));
        }
    }

    return NUR_ERR(json_slice, NUR_ERROR_CTX(NUR_E_NOT_FOUND, "Key not found in JSON object", target_key));
}

/**
 * Parse an integer field by key.
 */
static NurResult_i32 json_get_int(const char *json, const char *target_key) {
    if (!json || !target_key) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Input is NULL"));
    }

    const char *p = skip_ws(json);
    if (*p != '{') {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "JSON must begin with '{'"));
    }
    p++;

    size_t key_len = strlen(target_key);

    while (1) {
        p = skip_ws(p);
        if (*p == '}') {
            return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_NOT_FOUND, "Key not found", target_key));
        }

        NurResult_json_slice key_res = parse_json_string(&p);
        if (NUR_IS_ERR(key_res)) {
            return NUR_ERR(i32, NUR_ERROR_OF(key_res));
        }
        JsonSlice key = NUR_VALUE(key_res);

        p = skip_ws(p);
        if (*p != ':') {
            return NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "Expected ':' after key"));
        }
        p++;

        p = skip_ws(p);
        int is_match = (key.len == key_len && strncmp(key.ptr, target_key, key_len) == 0);

        if (is_match) {
            char *endptr = NULL;
            long val = strtol(p, &endptr, 10);
            if (endptr == p) {
                return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_FORMAT, "Field is not a valid integer", target_key));
            }
            return NUR_OK(i32, (int32_t)val);
        }

        /* Skip value */
        if (*p == '"') {
            NurResult_json_slice skip_str = parse_json_string(&p);
            if (NUR_IS_ERR(skip_str)) return NUR_ERR(i32, NUR_ERROR_OF(skip_str));
        } else {
            while (*p && *p != ',' && *p != '}' && !isspace((unsigned char)*p)) p++;
        }

        p = skip_ws(p);
        if (*p == ',') {
            p++;
        } else if (*p == '}') {
            break;
        } else {
            return NUR_ERR(i32, NUR_ERROR(NUR_E_PARSE, "Expected ',' or '}'"));
        }
    }

    return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_NOT_FOUND, "Key not found", target_key));
}

int main(void) {
    printf("========================================\n");
    printf("  nur.h Zero-Allocation JSON Parser     \n");
    printf("========================================\n\n");

    const char *valid_json =
        "{\n"
        "  \"service\": \"database-cluster\",\n"
        "  \"port\": 5432,\n"
        "  \"status\": \"healthy\",\n"
        "  \"connections\": 128\n"
        "}";

    printf("Parsing JSON document:\n%s\n\n", valid_json);

    /* 1. Extract string: service */
    NurResult_json_slice svc_res = json_get_string(valid_json, "service");
    if (NUR_IS_OK(svc_res)) {
        JsonSlice s = NUR_VALUE(svc_res);
        printf("[SUCCESS] Extracted string 'service': %.*s\n", (int)s.len, s.ptr);
    } else {
        nur_error_eprint(&NUR_ERROR_OF(svc_res));
    }

    /* 2. Extract int: port */
    NurResult_i32 port_res = json_get_int(valid_json, "port");
    if (NUR_IS_OK(port_res)) {
        printf("[SUCCESS] Extracted integer 'port': %d\n", NUR_VALUE(port_res));
    } else {
        nur_error_eprint(&NUR_ERROR_OF(port_res));
    }

    /* 3. Extract non-existent key */
    printf("\nTesting Error Case 1: Missing key 'timeout':\n");
    NurResult_i32 timeout_res = json_get_int(valid_json, "timeout");
    if (NUR_IS_ERR(timeout_res)) {
        printf("[EXPECTED ERROR] ");
        nur_error_eprint(&NUR_ERROR_OF(timeout_res));
    }

    /* 4. Type mismatch: expected string, found number */
    printf("\nTesting Error Case 2: Type mismatch for 'port' as string:\n");
    NurResult_json_slice type_err = json_get_string(valid_json, "port");
    if (NUR_IS_ERR(type_err)) {
        printf("[EXPECTED ERROR] ");
        nur_error_eprint(&NUR_ERROR_OF(type_err));
    }

    /* 5. Malformed JSON */
    printf("\nTesting Error Case 3: Malformed JSON syntax:\n");
    const char *malformed_json = "{\"service\": \"unclosed string, \"port\": 80}";
    NurResult_json_slice malformed_res = json_get_string(malformed_json, "service");
    if (NUR_IS_ERR(malformed_res)) {
        printf("[EXPECTED ERROR] ");
        nur_error_eprint(&NUR_ERROR_OF(malformed_res));
    }

    printf("\n=== JSON Parser Example Finished Successfully ===\n");
    return 0;
}
