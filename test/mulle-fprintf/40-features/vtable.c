#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>
#include <string.h>

// Exercises dispatch through both vtables (mulle_buffer_functions and
// mulle_stdio_functions), the "write once, run anywhere" pattern.
int   main( void)
{
   struct mulle_buffer_stdio_functions *functions;
   void                                *fp;
   char                                dst[ 32];
   long                                n;

   // buffer side
   fp        = mulle_buffer_alloc( NULL);
   mulle_buffer_init( fp, 0, NULL);
   functions = &mulle_buffer_functions;
   functions->fputs( "Hello ", fp);
   functions->fputc( 'X', fp);
   functions->fputs( "!", fp);
   n = functions->ftell( fp);
   printf( "buffer_vtable_ftell: %ld\n", n);

   functions->fseek( fp, 0, SEEK_SET);
   memset( dst, 0, sizeof( dst));
   n = (long) functions->fread( dst, 1, (size_t) n, fp);
   printf( "buffer_vtable_read: %ld \"%s\"\n", n, dst);
   functions->fclose( fp);

   // FILE side
   fp        = tmpfile();
   functions = &mulle_stdio_functions;
   functions->fputs( "via-file", fp);
   n = functions->ftell( fp);
   printf( "file_vtable_ftell: %ld\n", n);

   functions->fseek( fp, 0, SEEK_SET);
   memset( dst, 0, sizeof( dst));
   functions->fread( dst, 1, (size_t) n, fp);
   printf( "file_vtable_read: \"%s\"\n", dst);
   functions->fclose( fp);

   return( 0);
}
