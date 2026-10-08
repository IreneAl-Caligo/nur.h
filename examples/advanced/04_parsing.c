/*
 * 04_parsing.c - CSV parsing with comprehensive error handling
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 04_parsing.c -o parsing
 * Run: ./parsing
 *
 * Demonstrates:
 * - Zero-allocation CSV field parsing
 * - Detailed parsing error conditions
 * - Buffer size validation
 * - Field extraction with bounds checking
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <nur.h>

/**
 * Parse a single CSV field from a line at the specified field index.
 * Returns the length of the field copied to out_buf.
 * All parsing is done without dynamic allocation.
 */
NUR_NODISCARD
static NurResult_usize parse_csv_field(const char *line, int32_t field_index, 
                                       char *out_buf, size_t buf_size) {
    if (line == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "CSV line cannot be NULL"));
    }
    
    if (out_buf == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Output buffer cannot be NULL"));
    }
    
    if (buf_size == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Buffer size cannot be zero"));
    }
    
    if (field_index < 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Field index cannot be negative"));
    }
    
    /* Clear output buffer */
    out_buf[0] = '\0';
    
    /* Handle empty line */
    if (strlen(line) == 0) {
        if (field_index == 0) {
            return NUR_OK(usize, 0); /* Empty field is valid for index 0 */
        } else {
            return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Field index exceeds available fields"));
        }
    }
    
    /* Find the start of the target field */
    const char *field_start = line;
    int32_t current_field = 0;
    
    while (current_field < field_index && *field_start != '\0') {
        /* Skip to next comma */
        while (*field_start != ',' && *field_start != '\0') {
            field_start++;
        }
        
        if (*field_start == ',') {
            field_start++; /* Move past comma */
            current_field++;
        } else {
            /* Reached end of line before finding field */
            return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Field index exceeds available fields"));
        }
    }
    
    /* Find the end of the field */
    const char *field_end = field_start;
    while (*field_end != ',' && *field_end != '\0') {
        field_end++;
    }
    
    /* Calculate field length */
    size_t field_length = (size_t)(field_end - field_start);
    
    /* Check if field fits in buffer (including null terminator) */
    if (field_length >= buf_size) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Field too large for output buffer"));
    }
    
    /* Copy field to output buffer */
    if (field_length > 0) {
        memcpy(out_buf, field_start, field_length);
    }
    out_buf[field_length] = '\0';
    
    return NUR_OK(usize, field_length);
}

/**
 * Parse a complete CSV record into an array of field buffers.
 * Returns the number of fields successfully parsed.
 */
NUR_NODISCARD
static NurResult_i32 parse_csv_record(const char *line, char fields[][64], int32_t max_fields) {
    if (line == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "CSV line cannot be NULL"));
    }
    
    if (fields == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Fields array cannot be NULL"));
    }
    
    if (max_fields <= 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Max fields must be positive"));
    }
    
    /* Handle empty line */
    if (strlen(line) == 0) {
        return NUR_OK(i32, 0); /* Zero fields is valid for empty line */
    }
    
    int32_t field_count = 0;
    
    /* Parse fields one by one until we hit an error or run out */
    for (int32_t i = 0; i < max_fields; i++) {
        NurResult_usize field_result = parse_csv_field(line, i, fields[i], 64);
        
        if (NUR_IS_OK(field_result)) {
            field_count++;
        } else {
            nur_error_t error = NUR_ERROR_OF(field_result);
            if (error.code == NUR_E_OUT_OF_RANGE) {
                /* No more fields available - this is normal */
                break;
            } else {
                /* Real parsing error - propagate it */
                return NUR_ERR(i32, error);
            }
        }
    }
    
    return NUR_OK(i32, field_count);
}

/**
 * Validate that a CSV field contains only valid characters.
 * For demonstration, we'll check that it doesn't contain control characters.
 */
