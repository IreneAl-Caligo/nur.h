/*
 * 02_string_processing.c - Text processing pipeline with error handling
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 02_string_processing.c -o string_processing
 * Run: ./string_processing
 *
 * Demonstrates:
 * - String processing pipeline with multiple stages
 * - Buffer size validation and safety
 * - Text transformation with comprehensive error handling
 * - Chaining multiple string operations with error propagation
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <nur.h>

/**
 * Remove leading and trailing whitespace from a string.
 * Returns the length of the trimmed string.
 */
NUR_NODISCARD
static NurResult_usize trim_whitespace(const char *in, char *out, size_t out_len) {
    if (in == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Input string cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Output buffer cannot be NULL"));
    }
    
    if (out_len == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Output buffer size cannot be zero"));
    }
    
    /* Clear output buffer */
    out[0] = '\0';
    
    /* Find start of non-whitespace */
    const char *start = in;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    
    /* Handle empty or all-whitespace string */
    if (*start == '\0') {
        return NUR_OK(usize, 0); /* Empty result */
    }
    
    /* Find end of non-whitespace */
    const char *end = in + strlen(in) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    
    /* Calculate trimmed length */
    size_t trimmed_len = (size_t)(end - start + 1);
    
    /* Check if result fits in output buffer */
    if (trimmed_len >= out_len) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Trimmed string too large for output buffer"));
    }
    
    /* Copy trimmed string */
    memcpy(out, start, trimmed_len);
    out[trimmed_len] = '\0';
    
    return NUR_OK(usize, trimmed_len);
}

/**
 * Convert string to uppercase.
 * Returns the length of the converted string.
 */
NUR_NODISCARD
static NurResult_usize to_uppercase(const char *in, char *out, size_t out_len) {
    if (in == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Input string cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Output buffer cannot be NULL"));
    }
    
    if (out_len == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Output buffer size cannot be zero"));
    }
    
    size_t input_len = strlen(in);
    
    /* Check if input fits in output buffer */
    if (input_len >= out_len) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Input string too large for output buffer"));
    }
    
    /* Convert each character to uppercase */
    for (size_t i = 0; i < input_len; i++) {
        out[i] = (char)toupper((unsigned char)in[i]);
    }
    out[input_len] = '\0';
    
    return NUR_OK(usize, input_len);
}

/**
 * Count words in a text string.
 * Words are separated by whitespace.
 */
NUR_NODISCARD
static NurResult_i32 count_words(const char *text) {
    if (text == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Text string cannot be NULL"));
    }
    
    int32_t word_count = 0;
    bool in_word = false;
    
    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)text[i];
        
        if (isspace(ch)) {
            in_word = false;
        } else {
            if (!in_word) {
                word_count++;
                
                /* Check for overflow */
                if (word_count < 0) {
                    return NUR_ERR(i32, NUR_ERROR(NUR_E_OVERFLOW, "Word count exceeds maximum"));
                }
                
                in_word = true;
            }
        }
    }
    
    return NUR_OK(i32, word_count);
}

/**
 * Remove extra whitespace (collapse multiple spaces to single space).
 * Returns the length of the normalized string.
 */
NUR_NODISCARD
static NurResult_usize normalize_whitespace(const char *in, char *out, size_t out_len) {
    if (in == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Input string cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Output buffer cannot be NULL"));
    }
    
    if (out_len == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Output buffer size cannot be zero"));
    }
    
    size_t out_pos = 0;
    bool last_was_space = false;
    
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)in[i];
        
        if (isspace(ch)) {
            if (!last_was_space && out_pos > 0) {
                /* Add single space if not at start */
                if (out_pos >= out_len - 1) {
                    return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Normalized string too large for output buffer"));
                }
                out[out_pos++] = ' ';
                last_was_space = true;
            }
        } else {
            /* Regular character */
            if (out_pos >= out_len - 1) {
                return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Normalized string too large for output buffer"));
            }
            out[out_pos++] = (char)ch;
            last_was_space = false;
        }
    }
    
    out[out_pos] = '\0';
    return NUR_OK(usize, out_pos);
}

/**
 * Replace all occurrences of a character with another character.
 * Returns the length of the processed string.
 */
NUR_NODISCARD
static NurResult_usize replace_char(const char *in, char *out, size_t out_len, 
                                   char from_char, char to_char) {
    if (in == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Input string cannot be NULL"));
    }
    
    if (out == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Output buffer cannot be NULL"));
    }
    
    if (out_len == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Output buffer size cannot be zero"));
    }
    
    size_t input_len = strlen(in);
    
    /* Check if input fits in output buffer */
    if (input_len >= out_len) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Input string too large for output buffer"));
    }
    
    /* Copy string while replacing characters */
    for (size_t i = 0; i < input_len; i++) {
        if (in[i] == from_char) {
            out[i] = to_char;
        } else {
            out[i] = in[i];
        }
    }
    out[input_len] = '\0';
    
    return NUR_OK(usize, input_len);
}

