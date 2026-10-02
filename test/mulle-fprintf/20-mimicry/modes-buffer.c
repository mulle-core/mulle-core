#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <mulle-fprintf/mulle-fprintf.h>

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

static void   test_buffer( struct mulle_buffer *fp)
{
   int   res1;
   long  off;

   off   = mulle_buffer_ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = mulle_buffer_fseek( fp, -2, SEEK_CUR);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = mulle_buffer_ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = mulle_buffer_fputc( 'X', fp);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = mulle_buffer_ftell( fp);
   printf( "off %ld\n", off);

   errno = 0;
   res1  = mulle_buffer_fseek( fp, 0, SEEK_END);
   printf( "res1 %d: %s\n", res1, errno_name( errno));

   off   = mulle_buffer_ftell( fp);
   printf( "off %ld\n", off);
}


static void   test( char *mode)
{
   struct mulle_buffer   *fp;
   char   test_string[] = "VfL Bochum 1848";

   printf( "---------------- %s ----------------\n", mode);
   fp = mulle_buffer_fmemopen( (void *) test_string, strlen( test_string) - 4, mode);
   if( ! fp)
      abort();

   test_buffer( fp);

   mulle_buffer_fclose( fp);

   printf( "s: \"%s\"\n", test_string);
}


int   main( void)
{
   test( "w");
   test( "a");
   test( "r");
   test( "w+");
   test( "a+");
   test( "r+");

   return( 0);
}