NUR_NODISCARD
static NurResult_void validate_csv_field(const char *field, const char *field_name) {
    if (field == NULL) {
        return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_NULL_POINTER, "Field cannot be NULL", field_name));
    }
    
    size_t length = strlen(field);
    
    /* Check for reasonable field length */
    if (length > 1000) {
        return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_OUT_OF_RANGE, "Field too long (max 1000 chars)", field_name));
    }
    
    /* Check each character */
    for (size_t i = 0; i < length; i++) {
        unsigned char ch = (unsigned char)field[i];
        
        /* Reject control characters except tab */
        if (ch < 32 && ch != '\t') {
            return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_FORMAT, "Field contains invalid control characters", field_name));
        }
        
        /* Reject DEL character */
        if (ch == 127) {
            return NUR_ERR_VOID(NUR_ERROR_CTX(NUR_E_FORMAT, "Field contains DEL character", field_name));
        }
    }
    
    return NUR_OK_VOID();
}

/**
 * Parse and validate a CSV line representing a person record.
 * Expected format: "name,age,email,city"
 */
NUR_NODISCARD
static NurResult_void parse_person_record(const char *csv_line) {
    if (csv_line == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "CSV line cannot be NULL"));
    }
    
    /* Parse the record into fields */
    char fields[10][64]; /* Support up to 10 fields of 64 chars each */
    int32_t field_count;
    NUR_TRY_TO(void, i32, field_count, parse_csv_record(csv_line, fields, 10));
    
    /* Validate we have exactly 4 fields */
    if (field_count != 4) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_FORMAT, "Person record must have exactly 4 fields (name,age,email,city)"));
    }
    
    /* Validate individual fields */
    const char *field_names[] = {"name", "age", "email", "city"};
    for (int32_t i = 0; i < 4; i++) {
        NUR_TRY_VOID_FROM_VOID(validate_csv_field(fields[i], field_names[i]));
    }
    
    /* Additional validation for specific fields */
    
    /* Name cannot be empty */
    if (strlen(fields[0]) == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Name field cannot be empty"));
    }
    
    /* Age must be a valid number */
    const char *age_str = fields[1];
    if (strlen(age_str) == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Age field cannot be empty"));
    }
    
    for (size_t i = 0; age_str[i] != '\0'; i++) {
        if (age_str[i] < '0' || age_str[i] > '9') {
            return NUR_ERR_VOID(NUR_ERROR(NUR_E_PARSE, "Age field must contain only digits"));
        }
    }
    
    /* Email must contain @ symbol */
    if (strchr(fields[2], '@') == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_FORMAT, "Email field must contain @ symbol"));
    }
    
    /* City cannot be empty */
    if (strlen(fields[3]) == 0) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_INVALID_ARGUMENT, "City field cannot be empty"));
    }
    
    /* If we get here, the record is valid */
    printf("   Valid person: Name='%s', Age='%s', Email='%s', City='%s'\n",
           fields[0], fields[1], fields[2], fields[3]);
    
    return NUR_OK_VOID();
}

/**
 * Count the number of fields in a CSV line.
 */
NUR_NODISCARD
static NurResult_i32 count_csv_fields(const char *line) {
    if (line == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "CSV line cannot be NULL"));
    }
    
    if (strlen(line) == 0) {
        return NUR_OK(i32, 0); /* Empty line has zero fields */
    }
    
    int32_t field_count = 1; /* At least one field if line is not empty */
    
    /* Count commas to determine field count */
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') {
            field_count++;
            
            /* Prevent integer overflow */
            if (field_count < 0) {
                return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Too many fields in CSV line"));
            }
        }
    }
    
    return NUR_OK(i32, field_count);
}

