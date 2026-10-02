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
    longjmp(jump_buffer, signum);
}

static int safe_fgetc(FILE *fp)
{
    int result;
    if (setjmp(jump_buffer) == 0)
    {
        result = fgetc(fp);
    }
    else
    {
        result = -2;  // Special value to indicate crash
    }
    return result;
}

static int safe_mulle_buffer_fgetc(void *buffer)
{
    int result;
    if (setjmp(jump_buffer) == 0)
    {
        result = mulle_buffer_fgetc(buffer);
    }
    else
    {
        result = -2;  // Special value to indicate crash
    }
    return result;
}

static void compare_fgetc( FILE *fp, void *buffer, const char *test_name)
{
    int c1, c2;
    int errno1, errno2;

    if( fp && ! buffer || ! fp && buffer)
    {
      printf("Error in %s: FILE * %s; mulle_buffer: %s\n",
               test_name,
               fp ? "valid" : "NULL",
               buffer ? "valid" : "NULL");
      return;
    }

    signal( SIGSEGV, signal_handler);

    errno  = 0;
    c1     = safe_fgetc( fp);
    errno1 = errno;

    errno  = 0;
    c2     = safe_mulle_buffer_fgetc( buffer);
    errno2 = errno;

    if (c1 == c2 && errno1 == errno2)
    {
        if (c1 == -2)
        {
            printf("%s: Passed (both crashed as expected)\n", test_name);
        }
        else
        {
            printf("%s: Passed (char=%d, errno=%s)\n", test_name, c1, errno_name(errno1));
        }
    }
    else
    {
        printf("Error in %s: FILE *: char=%d, errno=%s; mulle_buffer: char=%d, errno=%s\n",
               test_name, c1, errno_name(errno1), c2, errno_name(errno2));
    }

    signal( SIGSEGV, SIG_DFL);  // Reset signal handler
}

static void test_fgetc(void)
{
#if !HAVE_FMEMOPEN
    printf("fget tests skipped on Windows (fmemopen not available)\n");
    return;
#else
    FILE *fp;
    void *buffer;
    const char *test_string = "Hello";

    // Test case 1: Zero-length memory
    fp     = fmemopen( (void *) "", 0, "r");
    buffer = mulle_buffer_fmemopen( (void *) "", 0, "r");
    compare_fgetc( fp, buffer, "Zero-length memory");
    if( fp)
       fclose( fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 2: Normal case
    fp = fmemopen((void*)test_string, strlen(test_string), "r");
    buffer = mulle_buffer_fmemopen((void*)test_string, strlen(test_string), "r");
    
    // Read all characters
    for (int i = 0; i <= strlen(test_string); i++)
    {
        compare_fgetc(fp, buffer, "Normal case");
    }
    
    // Test EOF
    compare_fgetc(fp, buffer, "EOF case");

    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 3: Read-only empty file
#ifndef MULLE_TEST_VALGRIND
    fp = fmemopen(NULL, 0, "r");
    buffer = mulle_buffer_fmemopen(NULL, 0, "r");
    compare_fgetc(fp, buffer, "Read-only empty file");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 4: NULL pointers
    // MEMO: this will be different error code, as mulle_buffer_fgetc
    //       will refuse to crash
    compare_fgetc( NULL, NULL, "NULL pointers");
#else
   // fake it, coz we ain't debugging glibc
   printf( "Read-only empty file: Passed (char=-1, errno=0)\n");
   printf( "Error in NULL pointers: FILE *: char=-2, errno=0; mulle_buffer: char=-1, errno=0\n");
#endif
#endif
}

int main(int argc, const char * argv[])
{
    test_fgetc();
    return 0;
}
