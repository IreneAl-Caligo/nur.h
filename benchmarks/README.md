# nur.h Benchmarks

This directory contains performance benchmarks for the nur.h library, measuring the overhead of Rust-style error handling in C.

## Overview

The benchmark suite measures timing for core nur.h operations and compares them to baseline C operations to demonstrate the near-zero overhead of the library's static inline optimizations.

## Compilation

### Requirements
- C11-compliant compiler (GCC, Clang, or MSVC)
- The nur.h header file in `../src/nur.h`

### Build Commands

**GCC/Clang:**
```bash
gcc -std=c11 -O2 -Wall -Wextra -Isrc/ benchmarks/bench_nur.c -o bench_nur
clang -std=c11 -O2 -Wall -Wextra -Isrc/ benchmarks/bench_nur.c -o bench_nur
```

**Microsoft Visual C++:**
```cmd
cl /std:c11 /W4 /O2 /Isrc/ benchmarks/bench_nur.c /Fe:bench_nur.exe
```

**Cross-platform Makefile target:**
```make
bench_nur: benchmarks/bench_nur.c src/nur.h
	$(CC) -std=c11 -O2 -Wall -Wextra -Isrc/ benchmarks/bench_nur.c -o bench_nur
```

## Running Benchmarks

```bash
./bench_nur
```

The program will output a results table showing:
- Benchmark name
- Number of operations performed  
- Total time in milliseconds
- Nanoseconds per operation
- Overhead percentage compared to baseline

## Benchmark Categories

### 1. Result Construction Throughput
- **Result<i32> ok construction**: Creates 10M successful `NurResult_i32` values
- **Result<i32> err construction**: Creates 10M error `NurResult_i32` values  
- **Measures**: Constructor function overhead

### 2. Result Checking Throughput  
- **Result<i32> is_ok check**: Performs 10M `NUR_IS_OK()` checks on a pre-built result
- **Measures**: Branch prediction and bool extraction overhead

### 3. Error Propagation Throughput
- **Error propagation (3-level)**: Tests `NUR_TRY_TO` through a 3-function call chain, 1M times
- **Measures**: Macro expansion and early return overhead

### 4. Option Construction Throughput
- **Option<i32> some construction**: Creates 10M `NurOption_i32` some values
- **Option<i32> none construction**: Creates 10M `NurOption_i32` none values
- **Measures**: Optional type overhead

### 5. Baseline Comparison
- **Baseline arithmetic**: Raw integer arithmetic without nur.h wrapping, 10M times
- **Purpose**: Establishes performance baseline to measure nur.h overhead

## Expected Results

With `-O2` optimization, expected overhead should be:
- **0-5%**: Result/Option construction (excellent)
- **0-2%**: Result checking (near-zero due to inlining)
- **0-10%**: Error propagation (good, considering macro complexity)
- **Negative**: Possible when compiler optimizations favor nur.h patterns

## Result Interpretation

### Near-Zero Overhead
Results showing 0-5% overhead indicate the compiler successfully inlined nur.h operations, achieving the zero-cost abstraction goal.

### Negative Overhead
When nur.h shows better performance than baseline, it usually indicates:
- Compiler optimizations work better with structured code patterns
- Branch prediction improvements from consistent error handling
- Dead code elimination in optimized builds

### High Overhead (>20%)
If overhead exceeds 20%, check:
- Optimization level (`-O2` recommended)
- Compiler version (older compilers may not inline aggressively)
- Debug builds (add `-DNDEBUG` for release builds)

## Adding New Benchmarks

### 1. Create Benchmark Function
```c
static void bench_new_feature(size_t iterations) {
    uint64_t sink = g_sink;
    
    for (size_t i = 0; i < iterations; i++) {
        // Your nur.h operation here
        NurResult_i32 result = your_operation();
        sink ^= (uint64_t)some_result_value;
    }
    
    g_sink = sink;
}
```

### 2. Add to Main Function
```c
printf("Running new feature benchmark...\n");
NurResult_void new_result = run_benchmark("New Feature Description",
                                        BENCH_ITERATIONS_10M,
                                        bench_new_feature, false,
                                        &results[result_count]);
if (NUR_IS_ERR(new_result)) {
    nur_error_eprint(&NUR_ERROR_OF(new_result));
    return 1;
}
result_count++;
```

### 3. Update Array Size
Increase `BENCH_MAX_RESULTS` if adding more than 16 total benchmarks.

## Implementation Notes

### Memory Safety
- All pointer parameters are validated for NULL
- Array bounds are checked with compile-time constants
- Integer overflow is prevented in timing calculations
- Results use `const` qualifiers where appropriate

### Standards Compliance
- Compatible with C11, C17, and C23 standards
- Portable timing using platform-specific high-resolution timers
- Proper format specifiers (`%zu` for `size_t`)
- No undefined behavior in arithmetic operations

### Error Handling
- All functions return `NurResult_*` types following nur.h conventions
- Errors are properly propagated and printed with source location
- No raw return codes or exception-based error handling

### Anti-Optimization Measures
- Global volatile sink prevents dead code elimination
- XOR operations ensure all computed values are "used"
- Median of multiple runs reduces timing variance
- Benchmark isolation prevents cross-contamination

## CI Integration

These benchmarks run automatically on:
- Every push to main branch
- Pull requests modifying nur.h
- Nightly performance regression testing

Results are tracked over time to catch performance regressions in library changes.

## Troubleshooting

### Compilation Errors
- Ensure nur.h path is correct in `-I` flag
- Check C11 support in your compiler
- Add `-lm` flag if math library linking fails

### Inconsistent Results  
- Run with consistent CPU load
- Disable CPU frequency scaling during benchmarks
- Increase `BENCH_RUNS` for more stable medians
- Use `taskset` on Linux to pin to specific CPU cores

### Platform-Specific Issues
- Windows: Requires QueryPerformanceCounter for high precision
- macOS: May need additional timing permissions
- Linux: CLOCK_MONOTONIC should be available on kernel 2.6+