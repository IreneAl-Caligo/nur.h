/* SPDX-License-Identifier: MIT */
/*
 * nur_test.c - self-checking test suite for nur.h.
 *
 * Build and run (any C11 compiler):
 *     cc -std=c11 -Wall -Wextra -Wpedantic -I. nur_test.c -o nur_test && ./nur_test
 * Exit status 0 means every check passed.  The suite adapts to the active
 * NUR_CONFIG_* settings, so it can be built under -DNDEBUG, -DNUR_CONFIG_MINIMAL,
 * -DNUR_CONFIG_SOURCE_LOCATION=0/1/2 and so on.
 */

#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Route panics into the test harness instead of aborting. */
struct nur_error;
static void test_panic_handler(const char *message, const struct nur_error *error,
                               const char *file, const char *function, unsigned line);
#define NUR_PANIC_HANDLER(m_, e_, f_, fn_, l_) \
    test_panic_handler((m_), (e_), (f_), (fn_), (l_))

#include <nur.h>

/* --------------------------------------------------------------------------
 * Harness
 * -------------------------------------------------------------------------- */

static int g_checks;
static int g_failures;

#define CHECK(cond_)                                                                  \
    do {                                                                              \
        ++g_checks;                                                                   \
        if (!(cond_)) {                                                               \
            ++g_failures;                                                             \
            fprintf(stderr, "CHECK FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond_);  \
        }                                                                             \
    } while (0)

#define CHECK_STR(actual_, expected_)                                                 \
    do {                                                                              \
        ++g_checks;                                                                   \
        const char *a_ = (actual_);                                                   \
        const char *e_ = (expected_);                                                 \
        if (a_ == NULL || strcmp(a_, e_) != 0) {                                      \
            ++g_failures;                                                             \
            fprintf(stderr, "CHECK_STR FAILED %s:%d: got \"%s\" expected \"%s\"\n",   \
                    __FILE__, __LINE__, a_ ? a_ : "(null)", e_);                      \
        }                                                                             \
    } while (0)

static jmp_buf g_jmp;
static int g_armed;
static int g_panics;
static const char *g_panic_message;
static int g_panic_has_cause;
static nur_error_code_t g_panic_cause;

static void test_panic_handler(const char *message, const struct nur_error *error,
                               const char *file, const char *function, unsigned line)
{
    (void)file;
    (void)function;
    (void)line;
    ++g_panics;
    g_panic_message   = message;
    g_panic_has_cause = (error != NULL);
    g_panic_cause     = (error != NULL) ? error->code : NUR_E_NONE;
    if (g_armed) {
        longjmp(g_jmp, 1);
    }
    fputs("unexpected panic outside EXPECT_PANIC\n", stderr);
    abort();
}

/* Runs `stmt_` and requires that it panics. */
#define EXPECT_PANIC(stmt_)                                                           \
    do {                                                                              \
        ++g_checks;                                                                   \
        g_armed = 1;                                                                  \
        if (setjmp(g_jmp) == 0) {                                                     \
            stmt_;                                                                    \
            ++g_failures;                                                             \
            fprintf(stderr, "EXPECT_PANIC FAILED %s:%d: did not panic: %s\n",         \
                    __FILE__, __LINE__, #stmt_);                                      \
        }                                                                             \
        g_armed = 0;                                                                  \
    } while (0)

/* --------------------------------------------------------------------------
 * Types under test
 * -------------------------------------------------------------------------- */

typedef struct point { int x; int y; } point_t;

NUR_DEFINE_RESULT(int, int_result);
NUR_DEFINE_RESULT(const char *, str_result);
NUR_DEFINE_RESULT(point_t, point_result);
NUR_DEFINE_OPTION(int, int_option);
NUR_DEFINE_OPTION(point_t, point_option);

static int g_calls; /* counts evaluations of the functions below */

static int_result ok_seven(void)
{
    ++g_calls;
    return NUR_OK(int_result, 7);
}

static int_result fail_not_found(void)
{
    ++g_calls;
    return NUR_FAIL(int_result, NUR_E_NOT_FOUND, "missing");
}

static int_result parse_positive(int v)
{
    ++g_calls;
    if (v < 0) {
        return NUR_FAIL(int_result, NUR_E_INVALID_ARGUMENT, "negative");
    }
    return NUR_OK(int_result, v);
}

static nur_status init_ok(void)
{
    ++g_calls;
    return NUR_STATUS_OK();
}

static nur_status init_fail(void)
{
    ++g_calls;
    return NUR_STATUS_ERR(NUR_MAKE_ERROR(NUR_E_BUSY, "device busy"));
}

/* --------------------------------------------------------------------------
 * Tests
 * -------------------------------------------------------------------------- */

static void test_version(void)
{
    CHECK(NUR_VERSION_NUMBER == (NUR_VERSION_MAJOR * 10000 + NUR_VERSION_MINOR * 100 + NUR_VERSION_PATCH));
    CHECK(NUR_VERSION_AT_LEAST(0, 0, 0));
    CHECK(NUR_VERSION_AT_LEAST(NUR_VERSION_MAJOR, NUR_VERSION_MINOR, NUR_VERSION_PATCH));
    CHECK(!NUR_VERSION_AT_LEAST(NUR_VERSION_MAJOR + 1, 0, 0));
    CHECK(strlen(NUR_VERSION_STRING) >= 5u);
}

static void test_error_codes(void)
{
    static const struct { nur_error_code_t code; const char *name; const char *text; } table[] = {
#define NUR_TEST_X(n_, v_, t_) { n_, #n_, t_ },
        NUR_ERROR_CODE_LIST(NUR_TEST_X)
#undef NUR_TEST_X
    };
    const size_t count = sizeof table / sizeof table[0];
    size_t i, j;

    CHECK(count >= 20u);
    for (i = 0; i < count; ++i) {
        CHECK_STR(nur_error_code_name(table[i].code), table[i].name);
        CHECK_STR(nur_error_code_text(table[i].code), table[i].text);
        CHECK(strncmp(table[i].name, "NUR_E_", 6) == 0);
        CHECK(table[i].text[0] != '\0');
        for (j = i + 1u; j < count; ++j) {
            CHECK(table[i].code != table[j].code);               /* numeric values are unique */
            CHECK(strcmp(table[i].name, table[j].name) != 0);    /* names are unique          */
        }
    }

    /* Numeric values are a stable, append-only contract: pin a few. */
    CHECK((int)NUR_E_NONE == 0);
    CHECK((int)NUR_E_INVALID_ARGUMENT == 1);
    CHECK((int)NUR_E_NOT_FOUND == 9);
    CHECK((int)NUR_E_TIMEOUT == 17);
    CHECK((int)NUR_E_BUG == 25);

    CHECK_STR(nur_error_code_name(12345), "NUR_E_UNKNOWN");
    CHECK_STR(nur_error_code_text(12345), "unknown error");
    CHECK_STR(nur_error_code_name(-1), "NUR_E_UNKNOWN");
    CHECK_STR(nur_error_code_name((int)NUR_E_BUG + 1), "NUR_E_UNKNOWN");   /* first unassigned value */
}

static void test_error_object(void)
{
    nur_error_t e = nur_error_make(NUR_E_IO, 5, "read failed", "disk0", "a.c", "f", 12u);
    nur_error_t c;
    nur_error_t zero;

    CHECK(e.code == NUR_E_IO);
    CHECK(e.system_code == 5);
    CHECK_STR(e.message, "read failed");
    CHECK_STR(e.context, "disk0");
    CHECK_STR(e.file, "a.c");
    CHECK_STR(e.function, "f");
    CHECK(e.line == 12u);

    c = nur_error_with_context(e, "disk1");
    CHECK_STR(c.context, "disk1");
    CHECK_STR(e.context, "disk0");               /* the original is untouched */
    CHECK(c.code == e.code && c.system_code == e.system_code);

    /* A NULL message falls back to the default description of the code. */
    e = nur_error_make(NUR_E_TIMEOUT, 0, NULL, NULL, NULL, NULL, 0u);
    CHECK_STR(nur_error_message(&e), nur_error_code_text(NUR_E_TIMEOUT));
    CHECK_STR(nur_error_message(NULL), "no error");

    /* Zero-initialised error means "no error". */
    memset(&zero, 0, sizeof zero);
    CHECK(zero.code == NUR_E_NONE);
}

static void test_format(void)
{
    char buf[256];
    char small[10];
    size_t n;
    nur_error_t e;

    e = nur_error_make(NUR_E_NOT_FOUND, 2, "could not open file", "config.ini",
                       "main.c", "load", 42u);
    n = nur_error_format(&e, buf, sizeof buf);
    CHECK_STR(buf, "NUR_E_NOT_FOUND: could not open file [config.ini] (system code 2) "
                   "at main.c:42 in load()");
    CHECK(n == strlen(buf));

    /* Truncation: always NUL-terminated, returns the full length (snprintf semantics). */
    n = nur_error_format(&e, small, sizeof small);
    CHECK(n == strlen("NUR_E_NOT_FOUND: could not open file [config.ini] (system code 2) "
                      "at main.c:42 in load()"));
    CHECK(strlen(small) == sizeof small - 1u);
    CHECK(strncmp(small, "NUR_E_NOT_", sizeof small - 1u) == 0);

    /* Measuring with no buffer; capacity of one yields the empty string. */
    CHECK(nur_error_format(&e, NULL, 0u) == n);
    small[0] = 'x';
    CHECK(nur_error_format(&e, small, 1u) == n);
    CHECK(small[0] == '\0');

    /* Optional parts are omitted when absent. */
    e = nur_error_make(NUR_E_TIMEOUT, 0, NULL, NULL, NULL, NULL, 0u);
    nur_error_format(&e, buf, sizeof buf);
    CHECK_STR(buf, "NUR_E_TIMEOUT: operation timed out");

    e = nur_error_make(NUR_E_IO, 0, "x", "", "f.c", NULL, 7u);   /* empty context, no function */
    nur_error_format(&e, buf, sizeof buf);
    CHECK_STR(buf, "NUR_E_IO: x at f.c:7");

    /* Negative system codes and the largest line number. */
    e = nur_error_make(NUR_E_SYSTEM, -5, "m", NULL, "f.c", NULL, 4294967295u);
    nur_error_format(&e, buf, sizeof buf);
    CHECK_STR(buf, "NUR_E_SYSTEM: m (system code -5) at f.c:4294967295");

    /* NULL error is treated as "no error". */
    nur_error_format(NULL, buf, sizeof buf);
    CHECK_STR(buf, "NUR_E_NONE: no error");

#if NUR_HAS_STDIO
    {
        /* nur_error_print writes the same text plus a newline. */
        FILE *tmp = tmpfile();
        if (tmp != NULL) {
            char line[128];
            e = nur_error_make(NUR_E_BUSY, 0, "device busy", NULL, NULL, NULL, 0u);
            nur_error_print(tmp, &e);
            rewind(tmp);
            CHECK(fgets(line, (int)sizeof line, tmp) != NULL);
            CHECK_STR(line, "NUR_E_BUSY: device busy\n");
            (void)fclose(tmp);
        }
        /* A message far larger than the print buffer is truncated, never overflowed,
         * and the line still ends with a newline. */
        tmp = tmpfile();
        if (tmp != NULL) {
            static char huge[NUR_CONFIG_PRINT_BUFFER_SIZE * 2u];
            static char back[NUR_CONFIG_PRINT_BUFFER_SIZE * 3u];
            size_t got;
            memset(huge, 'a', sizeof huge - 1u);
            huge[sizeof huge - 1u] = '\0';
            e = nur_error_make(NUR_E_IO, 0, huge, NULL, NULL, NULL, 0u);
            nur_error_print(tmp, &e);
            rewind(tmp);
            got = fread(back, 1u, sizeof back - 1u, tmp);
            back[got] = '\0';
            CHECK(got == (size_t)NUR_CONFIG_PRINT_BUFFER_SIZE - 1u);     /* (size-2) chars + '\n' */
            CHECK(back[got - 1u] == '\n');
            CHECK(strncmp(back, "NUR_E_IO: aaa", 13) == 0);
            (void)fclose(tmp);
        }
    }
#endif
}

static void test_errno_adapter(void)
{
    nur_error_t e;

    CHECK(nur_error_code_from_errno(0) == NUR_E_NONE);
#ifdef ENOENT
    CHECK(nur_error_code_from_errno(ENOENT) == NUR_E_NOT_FOUND);
#endif
#ifdef EACCES
    CHECK(nur_error_code_from_errno(EACCES) == NUR_E_PERMISSION);
#endif
#ifdef EEXIST
    CHECK(nur_error_code_from_errno(EEXIST) == NUR_E_ALREADY_EXISTS);
#endif
#ifdef ENOMEM
    CHECK(nur_error_code_from_errno(ENOMEM) == NUR_E_OUT_OF_MEMORY);
#endif
#ifdef EINVAL
    CHECK(nur_error_code_from_errno(EINVAL) == NUR_E_INVALID_ARGUMENT);
#endif
#ifdef ERANGE
    CHECK(nur_error_code_from_errno(ERANGE) == NUR_E_OUT_OF_RANGE);
#endif
#ifdef EILSEQ
    CHECK(nur_error_code_from_errno(EILSEQ) == NUR_E_ENCODING);
#endif
#ifdef EINTR
    CHECK(nur_error_code_from_errno(EINTR) == NUR_E_INTERRUPTED);
#endif
#ifdef ETIMEDOUT
    CHECK(nur_error_code_from_errno(ETIMEDOUT) == NUR_E_TIMEOUT);
#endif
#ifdef EPIPE
    CHECK(nur_error_code_from_errno(EPIPE) == NUR_E_DISCONNECTED);
#endif
#ifdef EMFILE
    CHECK(nur_error_code_from_errno(EMFILE) == NUR_E_RESOURCE_LIMIT);
#endif
#if defined(EAGAIN) && defined(EWOULDBLOCK)
    CHECK(nur_error_code_from_errno(EAGAIN) == NUR_E_BUSY);
    CHECK(nur_error_code_from_errno(EWOULDBLOCK) == NUR_E_BUSY);   /* may alias EAGAIN */
#endif
    CHECK(nur_error_code_from_errno(99999) == NUR_E_SYSTEM);

    /* The native value is preserved next to the semantic code. */
#ifdef ENOENT
    e = nur_error_from_errno(ENOENT, "open failed", "f.c", "fn", 9u);
    CHECK(e.code == NUR_E_NOT_FOUND);
    CHECK(e.system_code == ENOENT);
    CHECK_STR(e.message, "open failed");
    CHECK(e.line == 9u);
#endif

    /* An error object never carries NUR_E_NONE, even for errno == 0. */
    e = nur_error_from_errno(0, "unknown failure", NULL, NULL, 0u);
    CHECK(e.code == NUR_E_SYSTEM);
    CHECK(e.system_code == 0);

#if NUR_HAS_ERRNO && defined(EACCES)
    errno = EACCES;
    e = NUR_ERROR_FROM_ERRNO("cannot open");
    CHECK(e.code == NUR_E_PERMISSION);
    CHECK(e.system_code == EACCES);
#endif
#if NUR_HAS_ERRNO && defined(ENOENT)
    {
        /* A genuine failing system call, captured at the boundary. */
        FILE *f;
        errno = 0;
        f = fopen("/nonexistent-directory-for-nur-test/file.txt", "r");
        if (f == NULL) {
            e = NUR_ERROR_FROM_ERRNO("cannot open file");
            CHECK(e.code == NUR_E_NOT_FOUND);
            CHECK(e.system_code == ENOENT);
        } else {
            (void)fclose(f);
        }
    }
#endif
#ifdef ENOMEM
    e = NUR_ERROR_FROM_ERRNO_VALUE(ENOMEM, "alloc");
    CHECK(e.code == NUR_E_OUT_OF_MEMORY);
    CHECK(e.system_code == ENOMEM);
#endif
}

static void test_win32_adapter(void)
{
    nur_error_t e;

    CHECK(nur_error_code_from_win32(0UL) == NUR_E_NONE);
    CHECK(nur_error_code_from_win32(2UL) == NUR_E_NOT_FOUND);      /* ERROR_FILE_NOT_FOUND   */
    CHECK(nur_error_code_from_win32(3UL) == NUR_E_NOT_FOUND);      /* ERROR_PATH_NOT_FOUND   */
    CHECK(nur_error_code_from_win32(5UL) == NUR_E_PERMISSION);     /* ERROR_ACCESS_DENIED    */
    CHECK(nur_error_code_from_win32(8UL) == NUR_E_OUT_OF_MEMORY);  /* ERROR_NOT_ENOUGH_MEMORY*/
    CHECK(nur_error_code_from_win32(87UL) == NUR_E_INVALID_ARGUMENT);
    CHECK(nur_error_code_from_win32(183UL) == NUR_E_ALREADY_EXISTS);
    CHECK(nur_error_code_from_win32(995UL) == NUR_E_CANCELLED);
    CHECK(nur_error_code_from_win32(10054UL) == NUR_E_DISCONNECTED);
    CHECK(nur_error_code_from_win32(10060UL) == NUR_E_TIMEOUT);
    CHECK(nur_error_code_from_win32(10061UL) == NUR_E_CONNECTION);
    CHECK(nur_error_code_from_win32(424242UL) == NUR_E_SYSTEM);

    e = NUR_ERROR_FROM_WIN32(5, "open failed");
    CHECK(e.code == NUR_E_PERMISSION);
    CHECK(e.system_code == 5);
    e = nur_error_from_win32(0UL, "x", NULL, NULL, 0u);
    CHECK(e.code == NUR_E_SYSTEM);
}

static void test_result_basics(void)
{
    int_result ok = NUR_OK(int_result, 42);
    int_result err = NUR_FAIL(int_result, NUR_E_PARSE, "bad input");
    int_result zero;
    point_result pr = NUR_OK(point_result, ((point_t){ 3, 4 }));
    str_result sr = NUR_OK(str_result, "hello");

    CHECK(NUR_IS_OK(ok));
    CHECK(!NUR_IS_ERR(ok));
    CHECK(NUR_VALUE(ok) == 42);

    CHECK(NUR_IS_ERR(err));
    CHECK(!NUR_IS_OK(err));
    CHECK(NUR_ERROR(err).code == NUR_E_PARSE);
    CHECK_STR(NUR_ERROR(err).message, "bad input");

    /* Zero-initialised results fail closed: they are errors, never bogus successes. */
    memset(&zero, 0, sizeof zero);
    CHECK(NUR_IS_ERR(zero));
    CHECK(NUR_ERROR(zero).code == NUR_E_NONE);
    CHECK((int)NUR_RESULT_ERR == 0);

    CHECK(NUR_VALUE(pr).x == 3 && NUR_VALUE(pr).y == 4);
    CHECK_STR(NUR_VALUE(sr), "hello");

    /* Every result can carry an error. */
    CHECK(sizeof(int_result) >= sizeof(nur_error_t));
    CHECK(sizeof(point_result) >= sizeof(nur_error_t));

    /* Errors travel between result types unchanged. */
    sr = NUR_ERR(str_result, NUR_ERROR(err));
    CHECK(NUR_IS_ERR(sr));
    CHECK(NUR_ERROR(sr).code == NUR_E_PARSE);

#if !defined(NUR_CONFIG_NO_BUILTIN_TYPES)
    {
        nur_result_int bi = NUR_OK(nur_result_int, -1);
        nur_result_size bs = NUR_OK(nur_result_size, (size_t)9);
        nur_result_double bd = NUR_OK(nur_result_double, 1.5);
        nur_result_cstr bc = NUR_OK(nur_result_cstr, "c");
        nur_result_ptr bp = NUR_OK(nur_result_ptr, (void *)&bi);
        nur_result_bool bb = NUR_OK(nur_result_bool, true);
        nur_result_i64 b6 = NUR_OK(nur_result_i64, (int64_t)-5);
        nur_result_u64 bu = NUR_OK(nur_result_u64, (uint64_t)5);
        CHECK(NUR_VALUE(bi) == -1);
        CHECK(NUR_VALUE(bs) == 9u);
        CHECK(NUR_VALUE(bd) > 1.49 && NUR_VALUE(bd) < 1.51);
        CHECK_STR(NUR_VALUE(bc), "c");
        CHECK(NUR_VALUE(bp) == (void *)&bi);
        CHECK(NUR_VALUE(bb));
        CHECK(NUR_VALUE(b6) == -5 && NUR_VALUE(bu) == 5u);
    }
#endif
}

static void test_unwrap_family(void)
{
    int_result ok = NUR_OK(int_result, 5);

    /* Success path, single evaluation. */
    g_calls = 0;
    CHECK(NUR_UNWRAP(int_result, ok_seven()) == 7);
    CHECK(g_calls == 1);
    g_calls = 0;
    CHECK(NUR_EXPECT(int_result, ok_seven(), "seven is always available") == 7);
    CHECK(g_calls == 1);
    CHECK(NUR_UNWRAP(int_result, ok) == 5);

    /* unwrap_or: the fallback is used on error only. */
    g_calls = 0;
    CHECK(NUR_UNWRAP_OR(int_result, fail_not_found(), 99) == 99);
    CHECK(g_calls == 1);
    CHECK(NUR_UNWRAP_OR(int_result, ok, 99) == 5);

    /* Panics carry the error as their cause. */
    g_panics = 0;
    EXPECT_PANIC((void)NUR_UNWRAP(int_result, fail_not_found()));
    CHECK(g_panics == 1);
    CHECK(g_panic_has_cause);
    CHECK(g_panic_cause == NUR_E_NOT_FOUND);
    CHECK(strstr(g_panic_message, "NUR_UNWRAP") != NULL);

    EXPECT_PANIC((void)NUR_EXPECT(int_result, fail_not_found(), "config must load"));
    CHECK_STR(g_panic_message, "config must load");
    CHECK(g_panic_cause == NUR_E_NOT_FOUND);
}

/* ---- propagation fixtures ---- */

static int_result add_one(int v)
{
    int x;
    NUR_TRY_ASSIGN(int_result, x, parse_positive(v));
    return NUR_OK(int_result, x + 1);
}

static int_result add_two_values(int a, int b)
{
    int x, y;                                   /* two TRYs in one scope: unique temporaries */
    NUR_TRY_ASSIGN(int_result, x, parse_positive(a));
    NUR_TRY_ASSIGN(int_result, y, parse_positive(b));
    return NUR_OK(int_result, x + y);
}

static str_result describe(int v)
{
    int x;
    NUR_TRY_ASSIGN_AS(str_result, int_result, x, parse_positive(v));
    return NUR_OK(str_result, (x > 5) ? "big" : "small");
}

static nur_status check_positive(int v)
{
    NUR_TRY_AS(nur_status, int_result, parse_positive(v));
    return NUR_STATUS_OK();
}

static nur_status init_both(int fail_second)
{
    NUR_TRY(nur_status, init_ok());
    NUR_TRY(nur_status, fail_second ? init_fail() : init_ok());
    return NUR_STATUS_OK();
}

static int_result guarded(int flag, int v)
{
    int x = 0;
    if (flag)                                    /* macros are single statements */
        NUR_TRY_ASSIGN(int_result, x, parse_positive(v));
    else
        x = -1;
    return NUR_OK(int_result, x);
}

static int_result nested(int v)
{
    int x;
    /* TRY inside the expression of another TRY's argument list */
    NUR_TRY_ASSIGN(int_result, x, add_one(NUR_UNWRAP(int_result, add_one(v))));
    return NUR_OK(int_result, x);
}

static int_result store_through_index(int *array, int *index, int v)
{
    NUR_TRY_ASSIGN(int_result, array[(*index)++], parse_positive(v));   /* `out` evaluated once */
    return NUR_OK(int_result, 0);
}

static void test_propagation(void)
{
    int_result r;
    str_result s;
    nur_status st;
    int array[4] = { 0, 0, 0, 0 };
    int index = 0;

    r = add_one(4);
    CHECK(NUR_IS_OK(r) && NUR_VALUE(r) == 5);

    g_calls = 0;
    r = add_one(-1);                             /* error is propagated unchanged */
    CHECK(NUR_IS_ERR(r));
    CHECK(NUR_ERROR(r).code == NUR_E_INVALID_ARGUMENT);
    CHECK_STR(NUR_ERROR(r).message, "negative");
    CHECK(g_calls == 1);                         /* the callee ran exactly once */

    r = add_two_values(1, 2);
    CHECK(NUR_IS_OK(r) && NUR_VALUE(r) == 3);
    g_calls = 0;
    r = add_two_values(-1, 2);
    CHECK(NUR_IS_ERR(r) && g_calls == 1);        /* short-circuits after the first failure */
    g_calls = 0;
    r = add_two_values(1, -2);
    CHECK(NUR_IS_ERR(r) && g_calls == 2);

    /* Conversion across result types. */
    s = describe(9);
    CHECK(NUR_IS_OK(s));
    CHECK_STR(NUR_VALUE(s), "big");
    s = describe(2);
    CHECK_STR(NUR_VALUE(s), "small");
    s = describe(-3);
    CHECK(NUR_IS_ERR(s));
    CHECK(NUR_ERROR(s).code == NUR_E_INVALID_ARGUMENT);

    /* Status propagation. */
    st = check_positive(3);
    CHECK(NUR_IS_OK(st));
    st = check_positive(-3);
    CHECK(NUR_IS_ERR(st));
    CHECK(NUR_ERROR(st).code == NUR_E_INVALID_ARGUMENT);

    st = init_both(0);
    CHECK(NUR_IS_OK(st));
    st = init_both(1);
    CHECK(NUR_IS_ERR(st));
    CHECK(NUR_ERROR(st).code == NUR_E_BUSY);

    /* Safe in an unbraced if/else. */
    CHECK(NUR_VALUE(guarded(0, 5)) == -1);
    CHECK(NUR_VALUE(guarded(1, 5)) == 5);
    CHECK(NUR_IS_ERR(guarded(1, -5)));

    r = nested(1);
    CHECK(NUR_IS_OK(r) && NUR_VALUE(r) == 3);

    /* `out` is evaluated exactly once on success and not at all on failure. */
    r = store_through_index(array, &index, 8);
    CHECK(NUR_IS_OK(r) && index == 1 && array[0] == 8);
    r = store_through_index(array, &index, -8);
    CHECK(NUR_IS_ERR(r) && index == 1);
}

static void test_option(void)
{
    int_option some = NUR_SOME(int_option, 11);
    int_option none = NUR_NONE(int_option);
    point_option pn = NUR_NONE(point_option);
    point_option ps = NUR_SOME(point_option, ((point_t){ 1, 2 }));
    int_option zero;

    CHECK(NUR_IS_SOME(some) && !NUR_IS_NONE(some));
    CHECK(NUR_OPTION_VALUE(some) == 11);
    CHECK(NUR_IS_NONE(none) && !NUR_IS_SOME(none));
    CHECK(NUR_IS_NONE(pn));
    CHECK(NUR_OPTION_VALUE(pn).x == 0 && NUR_OPTION_VALUE(pn).y == 0);   /* None is all-zero */
    CHECK(NUR_OPTION_VALUE(ps).y == 2);

    memset(&zero, 0, sizeof zero);
    CHECK(NUR_IS_NONE(zero));                    /* zero-initialised option is None */

    CHECK(NUR_OPTION_UNWRAP(int_option, some) == 11);
    CHECK(NUR_OPTION_UNWRAP_OR(int_option, none, 7) == 7);
    CHECK(NUR_OPTION_UNWRAP_OR(int_option, some, 7) == 11);

    g_panics = 0;
    EXPECT_PANIC((void)NUR_OPTION_UNWRAP(int_option, none));
    CHECK(g_panics == 1);
    CHECK(!g_panic_has_cause);
    CHECK(strstr(g_panic_message, "None") != NULL);

#if !defined(NUR_CONFIG_NO_BUILTIN_TYPES)
    {
        nur_option_int oi = NUR_SOME(nur_option_int, 3);
        nur_option_ptr op = NUR_NONE(nur_option_ptr);
        CHECK(NUR_OPTION_UNWRAP(nur_option_int, oi) == 3);
        CHECK(NUR_IS_NONE(op));
    }
#endif
}

static void test_status(void)
{
    nur_status_t ok = NUR_STATUS_OK();
    nur_status_t bad = NUR_STATUS_ERR(nur_error_make(NUR_E_CANCELLED, 0, "stop", NULL, NULL, NULL, 0u));
    nur_status plain = ok;                       /* nur_status_t is an alias of nur_status */
    nur_status zero;

    CHECK(NUR_IS_OK(ok) && NUR_IS_OK(plain));
    CHECK(NUR_IS_ERR(bad));
    CHECK(NUR_ERROR(bad).code == NUR_E_CANCELLED);

    memset(&zero, 0, sizeof zero);
    CHECK(NUR_IS_ERR(zero));                     /* fail-closed */

    g_panics = 0;
    EXPECT_PANIC((void)NUR_EXPECT(nur_status, init_fail(), "init must succeed"));
    CHECK_STR(g_panic_message, "init must succeed");
    CHECK(g_panic_cause == NUR_E_BUSY);
    (void)NUR_EXPECT(nur_status, init_ok(), "init must succeed");   /* no panic */
    CHECK(g_panics == 1);
}

static void test_ignore(void)
{
    g_calls = 0;
    NUR_IGNORE(fail_not_found());                /* explicit, deliberate discard */
    CHECK(g_calls == 1);                         /* evaluated exactly once */
    NUR_IGNORE(init_fail());
    CHECK(g_calls == 2);
}

static int g_assert_side_effects;

static int assert_probe(void)
{
    ++g_assert_side_effects;
    return 1;
}

static void test_panic_and_assert(void)
{
    g_panics = 0;
    EXPECT_PANIC(NUR_PANIC("boom"));
    CHECK_STR(g_panic_message, "boom");
    CHECK(!g_panic_has_cause);

    EXPECT_PANIC(NUR_UNREACHABLE());
    CHECK(strstr(g_panic_message, "unreachable") != NULL);
    CHECK(g_panics == 2);

    g_assert_side_effects = 0;
#if NUR_CONFIG_ASSERTS
    NUR_ASSERT(1 == 1);                          /* passing assertion: no panic */
    NUR_ASSERT(assert_probe());
    CHECK(g_assert_side_effects == 1);           /* condition evaluated exactly once */
    EXPECT_PANIC(NUR_ASSERT(1 == 2));
    CHECK(strstr(g_panic_message, "1 == 2") != NULL);
    EXPECT_PANIC(NUR_ASSERT_MSG(0, "must hold"));
    CHECK(strstr(g_panic_message, "must hold") != NULL);
    CHECK(g_panics == 4);
#else
    NUR_ASSERT(assert_probe());                  /* disabled: not evaluated at all */
    NUR_ASSERT_MSG(assert_probe(), "unused");
    NUR_ASSERT(0 == 1);                          /* disabled: does not panic */
    CHECK(g_assert_side_effects == 0);
    CHECK(g_panics == 2);
    (void)assert_probe;
#endif
}

static void test_source_location(void)
{
    nur_error_t e = NUR_MAKE_ERROR(NUR_E_IO, "x"); const unsigned expected_line = (unsigned)__LINE__;
    nur_error_t c = NUR_MAKE_ERROR_CTX(NUR_E_IO, "x", "ctx");
    nur_error_t s = NUR_MAKE_ERROR_SYS(NUR_E_IO, 77, "x");

    CHECK_STR(c.context, "ctx");
    CHECK(e.context == NULL);
    CHECK(s.system_code == 77);

#if NUR_CONFIG_SOURCE_LOCATION == NUR_SOURCE_LOCATION_FULL
    CHECK(e.file != NULL && strstr(e.file, "nur_test.c") != NULL);
    CHECK_STR(e.function, "test_source_location");
    CHECK(e.line == expected_line);
#elif NUR_CONFIG_SOURCE_LOCATION == NUR_SOURCE_LOCATION_BASIC
    CHECK(e.file != NULL && strstr(e.file, "nur_test.c") != NULL);
    CHECK(e.function == NULL);
    CHECK(e.line == expected_line);
#else
    CHECK(e.file == NULL && e.function == NULL && e.line == 0u);
    (void)expected_line;
#endif
}

static void test_layout_independent_of_config(void)
{
    /* The error layout must not change with configuration: fixed fields only. */
    nur_error_t e;
    CHECK(sizeof e.code == sizeof(nur_error_code_t));
    CHECK(sizeof(nur_error_t) >= 3u * sizeof(void *) + sizeof(int) + sizeof(unsigned));
}

int main(void)
{
    test_version();
    test_error_codes();
    test_error_object();
    test_format();
    test_errno_adapter();
    test_win32_adapter();
    test_result_basics();
    test_unwrap_family();
    test_propagation();
    test_option();
    test_status();
    test_ignore();
    test_panic_and_assert();
    test_source_location();
    test_layout_independent_of_config();

    printf("nur.h %s: %d checks, %d failed\n", NUR_VERSION_STRING, g_checks, g_failures);
    return (g_failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
