/*
 * 03_allocation.c - Dynamic memory allocation with NurResult
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 03_allocation.c -o allocation
 * Run: ./allocation
 *
 * Demonstrates:
 * - nur_malloc and nur_realloc wrappers
 * - Building dynamic data structures with error handling
 * - Proper cleanup on error paths
 * - Simulating out-of-memory conditions
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nur.h>

/**
 * Dynamic string buffer that can grow as needed.
 */
typedef struct {
    char *data;        /* Dynamically allocated buffer */
    size_t length;     /* Current string length */
    size_t capacity;   /* Total buffer capacity */
} StringBuf;

/**
 * Create a new string buffer with initial capacity.
 */
NUR_NODISCARD
static NurResult_ptr str_new(size_t initial_capacity) {
    if (initial_capacity == 0) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Initial capacity cannot be zero"));
    }
    
    /* Allocate the StringBuf struct */
    NurResult_ptr buf_result = nur_malloc(sizeof(StringBuf));
    if (NUR_IS_ERR(buf_result)) {
        return buf_result; /* Propagate allocation error */
    }
    
    StringBuf *buf = (StringBuf *)NUR_VALUE(buf_result);
    
    /* Allocate the data buffer */
    NurResult_ptr data_result = nur_malloc(initial_capacity);
    if (NUR_IS_ERR(data_result)) {
        free(buf); /* Clean up the struct allocation */
        return data_result; /* Propagate allocation error */
    }
    
    /* Initialize the StringBuf */
    buf->data = (char *)NUR_VALUE(data_result);
    buf->data[0] = '\0'; /* Empty string initially */
    buf->length = 0;
    buf->capacity = initial_capacity;
    
    return NUR_OK(ptr, buf);
}

/**
 * Append a string to the buffer, growing it if necessary.
 */
NUR_NODISCARD
static NurResult_void str_append(StringBuf *buf, const char *str) {
    if (buf == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "StringBuf cannot be NULL"));
    }
    
    if (str == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "String to append cannot be NULL"));
    }
    
    size_t str_len = strlen(str);
    if (str_len == 0) {
        return NUR_OK_VOID(); /* Nothing to append */
    }
    
    size_t needed_capacity = buf->length + str_len + 1; /* +1 for null terminator */
    
    /* Check if we need to grow the buffer */
    if (needed_capacity > buf->capacity) {
        /* Calculate new capacity (double current or needed, whichever is larger) */
        size_t new_capacity = buf->capacity * 2;
        if (new_capacity < needed_capacity) {
            new_capacity = needed_capacity;
        }
        
        /* Check for overflow in capacity calculation */
        if (new_capacity < buf->capacity) {
            return NUR_ERR_VOID(NUR_ERROR(NUR_E_OVERFLOW, "Buffer capacity would overflow"));
        }
        
        /* Reallocate the buffer */
        NurResult_ptr realloc_result = nur_realloc(buf->data, new_capacity);
        if (NUR_IS_ERR(realloc_result)) {
            return NUR_ERR_VOID(NUR_ERROR_OF(realloc_result));
        }
        
        buf->data = (char *)NUR_VALUE(realloc_result);
        buf->capacity = new_capacity;
    }
    
    /* Append the string */
    strcpy(buf->data + buf->length, str);
    buf->length += str_len;
    
    return NUR_OK_VOID();
}

/**
 * Get the current string content (read-only).
 */
static const char *str_content(const StringBuf *buf) {
    return (buf != NULL && buf->data != NULL) ? buf->data : "";
}

/**
 * Get buffer statistics.
 */
static void str_stats(const StringBuf *buf, size_t *length, size_t *capacity) {
    if (buf != NULL) {
        if (length != NULL) *length = buf->length;
        if (capacity != NULL) *capacity = buf->capacity;
    } else {
        if (length != NULL) *length = 0;
        if (capacity != NULL) *capacity = 0;
    }
}

/**
 * Free a string buffer and all its resources.
 */
static void str_free(StringBuf *buf) {
    if (buf != NULL) {
        if (buf->data != NULL) {
            free(buf->data);
        }
        free(buf);
    }
}

/**
 * Create a string buffer by concatenating multiple strings.
 */
