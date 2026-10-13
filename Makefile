# Makefile for nur.h Library
# Supports multiple C standards and comprehensive testing

# Compiler settings
CC := gcc
CFLAGS_BASE := -Wall -Wextra -Wpedantic -I. -Isrc
CFLAGS_DEBUG := $(CFLAGS_BASE) -g -O0 -DDEBUG
CFLAGS_RELEASE := $(CFLAGS_BASE) -O2 -DNDEBUG

# C Standard variants
CFLAGS_C89 := $(CFLAGS_DEBUG) -std=c89
CFLAGS_C99 := $(CFLAGS_DEBUG) -std=c99
CFLAGS_C11 := $(CFLAGS_DEBUG) -std=c11  
CFLAGS_C17 := $(CFLAGS_DEBUG) -std=c17
CFLAGS_C23 := $(CFLAGS_DEBUG) -std=c23

# Platform detection
ifeq ($(OS),Windows_NT)
    EXE := .exe
    RUN_PREFIX := 
else
    EXE :=
    RUN_PREFIX := ./
endif

# Test executables
TEST_BINARY := test_nur$(EXE)
TEST_BINARY_C89 := test_c89_and_purist$(EXE)
TEST_BINARY_C99 := test_nur_c99$(EXE)
TEST_BINARY_C11 := test_nur_c11$(EXE)
TEST_BINARY_C17 := test_nur_c17$(EXE)
TEST_BINARY_C23 := test_nur_c23$(EXE)
C23_TEST_BINARY := test_c23_features$(EXE)

# Source files
HEADER := src/nur.h
TEST_SRC := test/test_nur.c
C23_TEST_SRC := test/test_c23_features.c

# Default target
.PHONY: all
all: test

# Syntax check only (no linking)
.PHONY: check
check: check-c89 check-c99 check-c11 check-c17 check-c23

.PHONY: check-c89
check-c89:
	@echo "Checking C89 compatibility..."
	$(CC) $(CFLAGS_C89) -fsyntax-only $(HEADER)

.PHONY: check-c99
check-c99:
	@echo "Checking C99 compatibility..."
	$(CC) $(CFLAGS_C99) -fsyntax-only $(HEADER)

.PHONY: check-c11  
check-c11:
	@echo "Checking C11 compatibility..."
	$(CC) $(CFLAGS_C11) -fsyntax-only $(HEADER)

.PHONY: check-c17
check-c17:
	@echo "Checking C17 compatibility..."
	$(CC) $(CFLAGS_C17) -fsyntax-only $(HEADER)

.PHONY: check-c23
check-c23:
	@echo "Checking C23 compatibility..."
	$(CC) $(CFLAGS_C23) -fsyntax-only $(HEADER)

# Build test executable (default C11)
$(TEST_BINARY): $(TEST_SRC) $(HEADER)
	@echo "Building test suite (C11)..."
	$(CC) $(CFLAGS_C11) -o $@ $(TEST_SRC)

# Build test executables for all standards
$(TEST_BINARY_C99): $(TEST_SRC) $(HEADER)
	$(CC) $(CFLAGS_C99) -o $@ $(TEST_SRC)

$(TEST_BINARY_C11): $(TEST_SRC) $(HEADER)
	$(CC) $(CFLAGS_C11) -o $@ $(TEST_SRC)

$(TEST_BINARY_C17): $(TEST_SRC) $(HEADER)  
	$(CC) $(CFLAGS_C17) -o $@ $(TEST_SRC)

$(TEST_BINARY_C23): $(TEST_SRC) $(HEADER)
	$(CC) $(CFLAGS_C23) -o $@ $(TEST_SRC)

# Build C23 feature test executable
$(C23_TEST_BINARY): $(C23_TEST_SRC) $(HEADER)
	@echo "Building C23 feature test suite..."
	$(CC) $(CFLAGS_C23) -o $@ $(C23_TEST_SRC)

# Run tests
.PHONY: test
test: $(TEST_BINARY) test-universal test-all-features
	@echo "Running test suite..."
	$(RUN_PREFIX)$(TEST_BINARY)

.PHONY: test-universal
test-universal: $(HEADER)
	@echo "Running universal type & batteries-included test..."
	$(CC) $(CFLAGS_C11) -o test/test_universal_type$(EXE) test/test_universal_type.c
	$(RUN_PREFIX)test/test_universal_type$(EXE)

