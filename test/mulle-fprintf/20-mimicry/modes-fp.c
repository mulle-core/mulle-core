#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "fmemopen-compat.h"

static const char* errno_name(int err)
{
    switch(err)
    {
    case 0: return "Success";
    case EBADF: return "EBADF";
    case EINVAL: return "EINVAL";
    case ENOSPC: return "ENOSPC";
    case ENOMEM: return "ENOMEM";
    case EACCES: return "EACCES";
    case EPERM: return "EPERM";
    default: return "UNKNOWN";
    }
}


static void   test_file( FILE *fp)
{
   int   res1;
   long  off;

   off   = ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = fseek( fp, -2, SEEK_CUR);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = fputc( 'X', fp);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = fseek( fp, 0, SEEK_END);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = ftell( fp);
   printf( "off %ld\n", off);
}


static void   test( char *mode)
{
   FILE   *fp;
   char   test_string[] = "VfL Bochum 1848";

   printf( "---------------- %s ----------------\n", mode);
   fp = fmemopen( (void *) test_string, strlen( test_string) - 4, mode);
   if( ! fp)
      abort();

   test_file( fp);

   printf( "s: \"%s\"\n", test_string);
   if( fp)
      fclose(fp);
}


int   main( void)
{
#if !HAVE_FMEMOPEN
   printf("modes-fp tests skipped on Windows (fmemopen not available)\n");
   return 0;
#else
   test( "r");
   test( "w");
   test( "a");
   test( "r+");
   test( "w+");
   test( "a+");

   return( 0);
#endif
}
