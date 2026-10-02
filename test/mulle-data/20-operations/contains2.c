#include <mulle-data/mulle-data.h>

#include <stdio.h>

static void  mulle_range_print( struct mulle_range a)
{
   printf( "{ %td, %td }", a.location, a.length);
}


int   main( int argc, char *argv[])
{
   int                  flag;
   struct mulle_range   a;
   struct mulle_range   b;

   a    = mulle_range_make( 0, 10);
   b    = mulle_range_make( 5, 0);
   flag = mulle_range_contains( a, b);
   mulle_range_print( a);
   printf( " contains ");
   mulle_range_print( b);
   printf( " = %s\n", flag ? "YES" : "NO");

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
