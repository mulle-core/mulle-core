#include <mulle-slug/mulle-slug.h>

#include <stdio.h>
#include <string.h>


int   main( int argc, char *argv[])
{
   char   buf[ 256];
   char   *result;

   // basic test
   result = mulle_slugify( buf, sizeof( buf), "Hello World!", 12);
   printf( "%s\n", result);

   // with delimiter
   result = mulle_slugify_with_delimiter( buf, sizeof( buf), "Hello World!", 12, '_');
   printf( "%s\n", result);

   // src_len == -1 (NUL-terminated)
   result = mulle_slugify( buf, sizeof( buf), "Take me home!", (size_t) -1);
   printf( "%s\n", result);

   // NULL src
   result = mulle_slugify( buf, sizeof( buf), NULL, 0);
   printf( "%s\n", result);

   // empty src
   result = mulle_slugify( buf, sizeof( buf), "", 0);
   printf( "%s\n", result);

   // special chars
   result = mulle_slugify( buf, sizeof( buf), "$100 < $200", (size_t) -1);
   printf( "%s\n", result);

   // unicode left double quote U+201C must NOT produce literal "
   result = mulle_slugify( buf, sizeof( buf), "\xe2\x80\x9c" "Hello" "\xe2\x80\x9c", (size_t) -1);
   printf( "%s\n", result);

   return( 0);
}