int main(void) {
    printf("=== nur.h CSV Parsing Demonstration ===\n\n");
    
    /* Example 1: Basic field parsing */
    printf("1. Basic CSV field parsing:\n");
    
    const char *test_line = "Alice,25,alice@example.com,New York";
    char field_buffer[64];
    
    for (int32_t i = 0; i < 5; i++) { /* Try to parse 5 fields (should find 4) */
        NurResult_usize field_result = parse_csv_field(test_line, i, field_buffer, sizeof(field_buffer));
        
        if (NUR_IS_OK(field_result)) {
            size_t length = NUR_VALUE(field_result);
            printf("   Field %d: '%s' (length: %zu)\n", i, field_buffer, length);
        } else {
            nur_error_t error = NUR_ERROR_OF(field_result);
            if (error.code == NUR_E_OUT_OF_RANGE) {
                printf("   Field %d: (not found - end of fields)\n", i);
            } else {
                printf("   Field %d: Error - ", i);
                nur_error_eprint(&error);
            }
        }
    }
    
    /* Example 2: Complete record parsing */
    printf("\n2. Complete CSV record parsing:\n");
    
    const char *test_records[] = {
        "John,30,john@test.com,Boston",
        "Mary,28,mary@test.com,Seattle", 
        "Bob,35,bob@test.com,Chicago",
        "Alice,22", /* Missing fields */
        "",         /* Empty line */
        "Eve,abc,eve@test.com,Portland" /* Invalid age */
    };
    
    for (size_t i = 0; i < 6; i++) {
        printf("   Record %zu: \"%s\"\n", i + 1, test_records[i]);
        
        char record_fields[10][64];
        NurResult_i32 parse_result = parse_csv_record(test_records[i], record_fields, 10);
        
        if (NUR_IS_OK(parse_result)) {
            int32_t field_count = NUR_VALUE(parse_result);
            printf("     Parsed %d fields:", field_count);
            for (int32_t j = 0; j < field_count; j++) {
                printf(" '%s'", record_fields[j]);
            }
            printf("\n");
        } else {
            printf("     Parse error: ");
            nur_error_eprint(&NUR_ERROR_OF(parse_result));
        }
    }
    
    /* Example 3: Field counting */
    printf("\n3. CSV field counting:\n");
    
    const char *count_tests[] = {
        "a,b,c,d",        /* 4 fields */
        "single",         /* 1 field */
        "",               /* 0 fields */
        "a,b,",           /* 3 fields (last is empty) */
        "one,two,three,four,five,six,seven,eight" /* 8 fields */
    };
    
    for (size_t i = 0; i < 5; i++) {
        NurResult_i32 count_result = count_csv_fields(count_tests[i]);
        if (NUR_IS_OK(count_result)) {
            printf("   \"%s\" has %d fields\n", count_tests[i], NUR_VALUE(count_result));
        } else {
            printf("   \"%s\" count error: ", count_tests[i]);
            nur_error_eprint(&NUR_ERROR_OF(count_result));
        }
    }
    
    /* Example 4: Person record validation */
    printf("\n4. Person record validation:\n");
    
    const char *person_records[] = {
        "Alice Johnson,25,alice.johnson@email.com,New York",
        "Bob Smith,30,bob@company.org,San Francisco", 
        "Carol,28,carol.missing.at.com,Boston",  /* Invalid email */
        "David Wilson,abc,david@test.com,Chicago", /* Invalid age */
        "",                                       /* Empty record */
        "Eve,25,eve@test.com",                   /* Missing city */
        "Frank Miller,35,frank@test.com,Seattle" /* Valid */
    };
    
    for (size_t i = 0; i < 7; i++) {
        printf("   Processing: \"%s\"\n", person_records[i]);
        
        NurResult_void validation = parse_person_record(person_records[i]);
        if (NUR_IS_ERR(validation)) {
            printf("     Validation failed: ");
            nur_error_eprint(&validation.error);
        }
    }
    
    /* Example 5: Error condition testing */
    printf("\n5. Error condition testing:\n");
    
    /* Buffer too small */
    char small_buffer[5];
    NurResult_usize small_result = parse_csv_field("VeryLongFieldName,other", 0, small_buffer, sizeof(small_buffer));
    if (NUR_IS_ERR(small_result)) {
        printf("   Buffer too small error: ");
        nur_error_eprint(&NUR_ERROR_OF(small_result));
    }
    
    /* Negative field index */
    NurResult_usize neg_result = parse_csv_field("a,b,c", -1, field_buffer, sizeof(field_buffer));
    if (NUR_IS_ERR(neg_result)) {
        printf("   Negative index error: ");
        nur_error_eprint(&NUR_ERROR_OF(neg_result));
    }
    
    /* NULL inputs */
    NurResult_usize null_result = parse_csv_field(NULL, 0, field_buffer, sizeof(field_buffer));
    if (NUR_IS_ERR(null_result)) {
        printf("   NULL input error: ");
        nur_error_eprint(&NUR_ERROR_OF(null_result));
    }
    
    printf("\n=== CSV parsing demonstration completed ===\n");
    return 0;
}