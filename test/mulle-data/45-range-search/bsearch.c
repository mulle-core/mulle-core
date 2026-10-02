#include <mulle-data/mulle-data.h>

#include <stdio.h>


struct mulle_range    range1[] =
{
  { 10, 10 },
  { 30, 10 },
  { 50, 10 }
};


static void  test( struct mulle_range  r)
{
   struct mulle_range  *result;

   result = mulle_range_intersects_bsearch( range1, 3, r);
   if( result)
      printf( "[%ld/%ld] intersects [%ld/%ld] \n", (long) r.location, (long) r.length, (long) result->location, (long) result->length);

   result = mulle_range_contains_bsearch( range1, 3, r);
   if( result)
      printf( "[%ld/%ld] is contained in [%ld/%ld] \n", (long) r.location, (long) r.length, (long) result->location, (long) result->length);
}


static int  coverage( void)
{
   if( mulle_range_contains_bsearch( NULL, 1, mulle_range_make( 0, 0)))
      return( -1);
   if( mulle_range_contains_bsearch( NULL, 0, mulle_range_make_invalid()))
      return( -1);
   if( mulle_range_contains_bsearch( range1, 0, mulle_range_make_invalid()))
      return( -1);

   if( mulle_range_intersects_bsearch( NULL, 1, mulle_range_make( 0, 0)))
      return( -1);
   if( mulle_range_intersects_bsearch( NULL, 0, mulle_range_make_invalid()))
      return( -1);
   if( mulle_range_intersects_bsearch( range1, 0, mulle_range_make_invalid()))
      return( -1);
   return( 0);
}



int   main( int argc, char *argv[])
{
   test( mulle_range_make(  0, 10));
   test( mulle_range_make( 10, 10));
   test( mulle_range_make(  0, 20));
   test( mulle_range_make( 10, 20));
   test( mulle_range_make( 60, 10));

   return( coverage());
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
