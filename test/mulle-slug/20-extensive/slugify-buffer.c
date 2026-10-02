#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{

   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_slugified_utf8data( buffer, (struct mulle_utf8data){  0 });
      mulle_buffer_add_slugified_utf8data( buffer, mulle_utf8data_make( NULL, 0));
      mulle_buffer_add_slugified_utf8data( buffer, mulle_utf8data_make( "a\ab\0c\n", 4));
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }

   return( 0);
}
