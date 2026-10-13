/*
 * test_super_header_and_dropin.c — Comprehensive validation of:
 * 1. Batteries-included super-header (stdio, math, string, stdlib, time out of the box)
 * 2. Built-in drop-in safe helpers (nur_sqrt, nur_strlen)
 * 3. The 4 domain patterns:
 *    - Pure numeric arithmetic (matrix multiplication boundary)
 *    - Tight inner loops (validate-at-entry, raw loop speed)
 *    - Trivial getters and setters (NurOption and NurResult_void)
 *    - Hardware ISRs (bare-metal register simulation with 0 heap)
 */

#include "../src/nur.h"
#include <assert.h>

/* --- Domain 1: Pure Numeric Arithmetic Pattern --- */
typedef struct {
    int rows;
    int cols;
    float data[4][4];
} Matrix4x4;

NUR_RESULT_TYPE(Matrix4x4, mat4);

static NurResult_mat4 multiply_matrices(const Matrix4x4 *a, const Matrix4x4 *b) {
    Matrix4x4 res;
    int i, j, k;

    if (a->cols != b->rows) {
        return NUR_ERR(mat4, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "matrix dimensions mismatch"));
    }
    memset(&res, 0, sizeof(res));
    res.rows = a->rows;
    res.cols = b->cols;

    /* Pure inner loop runs with zero overhead */
    for (i = 0; i < a->rows; ++i) {
        for (j = 0; j < b->cols; ++j) {
            float sum = 0.0f;
            for (k = 0; k < a->cols; ++k) {
                sum += a->data[i][k] * b->data[k][j];
            }
            res.data[i][j] = sum;
        }
    }
    return NUR_OK(mat4, res);
}

/* --- Domain 2: Tight Inner Loop Pattern --- */
static NurResult_void scale_buffer(float *buf, size_t len, float factor) {
    size_t i;
    NUR_CHECK(buf != NULL, NUR_ERROR(NUR_E_NULL_POINTER, "buffer is NULL"));
    /* Precondition verified once at boundary. Inner loop runs at 100% bare-metal speed */
    for (i = 0; i < len; ++i) {
        buf[i] *= factor;
    }
    return NUR_OK_VOID();
}

/* --- Domain 3: Trivial Getters & Setters Pattern --- */
typedef struct {
    int id;
    int port;
    bool has_nickname;
    char nickname[32];
} ServerConfig;

static NurResult_void server_set_port(ServerConfig *cfg, int port) {
    if (port <= 0 || port > 65535) {
        return nur_err_void(NUR_ERROR(NUR_E_OUT_OF_RANGE, "port must be between 1 and 65535"));
    }
    cfg->port = port;
    return nur_ok_void();
}

static NurOption_cstr server_get_nickname(const ServerConfig *cfg) {
    if (!cfg->has_nickname) {
        return NUR_NONE(cstr);
    }
    return NUR_SOME(cstr, (char *)cfg->nickname);
}

/* --- Domain 4: Hardware ISR Pattern (Bare Register Execution) --- */
typedef struct {
    volatile uint32_t status_reg;
    volatile uint32_t data_reg;
} HardwareUart;

static void isr_uart_rx_handler(HardwareUart *uart, uint8_t *out_byte, uint8_t *out_err_flags) {
    /* ISR reads bare registers directly with 0 heap allocation */
    uint32_t status = uart->status_reg;
    if (status & 0x01) { /* RX ready */
        *out_byte = (uint8_t)(uart->data_reg & 0xFF);
        *out_err_flags = (uint8_t)((status >> 1) & 0x07); /* framing, parity, overrun */
    }
}

