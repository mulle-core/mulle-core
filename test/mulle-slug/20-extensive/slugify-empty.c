#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   char   *slug;

   // coverage
   if( mulle_utf8_slugify( NULL))
      return( 1);

   // coverage
   slug = mulle_utf8_slugify( "");
   if( strcmp( slug, ""))
      return( 1);
   mulle_free( slug);

   return( 0);
}
