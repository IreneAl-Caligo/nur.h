/**
 * 06_http_client.c - HTTP 1.1 Protocol Parser & Serializer with nur.h
 *
 * Demonstrates:
 * - Parsing HTTP status line, headers, and payload with zero dynamic allocation
 * - Protocol error categorization: NUR_E_PROTOCOL, NUR_E_TRUNCATED, NUR_E_FORMAT
 * - Fixed-buffer HTTP request construction with overflow protection
 * - Rich error context capture during wire protocol decoding
 */

#include "nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HttpResponse {
    int status_code;
    const char *status_text;
    size_t content_length;
    const char *body;
    size_t body_len;
} HttpResponse;

NUR_RESULT_TYPE(HttpResponse, http_response);

/**
 * Serialize an HTTP/1.1 GET request into a caller-provided fixed buffer.
 * Protected against buffer overflow (returns NUR_E_OVERFLOW).
 */
static NurResult_usize http_build_request(char *buf, size_t buf_cap, const char *host, const char *path) {
    if (!buf || !host || !path) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Buffer, host, or path is NULL"));
    }

    int written = snprintf(buf, buf_cap,
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: nur.h-http-client/1.0\r\n"
        "Connection: close\r\n"
        "Accept: */*\r\n\r\n",
        path, host);

    if (written < 0 || (size_t)written >= buf_cap) {
        return NUR_ERR(usize, NUR_ERROR_CTX(NUR_E_OVERFLOW, "Request exceeds buffer capacity", path));
    }

    return NUR_OK(usize, (size_t)written);
}

/**
 * Parse an HTTP 1.1 response string into an HttpResponse structure.
 * Zero heap allocations.
 */
static NurResult_http_response http_parse_response(const char *raw_response) {
    if (!raw_response) {
        return NUR_ERR(http_response, NUR_ERROR(NUR_E_NULL_POINTER, "raw_response is NULL"));
    }

    HttpResponse res;
    memset(&res, 0, sizeof(res));

    /* 1. Validate HTTP version prefix */
    if (strncmp(raw_response, "HTTP/1.1 ", 9) != 0 && strncmp(raw_response, "HTTP/1.0 ", 9) != 0) {
        return NUR_ERR(http_response, NUR_ERROR_CTX(NUR_E_PROTOCOL, "Unsupported or missing HTTP version", raw_response));
    }

    const char *p = raw_response + 9;
    char *endptr = NULL;
    long status = strtol(p, &endptr, 10);
    if (endptr == p || status < 100 || status > 599) {
        return NUR_ERR(http_response, NUR_ERROR_CTX(NUR_E_FORMAT, "Invalid HTTP status code", p));
    }
    res.status_code = (int)status;

    /* Move to status text */
    p = endptr;
    while (*p == ' ') p++;
    const char *line_end = strstr(p, "\r\n");
    if (!line_end) {
        return NUR_ERR(http_response, NUR_ERROR(NUR_E_TRUNCATED, "Missing CRLF after status line"));
    }
    res.status_text = p; /* note: borrowed reference from raw_response */

    /* Locate headers end */
    const char *headers_start = line_end + 2;
    const char *body_marker = strstr(headers_start, "\r\n\r\n");
    if (!body_marker) {
        return NUR_ERR(http_response, NUR_ERROR(NUR_E_TRUNCATED, "Incomplete HTTP response: missing header terminator CRLF CRLF"));
    }

    /* Extract Content-Length if present */
    const char *cl_pos = strstr(headers_start, "Content-Length: ");
    if (cl_pos && cl_pos < body_marker) {
        cl_pos += 16;
        long len = strtol(cl_pos, NULL, 10);
        if (len >= 0) {
            res.content_length = (size_t)len;
        }
    }

    res.body = body_marker + 4;
    res.body_len = strlen(res.body);

    /* Verify content-length matches body length if specified */
    if (res.content_length > 0 && res.body_len < res.content_length) {
        return NUR_ERR(http_response, NUR_ERROR(NUR_E_TRUNCATED, "Body received is smaller than Content-Length"));
    }

    return NUR_OK(http_response, res);
}

static void test_response(const char *name, const char *raw) {
    printf("--- Testing: %s ---\n", name);
    NurResult_http_response r = http_parse_response(raw);
    if (NUR_IS_OK(r)) {
        HttpResponse resp = NUR_VALUE(r);
        printf("[SUCCESS] Status: %d\n", resp.status_code);
        printf("          Content-Length: %zu\n", resp.content_length);
        printf("          Body preview:   %.*s\n", (int)(resp.body_len > 40 ? 40 : resp.body_len), resp.body);
    } else {
        printf("[EXPECTED ERROR] ");
        nur_error_eprint(&NUR_ERROR_OF(r));
    }
    printf("\n");
}

int main(void) {
    printf("========================================\n");
    printf("  nur.h HTTP 1.1 Parser & Serializer    \n");
    printf("========================================\n\n");

    /* 1. Request Serialization Test */
    char req_buf[256];
    NurResult_usize req_res = http_build_request(req_buf, sizeof(req_buf), "api.example.com", "/v1/metrics");
    if (NUR_IS_OK(req_res)) {
        printf("[SUCCESS] Formatted Request (%zu bytes):\n%s\n", NUR_VALUE(req_res), req_buf);
    } else {
        nur_error_eprint(&NUR_ERROR_OF(req_res));
    }

    /* 2. Buffer overflow test */
    char small_buf[32];
    NurResult_usize overflow_res = http_build_request(small_buf, sizeof(small_buf), "api.example.com", "/v1/metrics");
    if (NUR_IS_ERR(overflow_res)) {
        printf("[EXPECTED ERROR] Request builder caught buffer overflow: ");
        nur_error_eprint(&NUR_ERROR_OF(overflow_res));
        printf("\n");
    }

    /* 3. Valid 200 OK Response */
    const char *resp_200 =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 26\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"status\":\"server online\"}";
    test_response("Valid 200 OK Response", resp_200);

    /* 4. Malformed Protocol Header */
    const char *resp_bad_proto =
        "INVALID_PROTO 200 OK\r\n"
        "\r\n";
    test_response("Invalid Protocol Header", resp_bad_proto);

    /* 5. Truncated Response Header */
    const char *resp_truncated =
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Type: text/plain\r\n";
    test_response("Truncated Header (missing end marker)", resp_truncated);

    /* 6. Truncated Body vs Content-Length */
    const char *resp_short_body =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 100\r\n"
        "\r\n"
        "short body";
    test_response("Body length mismatch vs Content-Length", resp_short_body);

    return 0;
}
