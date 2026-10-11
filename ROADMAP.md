# nur.h Project Roadmap & Ecosystem Evolution

This document defines the architectural roadmap, quality standards, and milestone tracking for `nur.h`.

---

## 🎯 Vision & True 10/10 Maturity Bar

`nur.h` aims to be the gold standard for type-safe, leak-free, zero-allocation error handling in the C ecosystem, serving both human software engineers and autonomous AI coding agents with zero friction.

---

## 🗺️ Milestone Tracking

### Milestone 1: Warning-Free Foundation Hardening ✅ [COMPLETED]
- [x] Fix unnamed struct/union pedantic warning in `NurResult_void` under ISO C99.
- [x] Guarantee 100% warning-free compilation under `-Wall -Wextra -Wpedantic` across ISO C99, C11, C17, and C23.
- [x] Enforce automated multi-standard validation matrix in GitHub Actions CI.
- [x] Verify AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) passes.

### Milestone 2: Production-Grade Developer Examples ✅ [COMPLETED]
- [x] INI configuration parser (`examples/real-world/01_config_parser.c`)
- [x] Pipeline string processor (`examples/real-world/02_string_processing.c`)
- [x] Zero-allocation CLI argument parser (`examples/real-world/03_cli_arg_parser.c`)
- [x] Zero-allocation JSON tokenizer & parser (`examples/real-world/04_json_parser.c`)
- [x] Cross-platform TCP socket server with leak-free `NUR_TRY_GOTO` (`examples/real-world/05_socket_server.c`)
- [x] Wire-format HTTP 1.1 request serializer & response parser (`examples/real-world/06_http_client.c`)
- [x] Thread-safe logging subsystem with RFC 8259 JSON serialization (`examples/real-world/07_thread_safe_logging.c`)
- [x] Embedded / bare-metal hardware sensor driver with CRC-8 validation (`examples/real-world/08_embedded_sensor.c`)

### Milestone 3: Ergonomic Developer API ✅ [COMPLETED]
- [x] Convenience assertion macros: `NUR_REQUIRE_OK(r)` and `NUR_REQUIRE_OK_MSG(r, msg)` with automated error diagnostic printing.
- [x] Clean error branching: `NUR_IF_ERR(r, err_var, body)` and `NUR_VOID_IF_ERR(r, err_var, body)`.
- [x] Portable pattern matching: `NUR_MATCH_AS(Type, r, val, ok_body, err, err_body)`.
- [x] GNU/C23 auto pattern matching: `NUR_MATCH(r, val, ok_body, err, err_body)`.
- [x] Semantic fallback: `NUR_OR_ELSE(r, fallback)`.

### Milestone 4: Packaging & Ecosystem Integration ✅ [COMPLETED]
- [x] Modern CMake package export with `find_package(nur CONFIG REQUIRED)` and alias `nur::nur`.
- [x] `pkg-config` specification (`nur.pc`).
- [x] Native Meson build definition (`meson.build`).
- [x] Comprehensive packaging documentation (`docs/PACKAGING.md`).

### Milestone 5: AI / LLM DX & Autonomous Safety ✅ [COMPLETED]
- [x] AI usage contract defining when to use and when NOT to use `nur.h` (`docs/AI_GUIDELINES.md`).
- [x] Refactoring prompt pack for Copilot, Claude, and GPT.
- [x] Machine-readable RFC 8259 error taxonomy and schema (`docs/ERROR_TAXONOMY.md` and `docs/errors.json`).
- [x] Automated C code migration helper script (`scripts/wrap_legacy_c.py`).

### Milestone 6: Quality, Benchmarking & Performance ✅ [COMPLETED]
- [x] Empirical latency benchmarking suite (`benchmarks/bench_nur.c`).
- [x] Zero-heap allocation and sub-nanosecond runtime overhead documentation (`docs/BENCHMARKS.md`).
- [x] Migration guide with before/after code blocks (`docs/MIGRATION.md`).

---

## 🏛️ Maintenance & Governance Policy

1. **Semantic Versioning (SemVer 2.0)**:
   - Major releases (`v2.x`): Breaking changes to macro signatures or type layouts.
   - Minor releases (`v1.x`): Backward-compatible additions of new macros, types, or adapters.
   - Patch releases (`v1.0.x`): Bug fixes, documentation updates, compiler compatibility tweaks.
2. **Zero-Allocation Guarantee**:
   - No heap allocations (`malloc`, `calloc`) will ever be added to core `nur.h` paths.
3. **Compiler Standards Commitment**:
   - Zero-warning builds under `-Wall -Wextra -Wpedantic` across ISO C99, C11, C17, and C23 will be strictly enforced as a blocking CI gate.