.PHONY: test-all-features
test-all-features: $(HEADER)
	@echo "Running complete A-to-Z features and error suite..."
	$(CC) $(CFLAGS_C11) -o test/test_all_features_and_errors$(EXE) test/test_all_features_and_errors.c
	$(RUN_PREFIX)test/test_all_features_and_errors$(EXE)

# Run tests for all C standards
.PHONY: test-all
test-all: test-c89 test-c99 test-c11 test-c17 test-c23

.PHONY: test-c89
test-c89: $(HEADER)
	@echo "Running C89 tests..."
	$(CC) $(CFLAGS_C89) -o $(TEST_BINARY_C89) TEST/test_c89_and_purist.c
	$(RUN_PREFIX)$(TEST_BINARY_C89)

.PHONY: test-purist
test-purist: $(HEADER)
	@echo "Running C-Purist pure-function tests..."
	$(CC) $(CFLAGS_C11) -o $(TEST_BINARY_C89) TEST/test_c89_and_purist.c
	$(RUN_PREFIX)$(TEST_BINARY_C89)

.PHONY: test-c99
test-c99: $(TEST_BINARY_C99)
	@echo "Running C99 tests..."
	$(RUN_PREFIX)$(TEST_BINARY_C99)

.PHONY: test-c11
test-c11: $(TEST_BINARY_C11)
	@echo "Running C11 tests..."
	$(RUN_PREFIX)$(TEST_BINARY_C11)

.PHONY: test-c17
test-c17: $(TEST_BINARY_C17)
	@echo "Running C17 tests..."
	$(RUN_PREFIX)$(TEST_BINARY_C17)

.PHONY: test-c23
test-c23: $(TEST_BINARY_C23)
	@echo "Running C23 tests..."
	$(RUN_PREFIX)$(TEST_BINARY_C23)

# Run C23 feature tests
.PHONY: test-c23-features
test-c23-features: $(C23_TEST_BINARY)
	@echo "Running C23 feature validation tests..."
	$(RUN_PREFIX)$(C23_TEST_BINARY)

.PHONY: test-result-helpers
test-result-helpers: $(HEADER)
	@echo "Running result helpers test..."
	$(CC) $(CFLAGS_C11) -o test/test_result_helpers$(EXE) test/test_result_helpers.c
	$(RUN_PREFIX)test/test_result_helpers$(EXE)

.PHONY: test-error-wrap
test-error-wrap: $(HEADER)
	@echo "Running error wrap test..."
	$(CC) $(CFLAGS_C11) -o test/test_error_wrap$(EXE) test/test_error_wrap.c
	$(RUN_PREFIX)test/test_error_wrap$(EXE)

.PHONY: test-ai-contract
test-ai-contract: $(HEADER)
	@echo "Running AI contract test..."
	$(CC) $(CFLAGS_C11) -o test/test_ai_contract_examples$(EXE) test/test_ai_contract_examples.c
	$(RUN_PREFIX)test/test_ai_contract_examples$(EXE)

.PHONY: test-compile-fail
test-compile-fail: $(HEADER)
	@echo "Verifying compile-fail nodiscard warning..."
ifeq ($(OS),Windows_NT)
	@cmd /C "$(CC) $(CFLAGS_BASE) -std=c11 -Werror=unused-result -c test/test_compile_fail_nodiscard.c -o test_tmp.o 2>nul && exit /b 1 || echo Compile-fail check PASSED (compiler rejected discarded NurResult) [OK]"
else
	@if $(CC) $(CFLAGS_BASE) -std=c11 -Werror=unused-result -c test/test_compile_fail_nodiscard.c -o test_tmp.o 2>/dev/null; then \
		echo "ERROR: Discarding NurResult did not fail compilation!"; \
		exit 1; \
	else \
		echo "Compile-fail check PASSED (compiler rejected discarded NurResult) [OK]"; \
	fi
endif

# AddressSanitizer and UndefinedBehaviorSanitizer
.PHONY: test-asan
test-asan:
	@echo "Running tests with AddressSanitizer and UndefinedBehaviorSanitizer..."
	$(CC) $(CFLAGS_BASE) -std=c11 -fsanitize=address,undefined -g -o test_asan$(EXE) $(TEST_SRC)
	$(RUN_PREFIX)test_asan$(EXE)
	@rm -f test_asan$(EXE)

