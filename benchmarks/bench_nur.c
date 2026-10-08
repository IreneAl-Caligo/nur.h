/*
 * bench_nur.c - Performance benchmarks for nur.h library
 * 
 * Measures the overhead of nur.h error handling compared to raw C operations.
 * Expected result: near-zero overhead due to static inline optimizations.
 * 
 * Compile with: gcc -std=c11 -O2 -Wall -Isrc/ benchmarks/bench_nur.c -o bench_nur
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>

/* C standard version detection */
#if __STDC_VERSION__ >= 201112L
    #define BENCH_C11_AVAILABLE 1
#endif

#if __STDC_VERSION__ >= 202311L
    #define BENCH_C23_AVAILABLE 1
#endif

/* Platform-specific timing with proper C11/C23 compatibility */
#ifdef _WIN32
    #include <windows.h>
    #define BENCH_CLOCK_MONOTONIC 0
    
    /* Use C23 timespec if available, otherwise define our own with unique name */
    #ifndef BENCH_C23_AVAILABLE
        typedef struct bench_timespec {
            time_t tv_sec;
            long tv_nsec;
        } bench_timespec_t;
    #else
        typedef struct timespec bench_timespec_t;
    #endif
    
    static int bench_clock_gettime(int clk_id, bench_timespec_t *spec) {
        if (!spec) return -1;
        (void)clk_id; /* Unused parameter */
        
        LARGE_INTEGER freq, count;
        if (!QueryPerformanceFrequency(&freq) || !QueryPerformanceCounter(&count)) {
            return -1;
        }
        
        spec->tv_sec = (time_t)(count.QuadPart / freq.QuadPart);
        spec->tv_nsec = (long)((count.QuadPart % freq.QuadPart) * 1000000000LL / freq.QuadPart);
        return 0;
    }
#else
    #include <time.h>
    typedef struct timespec bench_timespec_t;
    #ifdef CLOCK_MONOTONIC
        #define BENCH_CLOCK_MONOTONIC CLOCK_MONOTONIC
        #define bench_clock_gettime clock_gettime
    #else
        #define BENCH_CLOCK_MONOTONIC 1
        /* Fallback implementation for systems without POSIX timers */
        static int bench_clock_gettime(int clk_id, bench_timespec_t *spec) {
            if (!spec) return -1;
            (void)clk_id;
            
            clock_t c = clock();
            if (c == (clock_t)-1) return -1;
            
            spec->tv_sec = c / CLOCKS_PER_SEC;
            spec->tv_nsec = (c % CLOCKS_PER_SEC) * (1000000000L / CLOCKS_PER_SEC);
            return 0;
        }
    #endif
#endif

/* Include nur.h after platform setup */
#include "nur.h"

/* Benchmark configuration */
#define BENCH_ITERATIONS_10M  10000000
#define BENCH_ITERATIONS_1M   1000000
#define BENCH_RUNS            3

/* Global sink to prevent dead code elimination */
static volatile uint64_t g_sink = 0;

/* Timing utilities with proper error handling */
typedef struct {
    bench_timespec_t start;
    bench_timespec_t end;
} timer_t;

/* Timer functions return NurResult_void for proper error handling */
static NurResult_void timer_start(timer_t *t) {
    if (!t) {
        return nur_err_void(NUR_ERROR(NUR_E_NULL_POINTER, "timer pointer is NULL"));
    }
    
    if (bench_clock_gettime(BENCH_CLOCK_MONOTONIC, &t->start) != 0) {
        return nur_err_void(NUR_ERROR(NUR_E_IO, "failed to get start time"));
    }
    
    return nur_ok_void();
}

