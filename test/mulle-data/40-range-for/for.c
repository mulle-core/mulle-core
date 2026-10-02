#include <mulle-data/mulle-data.h>

#include <stdio.h>


static void   mulle_range_print( struct mulle_range a)
{
   if( ! mulle_range_is_valid( a))
      printf( "INVALID");
   else
      if( ! a.length)
         printf( "NULL");
      else
         printf( "%tu-%tu (%tu)", a.location, a.location + a.length - 1, a.length);
}


static void  test( struct mulle_range a)
{
   char   *sep = " ";

   mulle_range_print( a);
   printf( ": ");

   mulle_range_for( a, i)
   {
      printf( "%s%td", sep, i);
      sep =", ";
   }
   printf( "\n");
}


int   main( int argc, char *argv[])
{
   test( mulle_range_make( 0, 0));
   test( mulle_range_make( 0, 1));
   test( mulle_range_make( 2, 4));
   test( mulle_range_make_invalid());

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
