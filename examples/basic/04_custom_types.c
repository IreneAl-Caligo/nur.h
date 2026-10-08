/*
 * 04_custom_types.c - Custom types with NUR_RESULT_TYPE
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 04_custom_types.c -o custom_types
 * Run: ./custom_types
 *
 * Demonstrates:
 * - Defining custom structs
 * - Using NUR_RESULT_TYPE to create result types for custom structs
 * - Error propagation with custom types
 * - Validation functions returning results
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <nur.h>

/* Custom data structures */
typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    char name[32];
    int32_t port;
    bool enabled;
} Config;

/* Generate result types for our custom structs */
NUR_RESULT_TYPE(Point, point)
NUR_RESULT_TYPE(Config, config)

/**
 * Create a point with validation.
 * Coordinates must be in range [0, 1000].
 */
NUR_NODISCARD
static NurResult_point make_point(float x, float y) {
    if (x < 0.0f || x > 1000.0f) {
        return NUR_ERR(point, NUR_ERROR(NUR_E_OUT_OF_RANGE, "X coordinate must be in range [0, 1000]"));
    }
    
    if (y < 0.0f || y > 1000.0f) {
        return NUR_ERR(point, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Y coordinate must be in range [0, 1000]"));
    }
    
    Point p = { .x = x, .y = y };
    return NUR_OK(point, p);
}

/**
 * Calculate distance between two points.
 * Both points must be valid.
 */
NUR_NODISCARD
static NurResult_f32 point_distance(const Point *p1, const Point *p2) {
    if (p1 == NULL || p2 == NULL) {
        return NUR_ERR(f32, NUR_ERROR(NUR_E_NULL_POINTER, "Point cannot be NULL"));
    }
    
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    float distance = dx * dx + dy * dy; /* Simplified - no sqrt for demo */
    
    return NUR_OK(f32, distance);
}

/**
 * Parse configuration from "name=value" format.
 * Supports: name=string, port=number, enabled=true/false
 */
NUR_NODISCARD
static NurResult_config parse_config(const char *config_str) {
    if (config_str == NULL) {
        return NUR_ERR(config, NUR_ERROR(NUR_E_NULL_POINTER, "Config string cannot be NULL"));
    }
    
    /* Find the '=' separator */
    const char *equals = strchr(config_str, '=');
    if (equals == NULL) {
        return NUR_ERR(config, NUR_ERROR(NUR_E_FORMAT, "Config must be in 'key=value' format"));
    }
    
    size_t key_len = (size_t)(equals - config_str);
    if (key_len == 0) {
        return NUR_ERR(config, NUR_ERROR(NUR_E_FORMAT, "Config key cannot be empty"));
    }
    
    const char *value = equals + 1;
    if (*value == '\0') {
        return NUR_ERR(config, NUR_ERROR(NUR_E_FORMAT, "Config value cannot be empty"));
    }
    
    Config cfg = { .name = {0}, .port = 0, .enabled = false };
    
    /* Parse based on key */
    if (key_len == 4 && strncmp(config_str, "name", 4) == 0) {
        if (strlen(value) >= sizeof(cfg.name)) {
            return NUR_ERR(config, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Name too long"));
        }
        strcpy(cfg.name, value);
    }
    else if (key_len == 4 && strncmp(config_str, "port", 4) == 0) {
        /* Simple integer parsing */
        int32_t port = 0;
        for (const char *p = value; *p != '\0'; p++) {
            if (*p < '0' || *p > '9') {
                return NUR_ERR(config, NUR_ERROR(NUR_E_PARSE, "Port must be a number"));
            }
            port = port * 10 + (*p - '0');
            if (port > 65535) {
                return NUR_ERR(config, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Port must be <= 65535"));
            }
        }
        if (port == 0) {
            return NUR_ERR(config, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Port must be > 0"));
        }
        cfg.port = port;
    }
    else if (key_len == 7 && strncmp(config_str, "enabled", 7) == 0) {
        if (strcmp(value, "true") == 0) {
            cfg.enabled = true;
        } else if (strcmp(value, "false") == 0) {
            cfg.enabled = false;
        } else {
            return NUR_ERR(config, NUR_ERROR(NUR_E_PARSE, "Enabled must be 'true' or 'false'"));
        }
    }
    else {
        return NUR_ERR(config, NUR_ERROR(NUR_E_UNSUPPORTED, "Unknown config key"));
    }
    
    return NUR_OK(config, cfg);
}

/**
 * Validate a configuration object.
 * Ensures all required fields are properly set.
 */
NUR_NODISCARD
static NurResult_void validate_config(const Config *cfg) {
    if (cfg == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Config cannot be NULL"));
    }
    
    if (cfg->name[0] == '\0') {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_STATE, "Config name is required"));
    }
    
    if (cfg->port <= 0 || cfg->port > 65535) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_OUT_OF_RANGE, "Config port must be in range [1, 65535]"));
    }
    
    return NUR_OK_VOID();
}

