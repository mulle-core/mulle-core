#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <string.h>


static void   test( char *s, char *p)
{
   size_t  result;

   printf( "mulle_utf8_strcspn( ");
   if( ! s)
      printf( "NULL");
   else
      printf( "\"%s\"",  s);

   if( ! p)
      printf( ", NULL");
   else
      printf( ", \"%s\"", p);

   printf( ") = ");

   result = mulle_utf8_strcspn( s, p);
   printf( "%ld (%ld)\n", (long) result, (long) ((s && p) ? strcspn( s, p) : 0));
}



int   main( int argc, char **argv)
{
   test( NULL, NULL);
   test( "", NULL);
   test( NULL, "");
   test( "", "");
   test( "", "");

   test( NULL, "");

   test( "abcd", "a");
   test( "abcd", "b");
   test( "abcd", "d");

   test( "abcd", "xyz");
   test( "abcd", "xcz");

   test( "abcd", "ab");
   test( "abcd", "bc");

   test( "abcd", "abc");

   test( "abc", "b");
   test( "abc", "abc");

   return( 0);
}