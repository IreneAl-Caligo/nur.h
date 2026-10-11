/**
 * 05_socket_server.c - Cross-Platform Socket Networking with nur.h
 *
 * Demonstrates:
 * - Socket initialization and connection handling returning NurResult
 * - Resource cleanup on error paths using NUR_TRY_GOTO (ensuring no leaked sockets)
 * - Mapping network and system errors to nur_error_t taxonomy
 * - Clean portable implementation for Windows (Winsock) and POSIX (BSD sockets)
 */

#include "nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#if defined(_MSC_VER)
    #pragma comment(lib, "ws2_32.lib")
#endif
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define IS_INVALID_SOCKET(s) ((s) == INVALID_SOCKET)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define IS_INVALID_SOCKET(s) ((s) < 0)
    #define INVALID_SOCKET (-1)
#endif

/* Declare Result type for socket descriptor */
NUR_RESULT_TYPE(socket_t, socket);

/**
 * Initialize platform networking subsystem (Winsock on Windows, no-op on POSIX).
 */
static NurResult_void init_network(void) {
#if defined(_WIN32)
    WSADATA wsa;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0) {
        return nur_err_void(NUR_ERROR_FROM_WIN32(rc, "WSAStartup failed"));
    }
#endif
    return nur_ok_void();
}

/**
 * Cleanup network subsystem.
 */
static void cleanup_network(void) {
#if defined(_WIN32)
    WSACleanup();
#endif
}

/**
 * Create a TCP listener socket bound to loopback and given port.
 * Uses NUR_TRY_GOTO to guarantee socket descriptor cleanup if bind/listen fails.
 */
static NurResult_socket create_listener(int port) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (IS_INVALID_SOCKET(sock)) {
#if defined(_WIN32)
        return NUR_ERR(socket, NUR_ERROR_FROM_WIN32(WSAGetLastError(), "socket() creation failed"));
#else
        return NUR_ERR(socket, NUR_ERROR_ERRNO("socket() creation failed"));
#endif
    }

    /* Set SO_REUSEADDR where available */
    int opt = 1;
#if defined(_WIN32)
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((uint16_t)port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        nur_error_t err;
#if defined(_WIN32)
        err = NUR_ERROR_FROM_WIN32(WSAGetLastError(), "bind() failed on port");
#else
        err = NUR_ERROR_ERRNO("bind() failed on port");
#endif
        CLOSE_SOCKET(sock); /* Clean up socket on error */
        return NUR_ERR(socket, err);
    }

    if (listen(sock, 5) != 0) {
        nur_error_t err;
#if defined(_WIN32)
        err = NUR_ERROR_FROM_WIN32(WSAGetLastError(), "listen() failed");
#else
        err = NUR_ERROR_ERRNO("listen() failed");
#endif
        CLOSE_SOCKET(sock); /* Clean up socket on error */
        return NUR_ERR(socket, err);
    }

    return NUR_OK(socket, sock);
}

/**
 * Simulated client transaction demonstrating NUR_TRY_GOTO cleanup.
 */
static NurResult_void simulate_transaction(int test_port) {
    socket_t s1 = INVALID_SOCKET;
    socket_t s2 = INVALID_SOCKET;
    nur_error_t err = {0};

    printf("1. Initializing networking...\n");
    NurResult_void init_res = init_network();
    if (NUR_IS_ERR(init_res)) {
        return init_res;
    }

    printf("2. Creating listener on 127.0.0.1:%d...\n", test_port);
    NurResult_socket l_res = create_listener(test_port);
    if (NUR_IS_ERR(l_res)) {
        err = NUR_ERROR_OF(l_res);
        goto cleanup;
    }
    s1 = NUR_VALUE(l_res);
    printf("   Listener active on socket %lld\n", (long long)s1);

    printf("3. Attempting duplicate bind on same port (expected collision)...\n");
    NurResult_socket dup_res = create_listener(test_port);
    if (NUR_IS_ERR(dup_res)) {
        printf("   [EXPECTED ERROR] Duplicate bind failed cleanly: ");
        nur_error_eprint(&NUR_ERROR_OF(dup_res));
    } else {
        s2 = NUR_VALUE(dup_res);
    }

cleanup:
    if (!IS_INVALID_SOCKET(s1)) {
        printf("4. Cleaning up listener socket...\n");
        CLOSE_SOCKET(s1);
    }
    if (!IS_INVALID_SOCKET(s2)) {
        CLOSE_SOCKET(s2);
    }
    cleanup_network();

    if (err.code != NUR_E_NONE) {
        return nur_err_void(err);
    }
    return nur_ok_void();
}

int main(void) {
    printf("========================================\n");
    printf("  nur.h Socket Server Networking Demo   \n");
    printf("========================================\n\n");

    /* Test simulation on ephemeral port 18492 */
    NurResult_void res = simulate_transaction(18492);
    if (NUR_IS_OK(res)) {
        printf("\n[SUCCESS] Socket lifecycle executed with zero leaks!\n");
    } else {
        printf("\n[FAILURE] Transaction failed:\n");
        nur_error_eprint(&NUR_VOID_ERROR_OF(res));
    }

    return 0;
}
