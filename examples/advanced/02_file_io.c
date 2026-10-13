/*
 * 02_file_io.c - File I/O operations with comprehensive error handling
 * 
 * Compile: gcc -std=c11 -Wall -Wextra -Wpedantic -I../../src 02_file_io.c -o file_io
 * Run: ./file_io
 *
 * Demonstrates:
 * - File operations returning NurResult types
 * - NUR_ERROR_ERRNO_CTX for system error capture
 * - Resource cleanup on error paths
 * - Chained file operations with proper error propagation
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#if defined(__has_include)
#  if __has_include("../../src/nur.h")
#    include "../../src/nur.h"
#  elif __has_include("nur.h")
#    include "nur.h"
#  else
#    include <nur.h>
#  endif
#else
#  include "nur.h"
#endif

/* Platform-specific temp directory */
#ifdef _WIN32
    #define TEMP_DIR "C:\\Windows\\Temp\\"
#else
    #define TEMP_DIR "/tmp/"
#endif

/**
 * Safely open a file, returning NurResult_ptr wrapping FILE*.
 * On error, captures errno and provides context.
 */
NUR_NODISCARD
static NurResult_ptr open_file(const char *filepath, const char *mode) {
    if (filepath == NULL) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_NULL_POINTER, "Filepath cannot be NULL"));
    }
    
    if (mode == NULL) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_NULL_POINTER, "File mode cannot be NULL"));
    }
    
    if (strlen(filepath) == 0) {
        return NUR_ERR(ptr, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Filepath cannot be empty"));
    }
    
    FILE *fp = fopen(filepath, mode);
    if (fp == NULL) {
        return NUR_ERR(ptr, NUR_ERROR_ERRNO_CTX("Failed to open file", filepath));
    }
    
    return NUR_OK(ptr, fp);
}

/**
 * Read one line from file into a fixed buffer.
 * Returns NurResult_usize containing the number of bytes read.
 * Returns NUR_E_EOF when end of file is reached.
 */
NUR_NODISCARD
static NurResult_usize read_line(FILE *fp, char *buffer, size_t buffer_size) {
    if (fp == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "File pointer cannot be NULL"));
    }
    
    if (buffer == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Buffer cannot be NULL"));
    }
    
    if (buffer_size == 0) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_INVALID_ARGUMENT, "Buffer size cannot be zero"));
    }
    
    /* Clear the buffer first */
    buffer[0] = '\0';
    
    /* Try to read a line */
    if (buffer_size > (size_t)INT_MAX) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_OUT_OF_RANGE, "Buffer size too large for fgets"));
    }
    
    if (fgets(buffer, (int)buffer_size, fp) == NULL) {
        if (feof(fp)) {
            return NUR_ERR(usize, NUR_ERROR(NUR_E_EOF, "End of file reached"));
        }
        return NUR_ERR(usize, NUR_ERROR_ERRNO("Failed to read from file"));
    }
    
    /* Calculate actual length read */
    size_t length = strlen(buffer);
    
    /* Remove trailing newline if present */
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        length--;
    }
    
    return NUR_OK(usize, length);
}

/**
 * Write data to a file safely.
 * Returns NurResult_usize containing number of bytes written.
 */
NUR_NODISCARD
static NurResult_usize write_data(FILE *fp, const char *data) {
    if (fp == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "File pointer cannot be NULL"));
    }
    
    if (data == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Data cannot be NULL"));
    }
    
    size_t data_len = strlen(data);
    if (data_len == 0) {
        return NUR_OK(usize, 0); /* Nothing to write is not an error */
    }
    
    size_t written = fwrite(data, 1, data_len, fp);
    if (written != data_len) {
        return NUR_ERR(usize, NUR_ERROR_ERRNO("Failed to write complete data to file"));
    }
    
    /* Write newline */
    if (fputc('\n', fp) == EOF) {
        return NUR_ERR(usize, NUR_ERROR_ERRNO("Failed to write newline to file"));
    }
    
    return NUR_OK(usize, written + 1); /* Include newline in count */
}

/**
 * Create a temporary file with test content.
 * Returns the filename for later use.
 */
