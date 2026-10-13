/*
 * 01_config_parser.c - INI-style configuration file parser
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 01_config_parser.c -o config_parser
 * Run: ./config_parser
 *
 * Demonstrates:
 * - Real-world configuration file parsing
 * - File I/O combined with string processing
 * - NurOption for optional configuration values
 * - Complete error handling through parsing pipeline
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <nur.h>

/* Configuration entry structure */
typedef struct {
    char key[64];     /* Configuration key */
    char value[256];  /* Configuration value */
} ConfigEntry;

/* Configuration file structure */
typedef struct {
    ConfigEntry entries[64]; /* Fixed array of entries */
    int32_t count;          /* Number of valid entries */
} ConfigFile;

/* Generate NurOption type for configuration values */
NUR_OPTION_TYPE(const char *, ccstr);

/**
 * Parse a single configuration line in "key=value" format.
 * Handles comments (lines starting with #) and empty lines.
 */
NUR_NODISCARD
static NurResult_void config_parse_line(const char *line, ConfigEntry *out) {
    if (line == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Config line cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Output entry cannot be NULL"));
    }
    
    /* Skip leading whitespace */
    while (*line == ' ' || *line == '\t') {
        line++;
    }
    
    /* Skip empty lines and comments */
    if (*line == '\0' || *line == '\n' || *line == '#') {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_PARSE, "Line is empty or comment"));
    }
    
    /* Find the '=' separator */
    const char *equals = strchr(line, '=');
    if (equals == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_FORMAT, "Config line must contain '=' separator"));
    }
    
    /* Calculate key length */
    size_t key_len = (size_t)(equals - line);
    
    /* Remove trailing whitespace from key */
    while (key_len > 0 && (line[key_len - 1] == ' ' || line[key_len - 1] == '\t')) {
        key_len--;
    }
    
    if (key_len == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_FORMAT, "Config key cannot be empty"));
    }
    
    if (key_len >= sizeof(out->key)) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Config key too long"));
    }
    
    /* Extract the key */
    memcpy(out->key, line, key_len);
    out->key[key_len] = '\0';
    
    /* Move past '=' and skip leading whitespace in value */
    const char *value_start = equals + 1;
    while (*value_start == ' ' || *value_start == '\t') {
        value_start++;
    }
    
    /* Find end of value (end of line or start of comment) */
    const char *value_end = value_start;
    while (*value_end != '\0' && *value_end != '\n' && *value_end != '#') {
        value_end++;
    }
    
    /* Remove trailing whitespace from value */
    while (value_end > value_start && (value_end[-1] == ' ' || value_end[-1] == '\t' || value_end[-1] == '\r')) {
        value_end--;
    }
    
    size_t value_len = (size_t)(value_end - value_start);
    if (value_len >= sizeof(out->value)) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Config value too long"));
    }
    
    /* Extract the value */
    if (value_len > 0) {
        memcpy(out->value, value_start, value_len);
    }
    out->value[value_len] = '\0';
    
    return NUR_OK_VOID();
}

/**
 * Load configuration from a file path.
 */
NUR_NODISCARD
static NurResult_void config_load(const char *path, ConfigFile *out) {
    if (path == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Config file path cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Output config cannot be NULL"));
    }
    
    /* Initialize the config structure */
    out->count = 0;
    
    /* Open the file */
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return NUR_ERR_VOID(NUR_ERROR_ERRNO_CTX("Failed to open config file", path));
    }
    
    char line_buffer[512];
    int32_t line_number = 0;
    
    /* Read and parse each line */
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
        line_number++;
        
        /* Check if we have room for more entries */
        if (out->count >= (int32_t)(sizeof(out->entries) / sizeof(out->entries[0]))) {
            fclose(fp);
            return NUR_ERR_VOID(NUR_ERROR(NUR_E_RESOURCE_LIMIT, "Too many config entries"));
        }
        
        /* Try to parse the line */
        ConfigEntry entry;
        NurResult_void parse_result = config_parse_line(line_buffer, &entry);
        
        if (NUR_IS_OK(parse_result)) {
            /* Successfully parsed - add to config */
            out->entries[out->count] = entry;
            out->count++;
        } else {
            /* Check if it's a benign error (empty line/comment) */
            nur_error_t error = parse_result.error;
            if (error.code != NUR_E_PARSE) {
                /* Real parsing error - report with line number */
                fclose(fp);
                
                /* Create a new error with line context */
                char line_context[64];
                snprintf(line_context, sizeof(line_context), "line %d", line_number);
                
                return NUR_ERR_VOID(NUR_ERROR_CTX(error.code, error.message, line_context));
            }
            /* Otherwise ignore the line (comment/empty) */
        }
    }
    
    /* Check for read error */
    if (ferror(fp)) {
        fclose(fp);
        return NUR_ERR_VOID(NUR_ERROR_ERRNO_CTX("Error reading config file", path));
    }
    
    fclose(fp);
    return NUR_OK_VOID();
}

