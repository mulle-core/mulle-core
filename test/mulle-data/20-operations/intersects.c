#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// mulle_range_intersects() and mulle_range_intersection() are supposed to
// answer the same question. This test compares both for a grid of ranges,
// including adjacent (touching) and zero length ranges.
//
static void   mulle_range_print( struct mulle_range a)
{
   if( ! a.length)
      printf( "NULL");
   else
      printf( "%td-%td (%td)", a.location, a.location + a.length - 1, a.length);
}


int   main( int argc, char *argv[])
{
   struct mulle_range   a;
   struct mulle_range   b;
   int                  predicate;
   int                  result;

   for( a.location = 0; a.location <= 10; a.location += 5)
      for( a.length = 0; a.length <= 10; a.length += 5)
         for( b.location = 0; b.location <= 10; b.location += 5)
            for( b.length = 0; b.length <= 10; b.length += 5)
            {
               predicate = mulle_range_intersects( a, b);
               result    = mulle_range_intersection( a, b).length != 0;

               mulle_range_print( a);
               printf( " x ");
               mulle_range_print( b);
               printf( " : intersects=%d intersection=%d%s\n",
                       predicate,
                       result,
                       predicate != result ? "  DIFFER" : "");
            }

   return( 0);
}
