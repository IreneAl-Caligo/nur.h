/**
 * 07_thread_safe_logging.c - Thread-Safe Structured Logging Subsystem
 *
 * Demonstrates:
 * - Thread-safe error reporting using nur_error_json and nur_error_format
 * - Cross-platform synchronization (CRITICAL_SECTION on Windows, pthread_mutex on POSIX)
 * - Multi-threaded worker tasks returning NurResult and logging structured diagnostics
 * - Zero dynamic heap allocation in the error recording path
 */

#include "nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
    #include <windows.h>
    typedef CRITICAL_SECTION mutex_t;
    #define MUTEX_INIT(m) InitializeCriticalSection(&(m))
    #define MUTEX_LOCK(m) EnterCriticalSection(&(m))
    #define MUTEX_UNLOCK(m) LeaveCriticalSection(&(m))
    #define MUTEX_DESTROY(m) DeleteCriticalSection(&(m))
#else
    #include <pthread.h>
    typedef pthread_mutex_t mutex_t;
    #define MUTEX_INIT(m) pthread_mutex_init(&(m), NULL)
    #define MUTEX_LOCK(m) pthread_mutex_lock(&(m))
    #define MUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))
    #define MUTEX_DESTROY(m) pthread_mutex_destroy(&(m))
#endif

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

typedef struct Logger {
    mutex_t lock;
    FILE *sink;
    int enable_json;
} Logger;

static Logger g_logger;

/**
 * Initialize global logger.
 */
static void logger_init(FILE *sink, int enable_json) {
    MUTEX_INIT(g_logger.lock);
    g_logger.sink = sink ? sink : stderr;
    g_logger.enable_json = enable_json;
}

/**
 * Destroy global logger.
 */
static void logger_destroy(void) {
    MUTEX_DESTROY(g_logger.lock);
}

/**
 * Log an error in structured JSON or standard format.
 */
static void log_nur_error(LogLevel level, const nur_error_t *err) {
    if (!err) return;

    char buf[512];
    MUTEX_LOCK(g_logger.lock);

    if (g_logger.enable_json) {
        nur_error_json(err, buf, sizeof(buf));
        const char *level_str = (level == LOG_LEVEL_ERROR) ? "ERROR" : "WARN";
        fprintf(g_logger.sink, "{\"level\":\"%s\",\"error\":%s}\n", level_str, buf);
    } else {
        nur_error_format(err, buf, sizeof(buf));
        fprintf(g_logger.sink, "[LOG] %s\n", buf);
    }

    fflush(g_logger.sink);
    MUTEX_UNLOCK(g_logger.lock);
}

/**
 * Fallible worker operation.
 */
static NurResult_i32 process_job(int job_id, int should_fail) {
    if (should_fail) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_BUSY, "Worker queue overloaded", "job_queue_3"));
    }
    return NUR_OK(i32, job_id * 10);
}

/**
 * Worker runner.
 */
static void run_worker_task(int worker_id, int job_id, int should_fail) {
    NurResult_i32 res = process_job(job_id, should_fail);

    if (NUR_IS_OK(res)) {
        MUTEX_LOCK(g_logger.lock);
        fprintf(g_logger.sink, "[INFO] Worker %d completed job %d with result: %d\n",
                worker_id, job_id, NUR_VALUE(res));
        MUTEX_UNLOCK(g_logger.lock);
    } else {
        log_nur_error(LOG_LEVEL_ERROR, &NUR_ERROR_OF(res));
    }
}

int main(void) {
    printf("============================================\n");
    printf("  nur.h Thread-Safe Structured Logging Demo \n");
    printf("============================================\n\n");

    /* 1. Standard text logging demonstration */
    printf("--- Part 1: Human-Readable Format ---\n");
    logger_init(stdout, 0 /* text format */);

    run_worker_task(1, 101, 0 /* success */);
    run_worker_task(2, 102, 1 /* failure */);
    logger_destroy();

    /* 2. Structured JSON logging demonstration (ideal for log collectors / AI analysis) */
    printf("\n--- Part 2: Machine-Readable RFC 8259 JSON Format ---\n");
    logger_init(stdout, 1 /* JSON format */);

    run_worker_task(3, 201, 0 /* success */);
    run_worker_task(4, 202, 1 /* failure */);
    logger_destroy();

    printf("\n=== Logging Subsystem Demo Completed Successfully ===\n");
    return 0;
}
