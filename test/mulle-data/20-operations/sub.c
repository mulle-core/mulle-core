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


int   main( int argc, char *argv[])
{
   struct mulle_range   a;
   struct mulle_range   b;
   struct mulle_range   c[ 2];
   unsigned int         i;

   for( a.location = 0; a.location < 100; a.location += 25)
      for( b.location = 0; b.location < 100; b.location += 25)
         for( a.length = 0; a.length < 100; a.length += 25)
            for( b.length = 0; b.length < 100; b.length += 25)
//   a = mulle_range_make( 25, 25);
//   b = mulle_range_make( 0, 75);

            {
               mulle_range_subtract( a, b, c);
               mulle_range_print( a);
               printf( " - ");
               mulle_range_print( b);
               printf( " = ");
               for( i = 0; i < 2; i++)
               {
                  assert( mulle_range_is_valid( c[ i]));
                  if( i)
                     printf( ", ");
                  mulle_range_print( c[ i]); 
               }
               printf( "\n");
            }

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