.PHONY: test-ubsan
test-ubsan:
	@echo "Running tests with UndefinedBehaviorSanitizer..."
	$(CC) $(CFLAGS_BASE) -std=c11 -fsanitize=undefined -g -o test_ubsan$(EXE) $(TEST_SRC)
	$(RUN_PREFIX)test_ubsan$(EXE)
	@rm -f test_ubsan$(EXE)

# Memory testing with Valgrind (if available)
.PHONY: test-valgrind
test-valgrind: $(TEST_BINARY)
	@echo "Running memory safety tests..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./$(TEST_BINARY); \
	else \
		echo "Valgrind not available, running normal tests..."; \
		./$(TEST_BINARY); \
	fi

# Static analysis (if available)
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c11 --suppress=missingIncludeSystem --error-exitcode=1 $(HEADER); \
	else \
		echo "cppcheck not available, skipping static analysis"; \
	fi

# Code formatting
.PHONY: format
format:
	@echo "Formatting source code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(HEADER) $(TEST_SRC) $(C23_TEST_SRC) examples/basic/*.c benchmarks/*.c 2>/dev/null; \
		echo "Formatting complete."; \
	else \
		echo "clang-format not available. Please install it."; \
	fi

.PHONY: format-check
format-check:
	@echo "Checking code formatting..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format --dry-run --Werror $(HEADER) $(TEST_SRC) $(C23_TEST_SRC) examples/basic/*.c benchmarks/*.c; \
	else \
		echo "clang-format not available. Skipping format check."; \
	fi

# Clean build artifacts
.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	-cmd /C del /Q /F $(TEST_BINARY) $(TEST_BINARY_C99) $(TEST_BINARY_C11) $(TEST_BINARY_C17) $(TEST_BINARY_C23) $(C23_TEST_BINARY) *.o *.exe 2>nul
else
	rm -f $(TEST_BINARY) $(TEST_BINARY_C99) $(TEST_BINARY_C11) $(TEST_BINARY_C17) $(TEST_BINARY_C23) $(C23_TEST_BINARY)
	rm -f *.o *.exe
endif

# Development workflow
.PHONY: dev
dev: clean check test analyze

# Release workflow  
.PHONY: release
release: clean check-c99 check-c11 check-c17 test-all analyze format-check
	@echo "All release checks passed!"

# Coverage reporting (if gcov available)
.PHONY: coverage
coverage:
	@echo "Generating coverage report..."
	@if command -v gcov >/dev/null 2>&1; then \
		$(CC) $(CFLAGS_C11) --coverage -o $(TEST_BINARY)_cov $(TEST_SRC); \
		./$(TEST_BINARY)_cov; \
		gcov $(TEST_SRC); \
		echo "Coverage files generated: *.gcov"; \
	else \
		echo "gcov not available, skipping coverage analysis"; \
	fi

# Documentation generation
.PHONY: docs
docs: docs-api docs-coverage

.PHONY: docs-api
docs-api:
	@echo "Generating API documentation..."
	@if [ -f tools/generate_docs.py ]; then \
		python3 tools/generate_docs.py $(HEADER) --format markdown --output docs/api/generated_api.md; \
		python3 tools/generate_docs.py $(HEADER) --format html --output docs/api/generated_api.html; \
		echo "API documentation generated in docs/api/"; \
	else \
		echo "Documentation generator not found"; \
	fi

.PHONY: docs-coverage
docs-coverage: $(TEST_BINARY)
	@echo "Generating coverage documentation..."
	@if [ -f tools/generate_coverage_report.py ]; then \
		python3 tools/generate_coverage_report.py $(HEADER) $(TEST_SRC) --output docs/coverage_report.md; \
		python3 tools/generate_coverage_report.py $(HEADER) $(TEST_SRC) --json --output docs/coverage_report.json; \
		echo "Coverage documentation generated in docs/"; \
	else \
		echo "Coverage report generator not found"; \
	fi

