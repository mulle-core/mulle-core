#include <mulle-data/mulle-data.h>

#include <stdio.h>


struct mulle_range    range1[] =
{
  { 1, 1 },
  { 3, 1 },
  { 5, 1 }
};


static void  test( unsigned int location)
{
   struct mulle_range  *result;
   unsigned int        hole;

   result = mulle_range_contains_bsearch( range1, 3, mulle_range_make( location, 1));
   if( result)
   {
      printf( "%u : NO\n", location);
      return;
   }

   hole = _mulle_range_hole_bsearch( range1, 3, location);
   printf( "%u : %u\n", location, hole);
}



int   main( int argc, char *argv[])
{
   unsigned int  i;

   // coverage
   mulle_range_contains_bsearch( NULL, 0, mulle_range_make( 1, 1));

   for( i = 1; i <= 6; i++)
      test( i);
   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
