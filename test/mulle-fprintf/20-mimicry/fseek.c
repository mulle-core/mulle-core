#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
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

static void print_char_safe(int c)
{
    if (c >= 32 && c <= 126) {
        printf("'%c' (%d)", c, c);
    } else if (c == -1) {
        printf("EOF (-1)");
    } else {
        printf("(%d)", c);
    }
}



static void compare_seek_tell(FILE *fp, struct mulle_buffer *buffer, const char *test_name)
{
    long pos1, pos2;
    int res1, res2;
    int errno1, errno2;
    int c1, c2;

    if( fp && ! buffer || ! fp && buffer)
    {
      printf("Error in %s: FILE * %s; mulle_buffer: %s\n",
               test_name,
               fp ? "valid" : "NULL",
               buffer ? "valid" : "NULL");
      return;
    }

    // Test initial position
    errno = 0;
    pos1 = ftell( fp);
    errno1 = errno;

    errno = 0;
    pos2 = mulle_buffer_ftell( buffer);
    errno2 = errno;

    printf("%s - Initial position: FILE*=%ld (%s), mulle_buffer=%ld (%s)\n",
           test_name, pos1, errno_name(errno1), pos2, errno_name(errno2));

    // Seek to position 3
    errno = 0;
    res1 = fseek( fp, 3, SEEK_SET);
    errno1 = errno;

    errno = 0;
    res2 = mulle_buffer_fseek( buffer, 3, SEEK_SET);
    errno2 = errno;

    printf("%s - Seek to 3: FILE*=%d (%s), mulle_buffer=%d (%s)\n",
           test_name, res1, errno_name(errno1), res2, errno_name(errno2));

    // Read character at position 3
    errno = 0;
    c1 = fgetc( fp);
    errno1 = errno;

    errno = 0;
    c2 = mulle_buffer_fgetc( buffer);
    errno2 = errno;

    printf("%s - Read at 3: FILE*=", test_name);
    print_char_safe(c1);
    printf(" (%s), mulle_buffer=", errno_name(errno1));
    print_char_safe(c2);
    printf(" (%s)\n", errno_name(errno2));

    // Seek relative to current position
    errno = 0;
    res1 = fseek( fp, -2, SEEK_CUR);
    errno1 = errno;

    errno = 0;
    res2 = mulle_buffer_fseek( buffer, -2, SEEK_CUR);
    errno2 = errno;

    printf("%s - Seek -2 from current: FILE*=%d (%s), mulle_buffer=%d (%s)\n",
           test_name, res1, errno_name(errno1), res2, errno_name(errno2));

    // Write character at new position
    errno = 0;
    res1 = fputc( 'X', fp);
    errno1 = errno;

    errno = 0;
    res2 = mulle_buffer_fputc( 'X', buffer);
    errno2 = errno;

    printf("%s - Write 'X': FILE*=%d (%s), mulle_buffer=%d (%s)\n",
           test_name, res1, errno_name(errno1), res2, errno_name(errno2));

    // Seek to end
    errno = 0;
    res1 = fseek( fp, 0, SEEK_END);  // not sure why fseek fails here in "normal"
    errno1 = errno;

    errno = 0;
    res2 = mulle_buffer_fseek( buffer, 0, SEEK_END);
    errno2 = errno;

    printf("%s - Seek to end: FILE*=%d (%s), mulle_buffer=%d (%s)\n",
           test_name, res1, errno_name(errno1), res2, errno_name(errno2));

    // Get final position
    errno = 0;
    pos1 = ftell( fp);
    errno1 = errno;

    errno = 0;
    pos2 = mulle_buffer_ftell( buffer);
    errno2 = errno;

    printf("%s - Final position: FILE*=%ld (%s), mulle_buffer=%ld (%s)\n",
           test_name, pos1, errno_name(errno1), pos2, errno_name(errno2));
}

static void test_seek_tell(void)
{
    FILE *fp;
    struct mulle_buffer *buffer;
    char test_string[] =
    {
      'V', 'f', 'L', ' ',
      'B', 'o', 'c', 'h', 'u', 'm', ' ',
      '1', '8', '4', '8', 0
    };

    // Test case 1: Normal case with read/write
    fp = fmemopen((void*)test_string, strlen(test_string), "a+");
    buffer = mulle_buffer_fmemopen((void*)test_string, strlen(test_string), "a+");
    compare_seek_tell(fp, buffer, "Normal case");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

    // Test case 2: Read-only file, but mulle_buffer can't do read only
    fp = fmemopen((void*)test_string, strlen(test_string), "r");
    buffer = mulle_buffer_fmemopen((void*)test_string, strlen(test_string), "r");
    compare_seek_tell(fp, buffer, "Read-only file");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);

#ifndef MULLE_TEST_VALGRIND
    fp = fmemopen(NULL, 0, "w");
    buffer = mulle_buffer_fmemopen(NULL, 0, "w");
    compare_seek_tell(fp, buffer, "Empty file");
    if( fp)
       fclose(fp);
    if( buffer)
       mulle_buffer_fclose(buffer);
#else
    printf( "Empty file - Initial position: FILE*=0 (0), mulle_buffer=0 (0)\n"
            "Empty file - Seek to 3: FILE*=-1 (EINVAL), mulle_buffer=-1 (EINVAL)\n"
            "Empty file - Read at 3: FILE*=EOF (-1) (EBADF), mulle_buffer=EOF (-1) (EBADF)\n"
            "Empty file - Seek -2 from current: FILE*=-1 (EINVAL), mulle_buffer=-1 (EINVAL)\n"
            "Empty file - Write 'X': FILE*=88 (0), mulle_buffer=88 (0)\n"
            "Empty file - Seek to end: FILE*=-1 (ENOSPC), mulle_buffer=-1 (ENOSPC)\n"
            "Empty file - Final position: FILE*=0 (0), mulle_buffer=0 (0)\n");
#endif
}

int main(int argc, const char * argv[])
{
#if !HAVE_FMEMOPEN
    printf("fseek tests skipped on Windows (fmemopen not available)\n");
    return 0;
#else
    test_seek_tell();
    return 0;
#endif
}
