#include <mulle-data/mulle-data.h>

#include <stdio.h>
#include <string.h>


static void  print( char *s)
{
   if( ! s)
      printf( "NULL");
   else
      printf( "\"%s\"", s);
}


static void  test( char *a, char *b)
{
   struct mulle_data    a_data;
   struct mulle_data    b_data;
   char                 *c;

   a_data = mulle_data_make( a, a ? strlen( a) : 0);
   b_data = mulle_data_make( b, b ? strlen( b) : 0);
   c      = mulle_data_search_data( a_data, b_data);

   print( b);
   printf( " in ");
   print( a);
   printf( " -> ");
   print( c);
   printf( "\n");
}


int   main( int argc, char *argv[])
{
   test( NULL, NULL);
   test( NULL, "");
   test( "", NULL);

   test( "a", "a");
   test( "a", "b");
   test( "b", "a");
   test( "b", "b");

   test( "aa", "a");
   test( "aa", "b");
   test( "a",  "aa");
   test( "b",  "aa");

   test( "abc", "a");
   test( "abc", "b");
   test( "abc", "c");
   test( "abc", "ab");
   test( "abc", "bc");
   test( "abc", "ac");

   return( 0);
}
