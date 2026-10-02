#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


int   main( void)
{
   void   *buffer;
   char   buf[] = "Hello World";
   int    c;

   // Test "r+" mode: should be readable AND writable
   buffer = mulle_buffer_fmemopen( buf, sizeof( buf) - 1, "r+");
   if( ! buffer)
   {
      printf( "FAIL: could not open buffer in r+ mode\n");
      return( 1);
   }

   // should be able to read
   c = mulle_buffer_fgetc( buffer);
   if( c == EOF)
   {
      printf( "FAIL: r+ mode cannot read (got EOF)\n");
      mulle_buffer_fclose( buffer);
      return( 1);
   }
   printf( "read: '%c'\n", c);

   // seek back to start
   mulle_buffer_fseek( buffer, 0, SEEK_SET);

   // should be able to write
   errno = 0;
   c = mulle_buffer_fputc( 'X', buffer);
   if( c == EOF)
   {
      printf( "FAIL: r+ mode cannot write (got EOF, errno=%d)\n", errno);
      mulle_buffer_fclose( buffer);
      return( 1);
   }
   printf( "wrote: '%c'\n", (char) c);

   // verify the write took effect
   mulle_buffer_fseek( buffer, 0, SEEK_SET);
   c = mulle_buffer_fgetc( buffer);
   printf( "verify: '%c'\n", c);

   mulle_buffer_fclose( buffer);

   printf( "buf: \"%s\"\n", buf);

   return( 0);
}
