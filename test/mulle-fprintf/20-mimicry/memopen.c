#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "fmemopen-compat.h"

// Assuming these are defined elsewhere
extern void *mulle_buffer_fmemopen(void *buf, size_t size, const char *mode);
extern int mulle_buffer_fclose(void *buffer);

int main(void)
{
#if !HAVE_FMEMOPEN
    printf("memopen tests skipped on Windows (fmemopen not available)\n");
    return 0;
#else
    void *buffer;
    char buf[100];
    FILE  *fp;

    // Test 1: Open with NULL buffer and "r" mode
    //         doesn't fail anymore
    buffer = mulle_buffer_fmemopen(NULL, 100, "r");
    if (! buffer) {
        fprintf( stderr, "Test 1 failed: Opened NULL buffer in read mode\n");
        if( buffer)
           mulle_buffer_fclose(buffer);
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test 2: Open with existing buffer and "r" mode
    strcpy(buf, "Hello, World!");
    buffer = mulle_buffer_fmemopen(buf, strlen(buf), "r");
    if (!buffer) {
        fprintf( stderr, "Test 2 failed: Could not open existing buffer in read mode\n");
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test 3: Open with existing buffer and "wü" mode (should clear buffer[ 0])
    buffer = mulle_buffer_fmemopen(buf, sizeof(buf), "w+");
    if (!buffer) {
        fprintf( stderr, "Test 3 failed: Could not open existing buffer in write mode\n");
        return 1;
    }
    if (buf[0] != '\0') {
        fprintf( stderr, "Test 3 failed: Buffer not cleared in write mode\n");
        if( buffer)
           mulle_buffer_fclose(buffer);
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test 4: Open with existing buffer and "a" mode
    strcpy(buf, "Hello");
    buffer = mulle_buffer_fmemopen(buf, sizeof(buf), "a");
    if (!buffer) {
        fprintf( stderr, "Test 4 failed: Could not open existing buffer in append mode\n");
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test 5: Open with zero size
#ifndef MULLE_TEST_VALGRIND
    fp     = fmemopen( NULL, 0, "w");
    buffer = mulle_buffer_fmemopen(NULL, 0, "w");
    if (buffer && ! fp) {
        fprintf( stderr, "Test 5 failed: Opened buffer with zero size\n");
        if( buffer)
           mulle_buffer_fclose(buffer);
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);
    if( fp)
       fclose(fp);
#endif
    // Test 6: Open with invalid mode
    buffer = mulle_buffer_fmemopen(buf, sizeof(buf), "x");
    if (buffer) {
        fprintf( stderr, "Test 6 failed: Opened buffer with invalid mode\n");
        if( buffer)
           mulle_buffer_fclose(buffer);
        return 1;
    }

    // Test 7: Open with NULL buffer and "w" mode
    buffer = mulle_buffer_fmemopen(NULL, 100, "w");
    if (!buffer) {
        fprintf( stderr, "Test 7 failed: Could not open NULL buffer in write mode\n");
        return 1;
    }
    if( buffer)
       mulle_buffer_fclose(buffer);

    return 0;
#endif
}