NUR_NODISCARD
static NurResult_ptr str_concat(const char *strings[], size_t count) {
    if (strings == NULL) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_NULL_POINTER, "Strings array cannot be NULL"));
    }
    
    if (count == 0) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "String count cannot be zero"));
    }
    
    /* Calculate total length needed */
    size_t total_length = 0;
    for (size_t i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            size_t len = strlen(strings[i]);
            /* Check for overflow */
            if (total_length > SIZE_MAX - len) {
                return NUR_ERR(ptr, NUR_ERROR(NUR_E_OVERFLOW, "Total string length would overflow"));
            }
            total_length += len;
        }
    }
    
    /* Create buffer with exact capacity needed */
    StringBuf *buf;
    NUR_TRY_TO(ptr, ptr, buf, str_new(total_length + 1));
    
    /* Append all strings */
    for (size_t i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            NurResult_void append_result = str_append(buf, strings[i]);
            if (NUR_IS_ERR(append_result)) {
                str_free(buf); /* Clean up on error */
                return NUR_ERR(ptr, append_result.error);
            }
        }
    }
    
    return NUR_OK(ptr, buf);
}

/**
 * Simulate memory allocation with a limit to test OOM conditions.
 */
static size_t allocation_limit = SIZE_MAX; /* No limit by default */
static size_t allocated_bytes = 0;

/**
 * Set allocation limit for testing OOM scenarios.
 */
static void set_allocation_limit(size_t limit) {
    allocation_limit = limit;
    allocated_bytes = 0;
}

/**
 * Wrapper around nur_malloc that respects allocation limits.
 */
NUR_NODISCARD
static NurResult_ptr limited_malloc(size_t size) {
    if (allocated_bytes + size > allocation_limit) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_OUT_OF_MEMORY, "Allocation limit exceeded"));
    }
    
    NurResult_ptr result = nur_malloc(size);
    if (NUR_IS_OK(result)) {
        allocated_bytes += size;
    }
    
    return result;
}

/**
 * Create a string buffer using limited allocation.
 */
NUR_NODISCARD
static NurResult_ptr str_new_limited(size_t initial_capacity) {
    if (initial_capacity == 0) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Initial capacity cannot be zero"));
    }
    
    /* Allocate the StringBuf struct */
    NurResult_ptr buf_result = limited_malloc(sizeof(StringBuf));
    if (NUR_IS_ERR(buf_result)) {
        return buf_result;
    }
    
    StringBuf *buf = (StringBuf *)NUR_VALUE(buf_result);
    
    /* Allocate the data buffer */
    NurResult_ptr data_result = limited_malloc(initial_capacity);
    if (NUR_IS_ERR(data_result)) {
        free(buf); /* Clean up the struct allocation */
        allocated_bytes -= sizeof(StringBuf); /* Adjust counter */
        return data_result;
    }
    
    /* Initialize the StringBuf */
    buf->data = (char *)NUR_VALUE(data_result);
    buf->data[0] = '\0';
    buf->length = 0;
    buf->capacity = initial_capacity;
    
    return NUR_OK(ptr, buf);
}

