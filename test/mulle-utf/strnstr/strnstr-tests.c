#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static void   test( char *s, size_t len, char *p)
{
   char  *result;

   printf( "mulle_utf8_strnstr( ");
   if( ! s)
      printf( "NULL");
   else
      printf( "\"%.*s\"", (int) len, s);

   printf( ", %d", (int) len);

   if( ! p)
      printf( ", NULL");
   else
      printf( ", \"%s\"", p);

   printf( ") = ");

   result = mulle_utf8_strnstr( s, len, p);
   if( ! result)
      printf( "NULL\n");
   else
      printf( "\"%s\" (%ld)\n", p, (long) (result - s));
}



int   main( int argc, char **argv)
{
   test( NULL, 0, NULL);
   test( "", 0, NULL);
   test( NULL, 0, "");
   test( "", 0, "");
   test( "", 1, "");

   test( NULL, 2, "");

   test( "abcd", -1, "a");
   test( "abcd", 4, "b");
   test( "abcd", 4, "c");
   test( "abcd", 4, "d");

   test( "abcd", 4, "ab");
   test( "abcd", 4, "bc");
   test( "abcd", 4, "cd");

   test( "abcd", 4, "abc");

   test( "abc", 3, "b");
   test( "abc", 3, "abc");

   return( 0);
}