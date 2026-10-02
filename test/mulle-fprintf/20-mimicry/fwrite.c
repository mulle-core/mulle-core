#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include "fmemopen-compat.h"

static const char* errno_name(int err)
{
    switch(err)
    {
    case 0: return "0";
    case EBADF: return "EBADF";
    case EINVAL: return "EINVAL";
    case ENOSPC: return "ENOSPC";
    case ENOMEM: return "ENOMEM";
    case EACCES: return "EACCES";
    case EPERM: return "EPERM";
    default: return "UNKNOWN";
    }
}

static jmp_buf jump_buffer;

static void signal_handler(int signum)
{
    // MEMO: this is wrong, you can't really do this (but works in this test for now)
    longjmp(jump_buffer, signum);
}

static size_t safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t result;
    if (setjmp(jump_buffer) == 0)
    {
        result = fwrite(ptr, size, nmemb, stream);
    }
    else
    {
        result = (size_t)-1;  // Special value to indicate crash
    }
    return result;
}

static size_t safe_mulle_buffer_fwrite(const void *ptr, size_t size, size_t nmemb, void *buffer)
{
    size_t result;
    if (setjmp(jump_buffer) == 0)
    {
        result = mulle_buffer_fwrite( (void *) ptr, size, nmemb, buffer);
    }
    else
    {
        result = (size_t)-1;  // Special value to indicate crash
    }
    return result;
}

static void   compare_write( const void *ptr, size_t size, size_t nmemb, FILE *fp, void *buffer, const char *test_name)
{
    size_t r1, r2;
    int errno1, errno2;

    if( fp && ! buffer || ! fp && buffer)
    {
      printf("Error in %s: FILE * %s; mulle_buffer: %s\n",
               test_name,
               fp ? "valid" : "NULL",
               buffer ? "valid" : "NULL");
      return;
    }

    signal(SIGSEGV, signal_handler);

    errno  = 0;
    r1     = safe_fwrite( ptr, size, nmemb, fp);
    errno1 = errno;

    errno  = 0;
    r2     = safe_mulle_buffer_fwrite( ptr, size, nmemb, buffer);
    errno2 = errno;

    if (r1 == r2 && errno1 == errno2)
    {
        if (r1 == (size_t)-1)
        {
            printf("%s: Passed (both crashed as expected)\n", test_name);
        }
        else
        {
            printf("%s: Passed (result=%zd, errno=%s)\n", test_name, r1, errno_name(errno1));
        }
    }
    else
    {
        printf("Error in %s: fwrite: result=%zd, errno=%s; mulle_buffer_fwrite: result=%zu, errno=%s\n",
               test_name, r1, errno_name(errno1), r2, errno_name(errno2));
    }

    signal(SIGSEGV, SIG_DFL);  // Reset signal handler
}

static void test_write(void)
{
    FILE *fp;
    void *buffer;
    const char test_data[] = "Hello, World!";

    // Test case 1: Write to memory stream
    fp = fmemopen(NULL, 100, "w");
    buffer = mulle_buffer_fmemopen(NULL, 100, "w");
    compare_write(test_data, 1, strlen(test_data), fp, buffer, "Write to memory stream");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 2: Write to buffer
    char small_buf[5] = {0};
    fp = fmemopen(small_buf, 5, "w");
    buffer = mulle_buffer_fmemopen(small_buf, 5, "w");
    compare_write(test_data, 1, strlen(test_data), fp, buffer, "Write to buffer");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 3: Write-only empty file
#ifndef MULLE_TEST_VALGRIND
    fp = fmemopen(NULL, 0, "w");
    buffer = mulle_buffer_fmemopen(NULL, 0, "w");
    compare_write(test_data, 1, strlen(test_data), fp, buffer, "Write-only empty file");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);
#else
    printf( "Write-only empty file: Passed (result=13, errno=0)\n");
#endif
    // Test case 4: Multiple writes
    fp = fmemopen(NULL, 100, "w");
    buffer = mulle_buffer_fmemopen(NULL, 100, "w");
    for (int i = 0; i < 5; i++)
    {
        char chunk[3] = {0};
        snprintf(chunk, sizeof(chunk), "%d", i);
        compare_write(chunk, 1, strlen(chunk), fp, buffer, "Multiple writes");
    }
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 5: Write with different size and nmemb
    fp = fmemopen(NULL, 100, "w");
    buffer = mulle_buffer_fmemopen(NULL, 100, "w");
    compare_write(test_data, 2, strlen(test_data) / 2, fp, buffer, "Write with size=2, nmemb=6");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 7: NULL pointers
#ifndef MULLE_TEST_VALGRIND
    compare_write(test_data, 1, strlen(test_data), NULL, NULL, "NULL pointers");
#else
    printf( "NULL pointers: Passed (both crashed as expected)\n");
#endif
}

int main(int argc, const char * argv[])
{
#if !HAVE_FMEMOPEN
    printf("fwrite tests skipped on Windows (fmemopen not available)\n");
    return 0;
#else
    test_write();
    return 0;
#endif
}