# Benchmark tests (if available)
.PHONY: benchmark
benchmark:
	@echo "Running benchmarks..."
	@if [ -f benchmarks/bench_nur.c ]; then \
		$(CC) $(CFLAGS_RELEASE) -o bench_nur benchmarks/bench_nur.c; \
		./bench_nur; \
		rm -f bench_nur; \
	else \
		echo "Benchmark tests not found"; \
	fi

# Install header (for system-wide installation)
.PHONY: install
install:
	@echo "Installing nur.h..."
	@PREFIX=$${PREFIX:-/usr/local}; \
	INCLUDEDIR=$$PREFIX/include; \
	echo "Installing to: $$INCLUDEDIR"; \
	mkdir -p $$INCLUDEDIR; \
	cp $(HEADER) $$INCLUDEDIR/; \
	echo "nur.h installed successfully!"

# Uninstall header
.PHONY: uninstall
uninstall:
	@echo "Uninstalling nur.h..."
	@PREFIX=$${PREFIX:-/usr/local}; \
	INCLUDEDIR=$$PREFIX/include; \
	rm -f $$INCLUDEDIR/nur.h; \
	echo "nur.h uninstalled!"

# Create distribution package
.PHONY: dist
dist: clean
	@echo "Creating distribution package..."
	@VERSION=$$(cat VERSION); \
	DISTNAME="nur.h-$$VERSION"; \
	mkdir -p dist/$$DISTNAME; \
	cp -r src/ docs/ examples/ test/ benchmarks/ scripts/ dist/$$DISTNAME/; \
	cp Makefile README.md LICENSE VERSION CONTRIBUTING.md dist/$$DISTNAME/; \
	cd dist && tar -czf $$DISTNAME.tar.gz $$DISTNAME; \
	echo "Distribution created: dist/$$DISTNAME.tar.gz"

# Version management
.PHONY: version
version:
	@echo "nur.h version: $$(cat VERSION)"
	@echo "Header version: $$(grep 'NUR_VERSION_STRING' $(HEADER) | cut -d'"' -f2)"

# Build and check all examples
.PHONY: examples
examples:
	@echo "Validating all examples..."
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/basic/01_hello_result.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/basic/02_error_codes.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/basic/03_option_types.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/basic/04_custom_types.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/basic/05_error_context.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/advanced/01_error_propagation.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/advanced/02_file_io.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/advanced/03_allocation.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/advanced/04_parsing.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/01_config_parser.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/02_string_processing.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/03_cli_arg_parser.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/04_json_parser.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/05_socket_server.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/06_http_client.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/07_thread_safe_logging.c
	$(CC) $(CFLAGS_BASE) -fsyntax-only examples/real-world/08_embedded_sensor.c
	@echo "All 17 examples passed syntax and type checks!"

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all          - Run default test suite (same as 'test')"
	@echo "  check        - Syntax check for all C standards"
	@echo "  check-c99    - Syntax check for C99"
	@echo "  check-c11    - Syntax check for C11" 
	@echo "  check-c17    - Syntax check for C17"
	@echo "  check-c23    - Syntax check for C23"
	@echo "  test         - Build and run test suite (C11)"
	@echo "  test-all     - Run tests for all C standards"
	@echo "  test-c99     - Run C99 tests"
	@echo "  test-c11     - Run C11 tests"
	@echo "  test-c17     - Run C17 tests"
	@echo "  test-c23     - Run C23 tests"
	@echo "  test-c23-features- Run C23 feature validation tests"
	@echo "  test-valgrind- Run tests with memory checking"
	@echo "  coverage     - Generate code coverage report"
	@echo "  analyze      - Run static analysis"
	@echo "  format       - Format source code using clang-format"
	@echo "  format-check - Check code formatting"
	@echo "  benchmark    - Run performance benchmarks"
	@echo "  docs         - Generate all documentation"
	@echo "  docs-api     - Generate API documentation"
	@echo "  docs-coverage- Generate coverage documentation"
	@echo "  install      - Install header system-wide"
	@echo "  uninstall    - Uninstall header"
	@echo "  dist         - Create distribution package"
	@echo "  version      - Show version information"
	@echo "  clean        - Remove build artifacts"
	@echo "  dev          - Full development check (clean+check+test+analyze)"
	@echo "  release      - Full release validation"
	@echo "  help         - Show this help message"