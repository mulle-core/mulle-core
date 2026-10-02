#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static void   test( mulle_utf32_t *s, mulle_utf32_t *p)
{
   size_t  result;
   char    dst[ 128];
   char    *end;

   printf( "mulle_utf32_strspn( ");
   if( ! s)
      printf( "NULL");
   else
   {
      end = _mulle_utf32_convert_to_utf8( s, mulle_utf32_strlen( s), dst);

      printf( "\"%.*s\"", (int) (end - dst), dst);
   }

   if( ! p)
      printf( ", NULL");
   else
   {
      end = _mulle_utf32_convert_to_utf8( p, mulle_utf32_strlen( p), dst);
      printf( ", \"%.*s\"", (int) (end - dst), dst);
   }

   printf( ") = ");

   result = mulle_utf32_strspn( s, p);
   printf( "%ld\n", (long) result);
}



int   main( int argc, char **argv)
{
   mulle_utf32_t  abcd[ 5] = { 'a', 'b', 'c', 'd', 0 };
   mulle_utf32_t  abc[ 4] = { 'a', 'b', 'c', 0 };
   mulle_utf32_t  xyz[ 4] = { 'x', 'y', 'z', 0 };
   mulle_utf32_t  xcz[ 4] = { 'x', 'c', 'z', 0 };
   mulle_utf32_t  ab[ 4] = { 'a', 'b', 0 };
   mulle_utf32_t  bc[ 4] = { 'b', 'c', 0 };
   mulle_utf32_t  a[ 2] = { 'a', 0 };
   mulle_utf32_t  b[ 2] = { 'b', 0 };
   mulle_utf32_t  d[ 2] = { 'b', 0 };
   mulle_utf32_t  empty[ 1] = { 0 };

   test( NULL, NULL);
   test( empty, NULL);
   test( NULL, empty);
   test( empty, empty);
   test( empty, empty);

   test( NULL, empty);

   test( abcd, a);
   test( abcd, b);
   test( abcd, d);
   test( abcd, d);

   test( abcd, xyz);
   test( abcd, xcz);

   test( abcd, ab);
   test( abcd, bc);

   test( abcd, abc);

   test( abc, b);
   test( abc, abc);

   return( 0);
}