static NurResult_usize timer_end_ns(timer_t *t) {
    if (!t) {
        return nur_err_usize(NUR_ERROR(NUR_E_NULL_POINTER, "timer pointer is NULL"));
    }
    
    if (bench_clock_gettime(BENCH_CLOCK_MONOTONIC, &t->end) != 0) {
        return nur_err_usize(NUR_ERROR(NUR_E_IO, "failed to get end time"));
    }
    
    /* Safe arithmetic with overflow checks */
    uint64_t start_ns = (uint64_t)t->start.tv_sec * 1000000000ULL;
    uint64_t end_ns = (uint64_t)t->end.tv_sec * 1000000000ULL;
    
    /* Check for overflow in nanosecond addition */
    if (start_ns > UINT64_MAX - (uint64_t)t->start.tv_nsec ||
        end_ns > UINT64_MAX - (uint64_t)t->end.tv_nsec) {
        return nur_err_usize(NUR_ERROR(NUR_E_OVERFLOW, "timestamp overflow"));
    }
    
    start_ns += (uint64_t)t->start.tv_nsec;
    end_ns += (uint64_t)t->end.tv_nsec;
    
    if (end_ns < start_ns) {
        return nur_err_usize(NUR_ERROR(NUR_E_INVALID_STATE, "end time before start time"));
    }
    
    return nur_ok_usize((size_t)(end_ns - start_ns));
}

/* Benchmark results with safe bounds */
#define BENCH_MAX_RESULTS 16  /* Allow room for expansion */

typedef struct {
    const char *name;
    size_t iterations;
    size_t time_ns;
    double ns_per_op;
} bench_result_t;

/* Helper to run benchmark multiple times and return median with proper error handling */
static NurResult_void run_benchmark(const char *name, size_t iterations, 
                                  void (*bench_func)(size_t), bool is_baseline,
                                  bench_result_t *out_result) {
    if (!name || !bench_func || !out_result) {
        return nur_err_void(NUR_ERROR(NUR_E_NULL_POINTER, "NULL parameter to run_benchmark"));
    }
    (void)is_baseline;
    
    if (iterations == 0) {
        return nur_err_void(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "iterations cannot be zero"));
    }
    
    size_t times[BENCH_RUNS];
    
    for (int run = 0; run < BENCH_RUNS; run++) {
        timer_t timer;
        
        /* Start timer with error checking */
        NurResult_void start_result = timer_start(&timer);
        if (NUR_IS_ERR(start_result)) {
            return start_result;
        }
        
        /* Run the benchmark */
        bench_func(iterations);
        
        /* End timer with error checking */
        NurResult_usize end_result = timer_end_ns(&timer);
        if (NUR_IS_ERR(end_result)) {
            return nur_err_void(NUR_ERROR_OF(end_result));
        }
        
        times[run] = NUR_VALUE(end_result);
    }
    
    /* Simple bubble sort to find median - safe with fixed array size */
    for (int i = 0; i < BENCH_RUNS - 1; i++) {
        for (int j = 0; j < BENCH_RUNS - 1 - i; j++) {
            if (times[j] > times[j + 1]) {
                size_t temp = times[j];
                times[j] = times[j + 1];
                times[j + 1] = temp;
            }
        }
    }
    
    size_t median_time = times[BENCH_RUNS / 2];
    double ns_per_op = (double)median_time / (double)iterations;
    
    /* Populate result structure */
    out_result->name = name;
    out_result->iterations = iterations;
    out_result->time_ns = median_time;
    out_result->ns_per_op = ns_per_op;
    
    return nur_ok_void();
}

/* Benchmark functions - updated to use size_t parameters */

static void bench_result_ok_construction(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        /* Safe cast from size_t to int32_t for benchmark values */
        int32_t val = (int32_t)(i & 0x7FFFFFFF); /* Keep within int32_t range */
        NurResult_i32 result = nur_ok_i32(val);
        sink ^= (uint64_t)NUR_VALUE(result);
    }
    
    g_sink = sink;
}

static void bench_result_err_construction(size_t iterations) {
    uint64_t sink = g_sink;
    nur_error_t err = NUR_ERROR(NUR_E_INVALID_ARGUMENT, "benchmark error");
    
    for (size_t i = 0; i < iterations; i++) {
        NurResult_i32 result = nur_err_i32(err);
        sink ^= (uint64_t)result.ok;
    }
    
    g_sink = sink;
}