int main(void) {
    printf("=== nur.h Dynamic Allocation Demonstration ===\n\n");
    
    /* Example 1: Basic string buffer operations */
    printf("1. Basic string buffer operations:\n");
    
    NurResult_ptr buf_result = str_new(16);
    if (NUR_IS_ERR(buf_result)) {
        printf("   Failed to create string buffer: ");
        nur_error_eprint(&NUR_ERROR_OF(buf_result));
        return 1;
    }
    
    StringBuf *buf = (StringBuf *)NUR_VALUE(buf_result);
    printf("   Created string buffer with capacity 16\n");
    
    /* Append some strings */
    const char *parts[] = {"Hello", " ", "world", "!"};
    for (size_t i = 0; i < 4; i++) {
        NurResult_void append_result = str_append(buf, parts[i]);
        if (NUR_IS_ERR(append_result)) {
            printf("   Failed to append '%s': ", parts[i]);
            nur_error_eprint(&append_result.error);
            str_free(buf);
            return 1;
        }
        
        size_t length, capacity;
        str_stats(buf, &length, &capacity);
        printf("   After appending '%s': length=%zu, capacity=%zu, content=\"%s\"\n",
               parts[i], length, capacity, str_content(buf));
    }
    
    /* Example 2: Buffer growth through reallocation */
    printf("\n2. Buffer growth demonstration:\n");
    
    const char *long_string = "This is a much longer string that will definitely exceed the initial buffer capacity and trigger reallocation.";
    NurResult_void long_append = str_append(buf, long_string);
    if (NUR_IS_OK(long_append)) {
        size_t length, capacity;
        str_stats(buf, &length, &capacity);
        printf("   After appending long string: length=%zu, capacity=%zu\n", length, capacity);
        printf("   Content: \"%.50s...\"\n", str_content(buf)); /* Show first 50 chars */
    } else {
        printf("   Error appending long string: ");
        nur_error_eprint(&long_append.error);
    }
    
    str_free(buf);
    
    /* Example 3: String concatenation */
    printf("\n3. String concatenation:\n");
    
    const char *words[] = {"The", " ", "quick", " ", "brown", " ", "fox", " ", "jumps"};
    NurResult_ptr concat_result = str_concat(words, 9);
    if (NUR_IS_OK(concat_result)) {
        StringBuf *concat_buf = (StringBuf *)NUR_VALUE(concat_result);
        size_t length, capacity;
        str_stats(concat_buf, &length, &capacity);
        printf("   Concatenated string: length=%zu, capacity=%zu\n", length, capacity);
        printf("   Content: \"%s\"\n", str_content(concat_buf));
        str_free(concat_buf);
    } else {
        printf("   Error in concatenation: ");
        nur_error_eprint(&NUR_ERROR_OF(concat_result));
    }
    
    /* Example 4: Testing error conditions */
    printf("\n4. Error condition testing:\n");
    
    /* NULL pointer errors */
    NurResult_void null_append = str_append(NULL, "test");
    if (NUR_IS_ERR(null_append)) {
        printf("   NULL buffer error: ");
        nur_error_eprint(&null_append.error);
    }
    
    /* Zero capacity error */
    NurResult_ptr zero_buf = str_new(0);
    if (NUR_IS_ERR(zero_buf)) {
        printf("   Zero capacity error: ");
        nur_error_eprint(&NUR_ERROR_OF(zero_buf));
    }
    
    /* Example 5: Simulating out-of-memory conditions */
    printf("\n5. Out-of-memory simulation:\n");
    
    /* Set a very small allocation limit */
    printf("   Setting allocation limit to 100 bytes\n");
    set_allocation_limit(100);
    
    NurResult_ptr limited_buf = str_new_limited(16);
    if (NUR_IS_OK(limited_buf)) {
        StringBuf *lbuf = (StringBuf *)NUR_VALUE(limited_buf);
        printf("   Successfully created limited buffer\n");
        
        /* Try to allocate more than the limit */
        printf("   Attempting to create another large buffer...\n");
        NurResult_ptr large_buf = str_new_limited(200);
        if (NUR_IS_ERR(large_buf)) {
            printf("   Expected OOM error: ");
            nur_error_eprint(&NUR_ERROR_OF(large_buf));
        }
        
        str_free(lbuf);
    } else {
        printf("   Failed to create limited buffer: ");
        nur_error_eprint(&NUR_ERROR_OF(limited_buf));
    }
    
    /* Example 6: Resource cleanup on complex operations */
    printf("\n6. Resource cleanup demonstration:\n");
    
    /* This will fail during concatenation due to allocation limits */
    const char *many_words[] = {
        "Word1", "Word2", "Word3", "Word4", "Word5",
        "Word6", "Word7", "Word8", "Word9", "Word10"
    };
    
    set_allocation_limit(50); /* Very restrictive */
    NurResult_ptr fail_concat = str_concat(many_words, 10);
    if (NUR_IS_ERR(fail_concat)) {
        printf("   Expected concatenation failure: ");
        nur_error_eprint(&NUR_ERROR_OF(fail_concat));
        printf("   (Resources were properly cleaned up on error)\n");
    }
    
    /* Reset allocation limit */
    set_allocation_limit(SIZE_MAX);
    
    printf("\n=== Dynamic allocation demonstration completed ===\n");
    return 0;
}
