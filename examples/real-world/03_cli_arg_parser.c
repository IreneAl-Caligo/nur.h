/**
 * 03_cli_arg_parser.c - CLI Argument Parser with nur.h
 *
 * Demonstrates:
 * - Parsing command line flags, key-value options, and positional arguments
 * - Zero-allocation parsing over argv
 * - Custom result type for parsed CLI configuration
 * - Semantic error categorization (INVALID_ARGUMENT, OUT_OF_RANGE, NOT_FOUND)
 * - Both success path and comprehensive error simulation
 */

#include "nur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* CLI Options Configuration Structure */
typedef struct CliConfig {
    const char *host;
    int port;
    int verbose;
    const char *input_file;
} CliConfig;

/* Declare custom Result type for CliConfig at file scope */
NUR_RESULT_TYPE(CliConfig, cli_config);

/**
 * Helper: parse port number with bounds validation (1..65535).
 */
static NurResult_i32 parse_port(const char *str) {
    if (!str || *str == '\0') {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Port string is empty or NULL"));
    }

    char *endptr = NULL;
    long val = strtol(str, &endptr, 10);
    if (*endptr != '\0') {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_PARSE, "Port is not a valid integer", str));
    }
    if (val <= 0 || val > 65535) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, "Port must be in range 1-65535", str));
    }

    return NUR_OK(i32, (int32_t)val);
}

/**
 * Parse CLI argument list into a validated CliConfig.
 */
static NurResult_cli_config parse_cli_args(int argc, char *argv[]) {
    CliConfig config;
    config.host = "127.0.0.1"; /* default */
    config.port = 8080;        /* default */
    config.verbose = 0;
    config.input_file = NULL;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            printf("Usage: app [--host <addr>] [--port <1-65535>] [-v|--verbose] <input_file>\n");
            /* Return invalid state or a specific error to indicate help requested */
            return NUR_ERR(cli_config, NUR_ERROR(NUR_E_INVALID_STATE, "Help requested by user"));
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(arg, "--host") == 0 || strcmp(arg, "-H") == 0) {
            if (i + 1 >= argc) {
                return NUR_ERR(cli_config, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, "Missing value for --host", arg));
            }
            config.host = argv[++i];
        } else if (strcmp(arg, "--port") == 0 || strcmp(arg, "-p") == 0) {
            if (i + 1 >= argc) {
                return NUR_ERR(cli_config, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, "Missing value for --port", arg));
            }
            NurResult_i32 port_res = parse_port(argv[++i]);
            if (NUR_IS_ERR(port_res)) {
                return NUR_ERR(cli_config, NUR_ERROR_OF(port_res));
            }
            config.port = NUR_VALUE(port_res);
        } else if (arg[0] == '-') {
            return NUR_ERR(cli_config, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, "Unrecognized option flag", arg));
        } else {
            /* Positional argument: input_file */
            if (config.input_file != NULL) {
                return NUR_ERR(cli_config, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, "Unexpected extra positional argument", arg));
            }
            config.input_file = arg;
        }
    }

    if (config.input_file == NULL) {
        return NUR_ERR(cli_config, NUR_ERROR(NUR_E_NOT_FOUND, "Required positional argument <input_file> is missing"));
    }

    return NUR_OK(cli_config, config);
}

static void test_scenario(const char *name, int argc, char *argv[]) {
    printf("=== Test Scenario: %s ===\n", name);
    printf("Command line: ");
    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    NurResult_cli_config res = parse_cli_args(argc, argv);
    if (NUR_IS_OK(res)) {
        CliConfig cfg = NUR_VALUE(res);
        printf("[SUCCESS] Config parsed successfully:\n");
        printf("  Host:       %s\n", cfg.host);
        printf("  Port:       %d\n", cfg.port);
        printf("  Verbose:    %s\n", cfg.verbose ? "yes" : "no");
        printf("  Input File: %s\n", cfg.input_file);
    } else {
        printf("[FAILURE] Failed to parse CLI args:\n  ");
        nur_error_eprint(&NUR_ERROR_OF(res));
    }
    printf("\n");
}

int main(void) {
    printf("========================================\n");
    printf("  nur.h CLI Argument Parser Example    \n");
    printf("========================================\n\n");

    /* Scenario 1: Happy path with full options */
    {
        char *args[] = { "app", "--host", "0.0.0.0", "--port", "9000", "-v", "data.csv" };
        test_scenario("Valid full options", 7, args);
    }

    /* Scenario 2: Happy path with default host and port */
    {
        char *args[] = { "app", "input.bin" };
        test_scenario("Valid minimal options (defaults used)", 2, args);
    }

    /* Scenario 3: Error - missing required positional argument */
    {
        char *args[] = { "app", "--port", "8080" };
        test_scenario("Missing required file", 3, args);
    }

    /* Scenario 4: Error - invalid port out of range */
    {
        char *args[] = { "app", "--port", "70000", "file.txt" };
        test_scenario("Out of range port", 4, args);
    }

    /* Scenario 5: Error - unrecognized flag */
    {
        char *args[] = { "app", "--unknown-flag", "file.txt" };
        test_scenario("Unrecognized option flag", 3, args);
    }

    /* Scenario 6: Error - missing flag value */
    {
        char *args[] = { "app", "--host" };
        test_scenario("Missing flag parameter value", 2, args);
    }

    return 0;
}