NUR_NODISCARD
static NurResult_cstr create_temp_file(const char *content[], size_t line_count) {
    if (content == NULL) {
        return NUR_ERR(cstr, NUR_ERROR(NUR_E_NULL_POINTER, "Content array cannot be NULL"));
    }
    
    /* Create temp filename using time for uniqueness (portable) */
    static char temp_filename[256];
    time_t now = time(NULL);
    if (snprintf(temp_filename, sizeof(temp_filename), TEMP_DIR "nur_test_%ld.txt", (long)now) < 0) {
        return NUR_ERR(cstr, NUR_ERROR(NUR_E_IO, "Failed to create temp filename"));
    }
    
    /* Open file for writing */
    FILE *fp;
    NUR_TRY_TO(cstr, ptr, fp, open_file(temp_filename, "w"));
    
    /* Write all lines */
    for (size_t i = 0; i < line_count; i++) {
        if (content[i] != NULL) {
            size_t written;
            NUR_TRY_TO(cstr, usize, written, write_data(fp, content[i]));
            (void)written; /* Acknowledge we don't use the return value */
        }
    }
    
    /* Close file */
    if (fclose(fp) != 0) {
        return NUR_ERR(cstr, NUR_ERROR_ERRNO_CTX("Failed to close temp file", temp_filename));
    }
    
    return NUR_OK(cstr, temp_filename);
}

/**
 * Process a file by reading all lines and counting them.
 * Demonstrates chained file operations with proper cleanup.
 */
NUR_NODISCARD
static NurResult_usize process_file(const char *filepath) {
    if (filepath == NULL) {
        return NUR_ERR(usize, NUR_ERROR(NUR_E_NULL_POINTER, "Filepath cannot be NULL"));
    }
    
    /* Open the file */
    FILE *fp;
    NUR_TRY_TO(usize, ptr, fp, open_file(filepath, "r"));
    
    size_t line_count = 0;
    char buffer[256];
    
    /* Read lines until EOF */
    while (true) {
        NurResult_usize read_result = read_line(fp, buffer, sizeof(buffer));
        
        if (NUR_IS_OK(read_result)) {
            line_count++;
            printf("   Line %zu: %s\n", line_count, buffer);
        } else {
            nur_error_t error = NUR_ERROR_OF(read_result);
            if (error.code == NUR_E_EOF) {
                /* EOF is expected - break the loop */
                break;
            }
            /* Real error - close file and propagate */
            (void)fclose(fp); /* Clean up before returning error */
            return NUR_ERR(usize, error);
        }
    }
    
    /* Close file */
    if (fclose(fp) != 0) {
        return NUR_ERR(usize, NUR_ERROR_ERRNO_CTX("Failed to close file after processing", filepath));
    }
    
    return NUR_OK(usize, line_count);
}

/**
 * Copy file content from source to destination.
 * Demonstrates resource management with multiple open files.
 */
NUR_NODISCARD
static NurResult_void copy_file(const char *src_path, const char *dst_path) {
    if (src_path == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Source path cannot be NULL"));
    }
    
    if (dst_path == NULL) {
        return NUR_ERR_VOID(NUR_ERROR(NUR_E_NULL_POINTER, "Destination path cannot be NULL"));
    }
    
    /* Open source file */
    FILE *src_fp;
    NUR_TRY_TO(void, ptr, src_fp, open_file(src_path, "r"));
    
    /* Open destination file */
    FILE *dst_fp;
    {
        NurResult_ptr dst_result = open_file(dst_path, "w");
        if (NUR_IS_ERR(dst_result)) {
            (void)fclose(src_fp); /* Clean up source file */
            return NUR_ERR_VOID(NUR_ERROR_OF(dst_result));
        }
        dst_fp = NUR_VALUE(dst_result);
    }
    
    /* Copy line by line */
    char buffer[256];
    size_t lines_copied = 0;
    
    while (true) {
        NurResult_usize read_result = read_line(src_fp, buffer, sizeof(buffer));
        
        if (NUR_IS_OK(read_result)) {
            /* Write the line to destination */
            NurResult_usize write_result = write_data(dst_fp, buffer);
            if (NUR_IS_ERR(write_result)) {
                /* Cleanup both files before returning error */
                (void)fclose(src_fp);
                (void)fclose(dst_fp);
                return NUR_ERR_VOID(NUR_ERROR_OF(write_result));
            }
            lines_copied++;
        } else {
            nur_error_t error = NUR_ERROR_OF(read_result);
            if (error.code == NUR_E_EOF) {
                break; /* Normal end of file */
            }
            /* Error during read - cleanup both files */
            (void)fclose(src_fp);
            (void)fclose(dst_fp);
            return NUR_ERR_VOID(error);
        }
    }
    
    /* Close both files */
    if (fclose(src_fp) != 0) {
        (void)fclose(dst_fp); /* Still try to close destination */
        return NUR_ERR_VOID(NUR_ERROR_ERRNO_CTX("Failed to close source file", src_path));
    }
    
    if (fclose(dst_fp) != 0) {
        return NUR_ERR_VOID(NUR_ERROR_ERRNO_CTX("Failed to close destination file", dst_path));
    }
    
    printf("   Successfully copied %zu lines from %s to %s\n", 
           lines_copied, src_path, dst_path);
    
    return NUR_OK_VOID();
}

