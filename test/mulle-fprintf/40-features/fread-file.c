#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>
#include <string.h>


int   main( void)
{
   struct mulle_buffer   buffer;
   FILE                  *fp;
   char                  *s;
   size_t                rval;

   // fixture: testdir/hello.bin ("Hello, buffer file world", 24 bytes)
   fp = fopen( "testdir/hello.bin", "rb");
   if( ! fp)
      return( 1);

   // mulle_buffer_fread_FILE: read a fixed number of bytes
   mulle_buffer_init( &buffer, 0, NULL);
   rval = mulle_buffer_fread_FILE( &buffer, 1, 5, fp);
   s    = mulle_buffer_extract_string( &buffer);
   printf( "fread_FILE: %zu \"%s\"\n", rval, s);
   mulle_free( s);
   mulle_buffer_done( &buffer);

   // mulle_buffer_fread_FILE_all: read the remaining bytes
   rewind( fp);
   mulle_buffer_init( &buffer, 0, NULL);
   rval = mulle_buffer_fread_FILE_all( &buffer, fp);
   s    = mulle_buffer_extract_string( &buffer);
   printf( "fread_FILE_all: %zu \"%s\"\n", rval, s);
   mulle_free( s);
   mulle_buffer_done( &buffer);

   fclose( fp);

   // NULL fp guards (E6)
   mulle_buffer_init( &buffer, 0, NULL);
   rval = mulle_buffer_fread_FILE( &buffer, 1, 5, NULL);
   printf( "fread_FILE_null_fp: %zu errno=%s\n", rval, errno == EINVAL ? "EINVAL" : "?");
   mulle_buffer_done( &buffer);

   mulle_buffer_init( &buffer, 0, NULL);
   rval = mulle_buffer_fread_FILE_all( &buffer, NULL);
   printf( "fread_FILE_all_null_fp: %zu errno=%s\n", rval, errno == EINVAL ? "EINVAL" : "?");
   mulle_buffer_done( &buffer);

   // mulle_buffer_do_filepath
   mulle_buffer_do_filepath( name, "testdir/hello.bin", MULLE_BUFFER_IS_TEXT)
   {
      s = mulle_buffer_extract_string( name);
      printf( "do_filepath: \"%s\"\n", s);
      mulle_free( s);
   }

   return( 0);
}
