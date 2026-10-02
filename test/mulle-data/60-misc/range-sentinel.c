#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// mulle_not_found_e is returned by get_first_location/get_last_location for
// empty ranges. This test proves that value is OUT OF BAND: no *valid*
// non-empty range can produce it, because mulle_range_is_valid caps
// location at mulle_range_location_max == mulle_not_found_e - 1.
//
// Contrast with the hash APIs, where the -1 "in progress" sentinel is a
// value real data can legitimately produce. Here it cannot.
//
// Output is word size independent.
//
int   main( int argc, char *argv[])
{
   struct mulle_range   range;

   // the ceiling is exactly one below the sentinel, by construction
   printf( "location_max+1==not_found : %d\n",
           mulle_range_location_max + 1 == mulle_not_found_e);

   // a non-empty range starting AT the sentinel: mulle_range_make allows it
   // (invalid ranges can be constructed freely, then filtered with is_valid),
   // but is_valid rejects it.
   range = mulle_range_make( mulle_not_found_e, 1);
   printf( "is_valid({nf,1})          : %d\n", mulle_range_is_valid( range));

   // { mulle_not_found_e, 0 } is the documented invalid-range marker
   range = mulle_range_make( mulle_not_found_e, 0);
   printf( "make(nf,0) allowed        : %d\n",
           range.location == mulle_not_found_e);

   // highest legal non-empty range: first and last are both below sentinel
   range = mulle_range_make( mulle_range_location_max, 1);
   printf( "highest valid range       : is_valid=%d\n",
           mulle_range_is_valid( range));
   printf( "  first==not_found        : %d\n",
           mulle_range_get_first_location( range) == mulle_not_found_e);
   printf( "  last==not_found         : %d\n",
           mulle_range_get_last_location( range) == mulle_not_found_e);

   // empty range: sentinel is the answer, and it means "nothing here"
   range = mulle_range_make( 100, 0);
   printf( "empty range               : first==not_found=%d last==not_found=%d\n",
           mulle_range_get_first_location( range) == mulle_not_found_e,
           mulle_range_get_last_location( range) == mulle_not_found_e);

   // a range whose end lands exactly on the sentinel is still safe:
   // last_location is end-1, which is below the sentinel
   range = mulle_range_make( mulle_not_found_e - 4, 4);
   printf( "end lands on sentinel     : is_valid=%d last==not_found=%d\n",
           mulle_range_is_valid( range),
           mulle_range_get_last_location( range) == mulle_not_found_e);

   return( 0);
}