/**
 * Clean up temporary files (utility function).
 */
static void cleanup_temp_file(const char *filepath) {
    if (filepath != NULL) {
        (void)remove(filepath); /* Ignore errors in cleanup */
    }
}

int main(void) {
    printf("=== nur.h File I/O Demonstration ===\n\n");
    
    /* Example 1: Create a temporary file */
    printf("1. Creating temporary file with test content:\n");
    
    const char *test_content[] = {
        "Line one of test content",
        "Line two with some numbers: 12345", 
        "Line three with special chars: !@#$%",
        "Final line of the test file",
    };
    
    NurResult_cstr temp_file = create_temp_file(test_content, 4);
    if (NUR_IS_ERR(temp_file)) {
        printf("   Failed to create temp file: ");
        nur_error_eprint(&NUR_ERROR_OF(temp_file));
        return 1;
    }
    
    char *temp_filename = NUR_VALUE(temp_file);
    printf("   Created temp file: %s\n", temp_filename);
    
    /* Example 2: Process the file (read and count lines) */
    printf("\n2. Processing file (reading all lines):\n");
    
    NurResult_usize line_count = process_file(temp_filename);
    if (NUR_IS_OK(line_count)) {
        printf("   Total lines processed: %zu\n", NUR_VALUE(line_count));
    } else {
        printf("   Error processing file: ");
        nur_error_eprint(&NUR_ERROR_OF(line_count));
    }
    
    /* Example 3: Copy the file to another location */
    printf("\n3. Copying file to new location:\n");
    
    char copy_filename[256];
    time_t copy_time = time(NULL);
    if (snprintf(copy_filename, sizeof(copy_filename), TEMP_DIR "nur_copy_%ld.txt", (long)copy_time) < 0) {
        printf("   Failed to format copy filename\n");
        return 1;
    }
    
    NurResult_void copy_result = copy_file(temp_filename, copy_filename);
    if (NUR_IS_ERR(copy_result)) {
        printf("   Error copying file: ");
        nur_error_eprint(&copy_result.error);
    }
    
    /* Example 4: Try to open non-existent file */
    printf("\n4. Attempting to open non-existent file:\n");
    
    NurResult_ptr nonexistent = open_file("definitely_does_not_exist.txt", "r");
    if (NUR_IS_ERR(nonexistent)) {
        printf("   Expected error: ");
        nur_error_eprint(&NUR_ERROR_OF(nonexistent));
    }
    
    /* Example 5: Try invalid operations */
    printf("\n5. Testing error conditions:\n");
    
    /* NULL file pointer */
    char buffer[100];
    NurResult_usize null_read = read_line(NULL, buffer, sizeof(buffer));
    if (NUR_IS_ERR(null_read)) {
        printf("   NULL file pointer error: ");
        nur_error_eprint(&NUR_ERROR_OF(null_read));
    }
    
    /* Zero buffer size */
    FILE *dummy_fp = (FILE*)0x1; /* Fake pointer for demo */
    NurResult_usize zero_size = read_line(dummy_fp, buffer, 0);
    if (NUR_IS_ERR(zero_size)) {
        printf("   Zero buffer size error: ");
        nur_error_eprint(&NUR_ERROR_OF(zero_size));
    }
    
    /* Example 6: Cleanup */
    printf("\n6. Cleaning up temporary files:\n");
    cleanup_temp_file(temp_filename);
    cleanup_temp_file(copy_filename);
    printf("   Temporary files cleaned up\n");
    
    printf("\n=== File I/O demonstration completed ===\n");
    return 0;
}