static void bench_result_check(size_t iterations) {
    uint64_t sink = g_sink;
    NurResult_i32 result = nur_ok_i32(42);
    
    for (size_t i = 0; i < iterations; i++) {
        bool is_ok = NUR_IS_OK(result);
        sink ^= (uint64_t)is_ok;
    }
    
    g_sink = sink;
}

/* Test function for error propagation benchmark */
static NurResult_i32 test_func_level3(int32_t val) {
    if (val < 0) {
        return nur_err_i32(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "negative value"));
    }
    return nur_ok_i32(val * 3);
}

static NurResult_i32 test_func_level2(int32_t val) {
    int32_t result;
    NUR_TRY_TO(i32, i32, result, test_func_level3(val));
    return nur_ok_i32(result * 2);
}

static NurResult_i32 test_func_level1(int32_t val) {
    int32_t result;
    NUR_TRY_TO(i32, i32, result, test_func_level2(val));
    return nur_ok_i32(result * 1);
}

static void bench_error_propagation(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        /* Keep value positive and within int32_t range */
        int32_t val = (int32_t)((i % 1000) + 1); /* Always positive, 1-1000 */
        NurResult_i32 result = test_func_level1(val);
        if (NUR_IS_OK(result)) {
            sink ^= (uint64_t)NUR_VALUE(result);
        }
    }
    
    g_sink = sink;
}

static void bench_option_some_construction(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        /* Safe cast from size_t to int32_t */
        int32_t val = (int32_t)(i & 0x7FFFFFFF);
        NurOption_i32 option = nur_some_i32(val);
        sink ^= (uint64_t)NUR_OPTION_VALUE(option);
    }
    
    g_sink = sink;
}

static void bench_option_none_construction(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        NurOption_i32 option = nur_none_i32();
        sink ^= (uint64_t)option.some;
    }
    
    g_sink = sink;
}

static void bench_baseline_arithmetic(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        /* Safe cast and arithmetic matching error propagation test */
        int32_t val = (int32_t)((i % 1000) + 1);
        int32_t result = val * 6; /* Same computation as error propagation test */
        sink ^= (uint64_t)result;
    }
    
    g_sink = sink;
}

/* Print benchmark results table with proper format specifiers and safety checks */
static NurResult_void print_results(const bench_result_t *results, size_t count, double baseline_ns_per_op) {
    if (!results) {
        return nur_err_void(NUR_ERROR(NUR_E_NULL_POINTER, "results array is NULL"));
    }
    
    if (count == 0) {
        return nur_err_void(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "result count cannot be zero"));
    }
    
    if (count > BENCH_MAX_RESULTS) {
        return nur_err_void(NUR_ERROR(NUR_E_OUT_OF_RANGE, "result count exceeds maximum"));
    }
    
    printf("Benchmark Results\n");
    printf("=================\n\n");
    printf("%-30s %10s %12s %12s %12s\n", 
           "Benchmark", "Ops", "Total (ms)", "ns/op", "vs Baseline");
    printf("%-30s %10s %12s %12s %12s\n", 
           "----------", "---", "----------", "-----", "-----------");
    
    for (size_t i = 0; i < count; i++) {
        const bench_result_t *r = &results[i];
        
        /* Validate individual result */
        if (!r->name) {
            return nur_err_void(NUR_ERROR(NUR_E_NULL_POINTER, "benchmark name is NULL"));
        }
        
        double total_ms = (double)r->time_ns / 1000000.0;
        
        if (baseline_ns_per_op > 0.0) {
            double overhead_pct = ((r->ns_per_op - baseline_ns_per_op) / baseline_ns_per_op) * 100.0;
            printf("%-30s %10zu %12.3f %12.3f %10.1f%%\n",
                   r->name, r->iterations, total_ms, r->ns_per_op, overhead_pct);
        } else {
            printf("%-30s %10zu %12.3f %12.3f %12s\n",
                   r->name, r->iterations, total_ms, r->ns_per_op, "baseline");
        }
    }
    
    printf("\nNotes:\n");
    printf("- Compiled with -O2 optimization\n");
    printf("- Results show median of %d runs\n", BENCH_RUNS);
    printf("- Expected: near-zero overhead due to static inline optimization\n");
    printf("- Negative overhead indicates nur.h is faster (compiler optimization effects)\n");
    
    return nur_ok_void();
}