/**
 * Get a configuration value by key.
 * Returns NurOption_ccstr - Some(value) if found, None if not found.
 */
static NurOption_ccstr config_get(const ConfigFile *cfg, const char *key) {
    if (cfg == NULL || key == NULL) {
        return NUR_NONE(ccstr);
    }
    
    /* Linear search through entries */
    for (int32_t i = 0; i < cfg->count; i++) {
        if (strcmp(cfg->entries[i].key, key) == 0) {
            return NUR_SOME(ccstr, cfg->entries[i].value);
        }
    }
    
    return NUR_NONE(ccstr);
}

/**
 * Get a configuration value as an integer.
 */
NUR_NODISCARD
static NurResult_i32 config_get_int(const ConfigFile *cfg, const char *key) {
    if (cfg == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Config cannot be NULL"));
    }
    
    if (key == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Key cannot be NULL"));
    }
    
    NurOption_ccstr value_opt = config_get(cfg, key);
    if (NUR_IS_NONE(value_opt)) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_NOT_FOUND, "Configuration key not found", key));
    }
    
    const char *value_str = NUR_OPTION_VALUE(value_opt);
    
    /* Parse as integer */
    if (strlen(value_str) == 0) {
        return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_INVALID_ARGUMENT, "Configuration value is empty", key));
    }
    
    /* Check for valid digits */
    bool negative = false;
    size_t start_idx = 0;
    
    if (value_str[0] == '-') {
        negative = true;
        start_idx = 1;
        if (strlen(value_str) == 1) {
            return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_PARSE, "Configuration value is just a minus sign", key));
        }
    }
    
    for (size_t i = start_idx; value_str[i] != '\0'; i++) {
        if (value_str[i] < '0' || value_str[i] > '9') {
            return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_PARSE, "Configuration value contains non-digit characters", key));
        }
    }
    
    /* Convert to integer */
    int32_t result = 0;
    for (size_t i = start_idx; value_str[i] != '\0'; i++) {
        int32_t digit = value_str[i] - '0';
        
        /* Check for overflow */
        if (negative) {
            if (result < (INT32_MIN + digit) / 10) {
                return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OVERFLOW, "Configuration value too small for int32", key));
            }
            result = result * 10 - digit;
        } else {
            if (result > (INT32_MAX - digit) / 10) {
                return NUR_ERR(i32, NUR_ERROR_CTX(NUR_E_OVERFLOW, "Configuration value too large for int32", key));
            }
            result = result * 10 + digit;
        }
    }
    
    return NUR_OK(i32, result);
}

/**
 * Create a temporary config file for testing.
 */
NUR_NODISCARD
static NurResult_cstr create_test_config(void) {
    /* Create temp filename */
    static char temp_filename[256];
    time_t now = time(NULL);
    snprintf(temp_filename, sizeof(temp_filename), 
#ifdef _WIN32
             "C:\\Windows\\Temp\\nur_config_%ld.ini", 
#else
             "/tmp/nur_config_%ld.ini", 
#endif
             (long)now);
    
    /* Create test config content */
    const char *config_content = 
        "# Server Configuration\n"
        "server_port = 8080\n"
        "server_host = localhost\n"
        "\n"
        "# Database Settings\n"
        "db_host = db.example.com\n"
        "db_port = 5432\n"
        "db_name = myapp\n"
        "db_timeout = 30\n"
        "\n"
        "# Application Settings\n"
        "max_connections = 100\n"
        "log_level = info\n"
        "debug_mode = false\n"
        "\n"
        "# Paths\n"
        "data_dir = /var/lib/myapp\n"
        "log_dir = /var/log/myapp  # With inline comment\n"
        "\n"
        "# Invalid lines for testing\n"
        "# This is a comment\n"
        "\n"
        "valid_key = valid_value\n";
    
    /* Write to file */
    FILE *fp = fopen(temp_filename, "w");
    if (fp == NULL) {
        return NUR_ERR(cstr, NUR_ERROR_ERRNO("Failed to create test config file"));
    }
    
    if (fputs(config_content, fp) == EOF) {
        fclose(fp);
        return NUR_ERR(cstr, NUR_ERROR_ERRNO("Failed to write test config content"));
    }
    
    if (fclose(fp) != 0) {
        return NUR_ERR(cstr, NUR_ERROR_ERRNO("Failed to close test config file"));
    }
    
    return NUR_OK(cstr, temp_filename);
}

/**
 * Clean up test file.
 */
static void cleanup_test_file(const char *filename) {
    if (filename != NULL) {
        remove(filename); /* Ignore errors */
    }
}

