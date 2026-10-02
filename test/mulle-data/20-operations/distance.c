#include <mulle-data/mulle-data.h>

#include <stdio.h>

static void  mulle_range_print( struct mulle_range a)
{
   if( ! a.length)
      printf( "NULL");  
   else
      printf( "%td-%td (%td)", a.location, a.location + a.length - 1, a.length);  
}


int   main( int argc, char *argv[])
{
   struct mulle_range   a;
   uintptr_t            c;

   for( a.location = 0; a.location < 100; a.location += 25)
      for( a.length = 0; a.length < 100; a.length += 25)
         for( c = 0; c <= 150; c += 25)
         {
            mulle_range_print( a);
            printf( " distance to %td = %td\n",
                  c, mulle_range_distance_to_location( a, c));
         }

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