int main(void) {
    double sq;
    char text[6];
    size_t len;
    int r;
    time_t t;
    NurResult_f64 sq_ok;
    NurResult_f64 sq_err;
    NurResult_usize str_ok;
    NurResult_usize str_err;
    Matrix4x4 m1, m2;
    NurResult_mat4 m_res;
    Matrix4x4 out;
    float audio[4];
    NurResult_void scale_res;
    ServerConfig srv;
    NurOption_cstr nick_opt;
    HardwareUart mock_uart;
    uint8_t byte_val = 0, err_flags = 0;

    puts("=== Super-Header & Drop-in Safety Validation ===");

    /* 1. Super-header drop-in standard C tests (zero extra includes required) */
    printf("1. Standard printf works\n");

    sq = sqrt(16.0);
    assert(sq == 4.0);
    printf("2. Standard sqrt(16.0) = %f\n", sq);

    memcpy(text, "Hello", 6);
    len = strlen(text);
    assert(len == 5);
    printf("3. Standard strlen(\"Hello\") = %lu\n", (unsigned long)len);

    r = rand();
    (void)r;
    printf("4. Standard rand() = %d\n", r);

    time(&t);
    printf("5. Standard time() = %ld\n", (long)t);

    /* 2. Built-in drop-in safe helpers (nur_sqrt, nur_strlen) */
    sq_ok = nur_sqrt(25.0);
    assert(NUR_IS_OK(sq_ok) && NUR_VALUE(sq_ok) == 5.0);

    sq_err = nur_sqrt(-4.0);
    assert(NUR_IS_ERR(sq_err) && NUR_ERROR_OF(sq_err).code == NUR_E_INVALID_ARGUMENT);

    str_ok = nur_strlen("nur.h");
    assert(NUR_IS_OK(str_ok) && NUR_VALUE(str_ok) == 5);

    str_err = nur_strlen(NULL);
    assert(NUR_IS_ERR(str_err) && NUR_ERROR_OF(str_err).code == NUR_E_NULL_POINTER);

    /* 3. Domain 1: Matrix Multiplication */
    memset(&m1, 0, sizeof(m1));
    m1.rows = 2; m1.cols = 2;
    m1.data[0][0] = 1.0f; m1.data[0][1] = 2.0f;
    m1.data[1][0] = 3.0f; m1.data[1][1] = 4.0f;

    memset(&m2, 0, sizeof(m2));
    m2.rows = 2; m2.cols = 2;
    m2.data[0][0] = 2.0f; m2.data[0][1] = 0.0f;
    m2.data[1][0] = 1.0f; m2.data[1][1] = 2.0f;

    m_res = multiply_matrices(&m1, &m2);
    assert(NUR_IS_OK(m_res));
    out = NUR_VALUE(m_res);
    assert(out.data[0][0] == 4.0f && out.data[0][1] == 4.0f);
    assert(out.data[1][0] == 10.0f && out.data[1][1] == 8.0f);

    /* 4. Domain 2: Tight loop with single entry check */
    audio[0] = 1.0f; audio[1] = 2.0f; audio[2] = 3.0f; audio[3] = 4.0f;
    scale_res = scale_buffer(audio, 4, 0.5f);
    assert(NUR_IS_OK(scale_res));
    assert(audio[0] == 0.5f && audio[3] == 2.0f);

    /* 5. Domain 3: Setters and Getters */
    memset(&srv, 0, sizeof(srv));
    srv.id = 1;
    srv.port = 80;
    srv.has_nickname = false;
    assert(NUR_IS_OK(server_set_port(&srv, 8080)));
    assert(srv.port == 8080);
    assert(NUR_IS_ERR(server_set_port(&srv, 999999)));

    nick_opt = server_get_nickname(&srv);
    assert(NUR_IS_NONE(nick_opt));

    /* 6. Domain 4: Hardware ISR */
    mock_uart.status_reg = 0x01;
    mock_uart.data_reg = 0x42;
    isr_uart_rx_handler(&mock_uart, &byte_val, &err_flags);
    assert(byte_val == 0x42 && err_flags == 0);

    puts("All super-header & drop-in tests PASSED cleanly! [OK]");
    return 0;
}