int main(void) {
    printf("nur.h Performance Benchmarks\n");
    printf("============================\n\n");
    printf("Measuring overhead of Rust-style error handling in C...\n\n");
    
    bench_result_t results[BENCH_MAX_RESULTS];
    size_t result_count = 0;
    
    /* Run baseline first to establish reference */
    printf("Running baseline benchmark...\n");
    NurResult_void baseline_result = run_benchmark("Baseline arithmetic", 
                                                  BENCH_ITERATIONS_10M, 
                                                  bench_baseline_arithmetic, true,
                                                  &results[result_count]);
    if (NUR_IS_ERR(baseline_result)) {
        nur_error_eprint(&NUR_ERROR_OF(baseline_result));
        return 1;
    }
    result_count++;
    
    double baseline_ns_per_op = results[0].ns_per_op;
    
    /* Run nur.h benchmarks with proper error handling */
    printf("Running Result<T> construction (ok)...\n");
    NurResult_void ok_result = run_benchmark("Result<i32> ok construction", 
                                           BENCH_ITERATIONS_10M, 
                                           bench_result_ok_construction, false,
                                           &results[result_count]);
    if (NUR_IS_ERR(ok_result)) {
        nur_error_eprint(&NUR_ERROR_OF(ok_result));
        return 1;
    }
    result_count++;
    
    printf("Running Result<T> construction (err)...\n");
    NurResult_void err_result = run_benchmark("Result<i32> err construction", 
                                            BENCH_ITERATIONS_10M, 
                                            bench_result_err_construction, false,
                                            &results[result_count]);
    if (NUR_IS_ERR(err_result)) {
        nur_error_eprint(&NUR_ERROR_OF(err_result));
        return 1;
    }
    result_count++;
    
    printf("Running Result<T> checks...\n");
    NurResult_void check_result = run_benchmark("Result<i32> is_ok check", 
                                              BENCH_ITERATIONS_10M, 
                                              bench_result_check, false,
                                              &results[result_count]);
    if (NUR_IS_ERR(check_result)) {
        nur_error_eprint(&NUR_ERROR_OF(check_result));
        return 1;
    }
    result_count++;
    
    printf("Running error propagation...\n");
    NurResult_void prop_result = run_benchmark("Error propagation (3-level)", 
                                             BENCH_ITERATIONS_1M, 
                                             bench_error_propagation, false,
                                             &results[result_count]);
    if (NUR_IS_ERR(prop_result)) {
        nur_error_eprint(&NUR_ERROR_OF(prop_result));
        return 1;
    }
    result_count++;
    
    printf("Running Option<T> construction (some)...\n");
    NurResult_void some_result = run_benchmark("Option<i32> some construction", 
                                             BENCH_ITERATIONS_10M, 
                                             bench_option_some_construction, false,
                                             &results[result_count]);
    if (NUR_IS_ERR(some_result)) {
        nur_error_eprint(&NUR_ERROR_OF(some_result));
        return 1;
    }
    result_count++;
    
    printf("Running Option<T> construction (none)...\n");
    NurResult_void none_result = run_benchmark("Option<i32> none construction", 
                                             BENCH_ITERATIONS_10M, 
                                             bench_option_none_construction, false,
                                             &results[result_count]);
    if (NUR_IS_ERR(none_result)) {
        nur_error_eprint(&NUR_ERROR_OF(none_result));
        return 1;
    }
    result_count++;
    
    printf("\n");
    NurResult_void print_result = print_results(results, result_count, baseline_ns_per_op);
    if (NUR_IS_ERR(print_result)) {
        nur_error_eprint(&NUR_ERROR_OF(print_result));
        return 1;
    }
    
    return 0;
}