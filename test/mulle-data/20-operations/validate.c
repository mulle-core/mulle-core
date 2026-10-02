#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// mulle_range_validate_against_length is documented to return an invalid
// range, if the range does not fit the given length. It also treats a length
// of (uintptr_t) -1 as "everything". Output is word size independent.
//
static void   print_result( char *title, struct mulle_range range, uintptr_t length)
{
   printf( "%-28.28s: is_valid=%d is_invalid_marker=%d",
           title,
           mulle_range_is_valid( range),
           range.location == mulle_not_found_e && ! range.length);
   if( mulle_range_is_valid( range))
      printf( " fits=%d", range.location + range.length <= length);
   printf( "\n");
}


int   main( int argc, char *argv[])
{
   struct mulle_range   range;

   range = mulle_range_validate_against_length( mulle_range_make( 0, 10), 10);
   print_result( "{0,10} against 10", range, 10);

   range = mulle_range_validate_against_length( mulle_range_make( 5, 10), 10);
   print_result( "{5,10} against 10", range, 10);

   range = mulle_range_validate_against_length( mulle_range_make( 20, 1), 10);
   print_result( "{20,1} against 10", range, 10);

   range = mulle_range_validate_against_length( mulle_range_make( 0, (uintptr_t) -1), 10);
   print_result( "{0,-1} against 10", range, 10);

   range = mulle_range_validate_against_length( mulle_range_make( 5, (uintptr_t) -1), 10);
   print_result( "{5,-1} against 10", range, 10);

   // location past the end, combined with the "everything" length
   range = mulle_range_validate_against_length( mulle_range_make( 20, (uintptr_t) -1), 10);
   print_result( "{20,-1} against 10", range, 10);

   range = mulle_range_validate_against_length( mulle_range_make( 0, 0), 0);
   print_result( "{0,0} against 0", range, 0);

   return( 0);
}
