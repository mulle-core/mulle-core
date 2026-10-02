#include <mulle-data/mulle-data.h>

#include <stdio.h>

static void  mulle_range_print( struct mulle_range a)
{
   if( ! a.length)
      printf( "NULL");  
   else
      printf( "%tu-%tu (%tu)", a.location, a.location + a.length - 1, a.length);
}


int   main( int argc, char *argv[])
{
   struct mulle_range   a;
   uintptr_t            location;

   for( a.location = 0; a.location < 4; a.location += 1)
      for( a.length = 0; a.length < 4; a.length += 1)
         for( location = 0; location <= 6; location += 1)
         {
            mulle_range_print( a);
            printf( " <= %tu = %s\n",
               location,
               mulle_range_less_than_or_equal_to_location( a, location) ? "YES" : "NO");
         }

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