/**
 * Create a point from two separate coordinate results.
 * Demonstrates error propagation with custom types.
 */
NUR_NODISCARD
static NurResult_point create_point_from_coords(NurResult_f32 x_result, NurResult_f32 y_result) {
    if (NUR_IS_ERR(x_result)) {
        return NUR_ERR(point, NUR_ERROR_OF(x_result));
    }
    
    if (NUR_IS_ERR(y_result)) {
        return NUR_ERR(point, NUR_ERROR_OF(y_result));
    }
    
    return make_point(NUR_VALUE(x_result), NUR_VALUE(y_result));
}

int main(void) {
    printf("=== nur.h Custom Types Demonstration ===\n\n");
    
    /* Example 1: Basic point creation and validation */
    printf("1. Point creation with validation:\n");
    
    NurResult_point p1 = make_point(10.5f, 20.3f);
    if (NUR_IS_OK(p1)) {
        Point point = NUR_VALUE(p1);
        printf("   Valid point: (%.1f, %.1f)\n", point.x, point.y);
    } else {
        printf("   Error: %s\n", NUR_ERROR_OF(p1).message);
    }
    
    NurResult_point p2 = make_point(-5.0f, 15.0f);
    if (NUR_IS_OK(p2)) {
        Point point = NUR_VALUE(p2);
        printf("   Valid point: (%.1f, %.1f)\n", point.x, point.y);
    } else {
        printf("   Error: %s\n", NUR_ERROR_OF(p2).message);
    }
    
    /* Example 2: Point distance calculation */
    printf("\n2. Point distance calculation:\n");
    
    NurResult_point pa = make_point(0.0f, 0.0f);
    NurResult_point pb = make_point(3.0f, 4.0f);
    
    if (NUR_IS_OK(pa) && NUR_IS_OK(pb)) {
        Point point_a = NUR_VALUE(pa);
        Point point_b = NUR_VALUE(pb);
        
        NurResult_f32 dist = point_distance(&point_a, &point_b);
        if (NUR_IS_OK(dist)) {
            printf("   Distance² between points: %.1f\n", NUR_VALUE(dist));
        } else {
            printf("   Distance calculation error: %s\n", NUR_ERROR_OF(dist).message);
        }
    } else {
        printf("   Cannot calculate distance - invalid points\n");
    }
    
    /* Example 3: Configuration parsing */
    printf("\n3. Configuration parsing:\n");
    
    const char *config_strings[] = {
        "name=myserver",
        "port=8080", 
        "enabled=true",
        "port=99999",  /* Invalid - too large */
        "invalid",     /* Invalid format */
        "unknown=value" /* Unknown key */
    };
    
    for (size_t i = 0; i < 6; i++) {
        NurResult_config cfg_result = parse_config(config_strings[i]);
        if (NUR_IS_OK(cfg_result)) {
            Config cfg = NUR_VALUE(cfg_result);
            printf("   \"%s\" -> name:'%s' port:%d enabled:%s\n", 
                   config_strings[i], cfg.name, cfg.port, cfg.enabled ? "true" : "false");
        } else {
            printf("   \"%s\" -> Error: %s\n", 
                   config_strings[i], NUR_ERROR_OF(cfg_result).message);
        }
    }
    
    /* Example 4: Configuration validation */
    printf("\n4. Configuration validation:\n");
    
    /* Create a valid config */
    Config valid_cfg = { .name = "webserver", .port = 80, .enabled = true };
    NurResult_void validation1 = validate_config(&valid_cfg);
    if (NUR_IS_OK(validation1)) {
        printf("   Valid config passed validation\n");
    } else {
        printf("   Validation failed: %s\n", validation1.error.message);
    }
    
    /* Create an invalid config */
    Config invalid_cfg = { .name = "", .port = -1, .enabled = false };
    NurResult_void validation2 = validate_config(&invalid_cfg);
    if (NUR_IS_OK(validation2)) {
        printf("   Invalid config somehow passed validation\n");
    } else {
        printf("   Validation correctly failed: %s\n", validation2.error.message);
    }
    
    /* Example 5: Error propagation */
    printf("\n5. Error propagation with custom types:\n");
    
    NurResult_f32 x_coord = NUR_OK(f32, 50.0f);
    NurResult_f32 y_coord = NUR_ERR(f32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Y coordinate source failed"));
    
    NurResult_point combined = create_point_from_coords(x_coord, y_coord);
    if (NUR_IS_OK(combined)) {
        Point p = NUR_VALUE(combined);
        printf("   Combined point: (%.1f, %.1f)\n", p.x, p.y);
    } else {
        printf("   Error propagated: %s\n", NUR_ERROR_OF(combined).message);
    }
    
    printf("\n=== Custom types demonstration completed ===\n");
    return 0;
}