/**
 * Complete text processing pipeline that applies multiple transformations.
 * Demonstrates error propagation through multiple stages.
 */
NUR_NODISCARD
static NurResult_void process_text(const char *input) {
    if (input == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Input text cannot be NULL"));
    }
    
    printf("   Processing: \"%s\"\n", input);
    
    /* Stage 1: Trim whitespace */
    char trimmed[256];
    size_t trimmed_len;
    NUR_TRY_TO(void, usize, trimmed_len, trim_whitespace(input, trimmed, sizeof(trimmed)));
    printf("     After trim: \"%s\" (length: %zu)\n", trimmed, trimmed_len);
    
    /* Stage 2: Normalize whitespace */
    char normalized[256];
    size_t normalized_len;
    NUR_TRY_TO(void, usize, normalized_len, normalize_whitespace(trimmed, normalized, sizeof(normalized)));
    printf("     After normalize: \"%s\" (length: %zu)\n", normalized, normalized_len);
    
    /* Stage 3: Convert to uppercase */
    char uppercase[256];
    size_t uppercase_len;
    NUR_TRY_TO(void, usize, uppercase_len, to_uppercase(normalized, uppercase, sizeof(uppercase)));
    printf("     After uppercase: \"%s\" (length: %zu)\n", uppercase, uppercase_len);
    
    /* Stage 4: Replace spaces with underscores */
    char final[256];
    size_t final_len;
    NUR_TRY_TO(void, usize, final_len, replace_char(uppercase, final, sizeof(final), ' ', '_'));
    printf("     Final result: \"%s\" (length: %zu)\n", final, final_len);
    
    /* Stage 5: Count words in original */
    int32_t word_count;
    NUR_TRY_TO(void, i32, word_count, count_words(input));
    printf("     Original word count: %d\n", word_count);
    
    return NUR_OK_VOID();
}

/**
 * Validate that a string contains only printable ASCII characters.
 */
NUR_NODISCARD
static NurResult_void validate_printable_ascii(const char *text) {
    if (text == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Text cannot be NULL"));
    }
    
    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned char ch = (unsigned char)text[i];
        
        /* Allow printable ASCII and common whitespace */
        if (ch < 32 || ch > 126) {
            if (ch != '\t' && ch != '\n' && ch != '\r') {
                return NUR_ERR_VOID(NUR_ERROR(NUR_E_FORMAT, "Text contains non-printable characters"));
            }
        }
    }
    
    return NUR_OK_VOID();
}

/**
 * Extract words from text into a fixed array.
 * Returns the number of words extracted.
 */
