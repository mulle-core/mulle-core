#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// Checks which of the "constructed" ranges are considered valid by
// mulle_range_is_valid. Output is word size independent.
//
int   main( int argc, char *argv[])
{
   struct mulle_range   range;

   range = mulle_range_zero;
   printf( "zero           : is_valid=%d\n", mulle_range_is_valid( range));

   range = mulle_range_make( 0, 0);
   printf( "make(0,0)      : is_valid=%d\n", mulle_range_is_valid( range));

   range = mulle_range_make( 1, 10);
   printf( "make(1,10)     : is_valid=%d\n", mulle_range_is_valid( range));

   range = mulle_range_make_invalid();
   printf( "make_invalid   : is_valid=%d not_found=%d\n",
           mulle_range_is_valid( range),
           range.location == mulle_not_found_e);

   range = mulle_range_make_all();
   printf( "make_all       : is_valid=%d location_is_0=%d length_is_minus_1=%d\n",
           mulle_range_is_valid( range),
           range.location == 0,
           range.length == (uintptr_t) -1);

   range = mulle_range_make( mulle_range_location_max, 1);
   printf( "make(max,1)    : is_valid=%d\n", mulle_range_is_valid( range));

   range = mulle_range_make( mulle_range_location_max, 2);
   printf( "make(max,2)    : is_valid=%d\n", mulle_range_is_valid( range));

   range = mulle_range_make_locations( 10, 4);
   printf( "locations(10,4): location=%td length=%td\n",
           range.location,
           range.length);

   return( 0);
}