int main(void) {
    printf("=== nur.h Configuration Parser Demonstration ===\n\n");
    
    /* Example 1: Create test configuration file */
    printf("1. Creating test configuration file:\n");
    
    NurResult_cstr config_file = create_test_config();
    if (NUR_IS_ERR(config_file)) {
        printf("   Failed to create test config: ");
        nur_error_eprint(&NUR_ERROR_OF(config_file));
        return 1;
    }
    
    char *filename = NUR_VALUE(config_file);
    printf("   Created test config: %s\n", filename);
    
    /* Example 2: Load configuration */
    printf("\n2. Loading configuration file:\n");
    
    ConfigFile config;
    NurResult_void load_result = config_load(filename, &config);
    if (NUR_IS_ERR(load_result)) {
        printf("   Failed to load config: ");
        nur_error_eprint(&load_result.error);
        cleanup_test_file(filename);
        return 1;
    }
    
    printf("   Successfully loaded %d configuration entries\n", config.count);
    
    /* Example 3: Look up configuration values */
    printf("\n3. Looking up configuration values:\n");
    
    const char *test_keys[] = {
        "server_port",
        "server_host", 
        "db_timeout",
        "log_level",
        "nonexistent_key",
        "debug_mode"
    };
    
    for (size_t i = 0; i < 6; i++) {
        NurOption_ccstr value_opt = config_get(&config, test_keys[i]);
        if (NUR_IS_SOME(value_opt)) {
            printf("   %s = %s\n", test_keys[i], NUR_OPTION_VALUE(value_opt));
        } else {
            printf("   %s = (not found)\n", test_keys[i]);
        }
    }
    
    /* Example 4: Get integer values */
    printf("\n4. Parsing integer configuration values:\n");
    
    const char *int_keys[] = {
        "server_port",
        "db_port", 
        "max_connections",
        "db_timeout",
        "log_level"  /* Should fail - not a number */
    };
    
    for (size_t i = 0; i < 5; i++) {
        NurResult_i32 int_result = config_get_int(&config, int_keys[i]);
        if (NUR_IS_OK(int_result)) {
            printf("   %s = %d\n", int_keys[i], NUR_VALUE(int_result));
        } else {
            printf("   %s = (error): ", int_keys[i]);
            nur_error_eprint(&NUR_ERROR_OF(int_result));
        }
    }
    
    /* Example 5: Test error conditions */
    printf("\n5. Testing error conditions:\n");
    
    /* Try to load non-existent file */
    ConfigFile dummy_config;
    NurResult_void bad_load = config_load("nonexistent_file.conf", &dummy_config);
    if (NUR_IS_ERR(bad_load)) {
        printf("   Non-existent file error: ");
        nur_error_eprint(&bad_load.error);
    }
    
    /* Test parsing errors with malformed content */
    printf("   Testing line parsing errors:\n");
    
    const char *bad_lines[] = {
        "key_without_equals",
        "=value_without_key",
        " = both_empty",
        "very_long_key_that_exceeds_the_maximum_allowed_length_for_configuration_keys_in_this_system = value"
    };
    
    for (size_t i = 0; i < 4; i++) {
        ConfigEntry dummy_entry;
        NurResult_void parse_result = config_parse_line(bad_lines[i], &dummy_entry);
        if (NUR_IS_ERR(parse_result)) {
            printf("     \"%s\": ", bad_lines[i]);
            nur_error_eprint(&parse_result.error);
        }
    }
    
    /* Example 6: Configuration validation */
    printf("\n6. Configuration validation example:\n");
    
    /* Check required configuration */
    const char *required_keys[] = {"server_port", "server_host", "db_host"};
    bool all_required_present = true;
    
    for (size_t i = 0; i < 3; i++) {
        NurOption_ccstr value = config_get(&config, required_keys[i]);
        if (NUR_IS_NONE(value)) {
            printf("   Missing required config: %s\n", required_keys[i]);
            all_required_present = false;
        }
    }
    
    if (all_required_present) {
        printf("   All required configuration keys are present\n");
        
        /* Validate port ranges */
        NurResult_i32 port = config_get_int(&config, "server_port");
        if (NUR_IS_OK(port)) {
            int32_t port_val = NUR_VALUE(port);
            if (port_val < 1 || port_val > 65535) {
                printf("   Warning: server_port %d is out of valid range [1-65535]\n", port_val);
            } else {
                printf("   server_port %d is in valid range\n", port_val);
            }
        }
    }
    
    /* Example 7: Cleanup */
    printf("\n7. Cleaning up:\n");
    cleanup_test_file(filename);
    printf("   Test configuration file cleaned up\n");
    
    printf("\n=== Configuration parser demonstration completed ===\n");
    return 0;
}