NUR_NODISCARD
static NurResult_i32 extract_words(const char *text, char words[][32], int32_t max_words) {
    if (text == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Text cannot be NULL"));
    }
    
    if (words == NULL) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_NULL_POINTER, "Words array cannot be NULL"));
    }
    
    if (max_words <= 0) {
        return NUR_ERR(i32, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Max words must be positive"));
    }
    
    int32_t word_count = 0;
    size_t text_pos = 0;
    size_t text_len = strlen(text);
    
    while (text_pos < text_len && word_count < max_words) {
        /* Skip whitespace */
        while (text_pos < text_len && isspace((unsigned char)text[text_pos])) {
            text_pos++;
        }
        
        if (text_pos >= text_len) {
            break; /* End of text */
        }
        
        /* Extract word */
        size_t word_start = text_pos;
        while (text_pos < text_len && !isspace((unsigned char)text[text_pos])) {
            text_pos++;
        }
        
        size_t word_len = text_pos - word_start;
        if (word_len >= 32) {
            return NUR_ERR(i32, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Word too long for buffer"));
        }
        
        /* Copy word */
        memcpy(words[word_count], text + word_start, word_len);
        words[word_count][word_len] = '\0';
        word_count++;
    }
    
    return NUR_OK(i32, word_count);
}

int main(void) {
    printf("=== nur.h String Processing Demonstration ===\n\n");
    
    /* Example 1: Individual string operations */
    printf("1. Individual string operations:\n");
    
    const char *test_string = "  Hello    World   \t\n  ";
    char buffer[128];
    
    /* Test trimming */
    NurResult_usize trim_result = trim_whitespace(test_string, buffer, sizeof(buffer));
    if (NUR_IS_OK(trim_result)) {
        printf("   Original: \"%s\"\n", test_string);
        printf("   Trimmed:  \"%s\" (length: %zu)\n", buffer, NUR_VALUE(trim_result));
    } else {
        printf("   Trim error: ");
        nur_error_eprint(&NUR_ERROR_OF(trim_result));
    }
    
    /* Test uppercase conversion */
    const char *case_test = "Hello World 123!";
    NurResult_usize upper_result = to_uppercase(case_test, buffer, sizeof(buffer));
    if (NUR_IS_OK(upper_result)) {
        printf("   Original: \"%s\"\n", case_test);
        printf("   Uppercase: \"%s\"\n", buffer);
    }
    
    /* Test word counting */
    const char *word_test = "The quick brown fox jumps over the lazy dog";
    NurResult_i32 count_result = count_words(word_test);
    if (NUR_IS_OK(count_result)) {
        printf("   Text: \"%s\"\n", word_test);
        printf("   Word count: %d\n", NUR_VALUE(count_result));
    }
    
    /* Example 2: Text processing pipeline */
    printf("\n2. Text processing pipeline:\n");
    
    const char *pipeline_inputs[] = {
        "  hello   world  ",
        "The Quick   Brown\tFox",
        "   single   ",
        "",
        "a b c d e f g h i j"
    };
    
    for (size_t i = 0; i < 5; i++) {
        printf("\n   Pipeline %zu:\n", i + 1);
        NurResult_void process_result = process_text(pipeline_inputs[i]);
        if (NUR_IS_ERR(process_result)) {
            printf("     Pipeline error: ");
            nur_error_eprint(&process_result.error);
        }
    }
    
    /* Example 3: Error condition testing */
    printf("\n3. Testing error conditions:\n");
    
    /* Buffer too small */
    char small_buffer[5];
    NurResult_usize small_result = trim_whitespace("This is a very long string", small_buffer, sizeof(small_buffer));
    if (NUR_IS_ERR(small_result)) {
        printf("   Buffer too small: ");
        nur_error_eprint(&NUR_ERROR_OF(small_result));
    }
    
    /* NULL input */
    NurResult_usize null_result = to_uppercase(NULL, buffer, sizeof(buffer));
    if (NUR_IS_ERR(null_result)) {
        printf("   NULL input: ");
        nur_error_eprint(&NUR_ERROR_OF(null_result));
    }
    
    /* Zero buffer size */
    NurResult_usize zero_result = normalize_whitespace("test", buffer, 0);
    if (NUR_IS_ERR(zero_result)) {
        printf("   Zero buffer size: ");
        nur_error_eprint(&NUR_ERROR_OF(zero_result));
    }
    
    /* Example 4: Text validation */
    printf("\n4. Text validation:\n");
    
    const char *validation_tests[] = {
        "Normal ASCII text",
        "Text with tab\tand newline\n",
        "Text with special chars: !@#$%",
        "Valid text 123"
    };
    
    for (size_t i = 0; i < 4; i++) {
        NurResult_void valid_result = validate_printable_ascii(validation_tests[i]);
        if (NUR_IS_OK(valid_result)) {
            printf("   \"%s\" - Valid\n", validation_tests[i]);
        } else {
            printf("   \"%s\" - Invalid: ", validation_tests[i]);
            nur_error_eprint(&valid_result.error);
        }
    }
    
    /* Example 5: Word extraction */
    printf("\n5. Word extraction:\n");
    
    const char *extract_text = "The quick brown fox jumps over the lazy dog";
    char extracted_words[10][32];
    
    NurResult_i32 extract_result = extract_words(extract_text, extracted_words, 10);
    if (NUR_IS_OK(extract_result)) {
        int32_t num_words = NUR_VALUE(extract_result);
        printf("   Extracted %d words from: \"%s\"\n", num_words, extract_text);
        for (int32_t i = 0; i < num_words; i++) {
            printf("     Word %d: \"%s\"\n", i + 1, extracted_words[i]);
        }
    } else {
        printf("   Word extraction error: ");
        nur_error_eprint(&NUR_ERROR_OF(extract_result));
    }
    
    /* Example 6: Character replacement */
    printf("\n6. Character replacement:\n");
    
    const char *replace_text = "hello-world-test";
    NurResult_usize replace_result = replace_char(replace_text, buffer, sizeof(buffer), '-', '_');
    if (NUR_IS_OK(replace_result)) {
        printf("   Original: \"%s\"\n", replace_text);
        printf("   Replaced: \"%s\"\n", buffer);
    }
    
    printf("\n=== String processing demonstration completed ===\n");
    return 0;
}