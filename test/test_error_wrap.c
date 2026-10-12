#include "../src/nur.h"
#include <assert.h>
#include <stdio.h>

static void test_wrap_preserves_code(void) {
    nur_error_t orig = NUR_ERROR(NUR_E_PARSE, "bad parse");
    nur_error_t wrapped = NUR_WRAP_ERROR(orig, "in read_config()");
    assert(wrapped.code == NUR_E_PARSE);
    assert(wrapped.context != (const char *)0);
    assert(wrapped.message != (const char *)0);
    puts("test_wrap_preserves_code: PASS");
}

static void test_error_is_predicate(void) {
    nur_error_t e = NUR_ERROR(NUR_E_NOT_FOUND, "file missing");
    assert(NUR_ERROR_IS(e, NUR_E_NOT_FOUND));
    assert(!NUR_ERROR_IS(e, NUR_E_IO));
    puts("test_error_is_predicate: PASS");
}

static void test_error_wrap_alias(void) {
    nur_error_t orig = NUR_ERROR(NUR_E_IO, "read failed");
    nur_error_t w = NUR_ERROR_WRAP(orig, "while loading file");
    assert(w.code == NUR_E_IO);
    assert(w.context != (const char *)0);
    puts("test_error_wrap_alias: PASS");
}

static void test_root_cause_chaining(void) {
    nur_error_t root = NUR_ERROR_CTX(NUR_E_CONNECTION, "connection reset", "192.168.1.1:8080");
    assert(root.cause == (const char *)0);
    assert(NUR_ERROR_CAUSE(root) == (const char *)0);

    /* Wrap once: previous context becomes cause, new context is set */
    nur_error_t wrapped = NUR_ERROR_WRAP(root, "http_handshake");
    assert(wrapped.context != (const char *)0);
    assert(strcmp(wrapped.context, "http_handshake") == 0);
    assert(wrapped.cause != (const char *)0);
    assert(strcmp(wrapped.cause, "192.168.1.1:8080") == 0);
    assert(strcmp(NUR_ERROR_CAUSE(wrapped), "192.168.1.1:8080") == 0);

    /* Format output contains both context and caused by */
    char buf[256];
    nur_error_format(&wrapped, buf, sizeof(buf));
    assert(strstr(buf, "[http_handshake]") != (char *)0);
    assert(strstr(buf, "caused by: 192.168.1.1:8080") != (char *)0);

    /* JSON output contains cause */
    char json[256];
    nur_error_json(&wrapped, json, sizeof(json));
    assert(strstr(json, "\"cause\":\"192.168.1.1:8080\"") != (char *)0);
    puts("test_root_cause_chaining: PASS");
}

static void test_literal_enforcement(void) {
    nur_error_t e1 = NUR_ERROR_LIT(NUR_E_PARSE, "syntax error");
    assert(e1.code == NUR_E_PARSE);
    assert(strcmp(e1.message, "syntax error") == 0);

    nur_error_t e2 = NUR_ERROR_LIT_CTX(NUR_E_OUT_OF_RANGE, "out of bounds", "index validator");
    assert(e2.code == NUR_E_OUT_OF_RANGE);
    assert(strcmp(e2.message, "out of bounds") == 0);
    assert(strcmp(e2.context, "index validator") == 0);
    puts("test_literal_enforcement: PASS");
}

static void test_structured_log_json(void) {
    nur_error_t e = NUR_ERROR_CTX(NUR_E_IO, "disk read error", "partition1");
    /* Log to a temporary file */
    FILE *tmp = fopen("test_log_tmp.json", "w+");
    assert(tmp != NULL);
    nur_error_log_json(&e, tmp);
    rewind(tmp);
    char buf[512] = {0};
    size_t r = fread(buf, 1, sizeof(buf) - 1, tmp);
    assert(r > 0);
    assert(strstr(buf, "\"code\":30") != NULL || strstr(buf, "NUR_E_IO") != NULL);
    fclose(tmp);
    remove("test_log_tmp.json");
    puts("test_structured_log_json: PASS");
}

static void test_scope_cleanup(void) {
#if defined(NUR_GNU_EXTENSIONS)
    {
        NUR_AUTO_FREE char *p = malloc(128);
        assert(p != NULL);
        p[0] = 'X';
    }
#endif
    puts("test_scope_cleanup: PASS");
}

int main(void) {
    puts("=== test_error_wrap ===");
    test_wrap_preserves_code();
    test_error_is_predicate();
    test_error_wrap_alias();
    test_root_cause_chaining();
    test_literal_enforcement();
    test_structured_log_json();
    test_scope_cleanup();
    puts("All tests passed!");
    return 0;
}
