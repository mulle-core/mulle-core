#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static void   test( mulle_utf16_t *s, mulle_utf16_t *p)
{
   size_t  result;
   char    dst[ 128];
   char    *end;

   printf( "mulle_utf16_strspn( ");
   if( ! s)
      printf( "NULL");
   else
   {
      end = _mulle_utf16_convert_to_utf8( s, mulle_utf16_strlen( s), dst);

      printf( "\"%.*s\"", (int) (end - dst), dst);
   }

   if( ! p)
      printf( ", NULL");
   else
   {
      end = _mulle_utf16_convert_to_utf8( p, mulle_utf16_strlen( p), dst);
      printf( ", \"%.*s\"", (int) (end - dst), dst);
   }

   printf( ") = ");

   result = mulle_utf16_strspn( s, p);
   printf( "%ld\n", (long) result);
}



int   main( int argc, char **argv)
{
   mulle_utf16_t  abcd[ 5] = { 'a', 'b', 'c', 'd', 0 };
   mulle_utf16_t  abc[ 4] = { 'a', 'b', 'c', 0 };
   mulle_utf16_t  xyz[ 4] = { 'x', 'y', 'z', 0 };
   mulle_utf16_t  xcz[ 4] = { 'x', 'c', 'z', 0 };
   mulle_utf16_t  ab[ 4] = { 'a', 'b', 0 };
   mulle_utf16_t  bc[ 4] = { 'b', 'c', 0 };
   mulle_utf16_t  a[ 2] = { 'a', 0 };
   mulle_utf16_t  b[ 2] = { 'b', 0 };
   mulle_utf16_t  d[ 2] = { 'b', 0 };
   mulle_utf16_t  empty[ 1] = { 0 };

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