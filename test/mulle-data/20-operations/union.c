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
   struct mulle_range   b;
   struct mulle_range   c;

   for( a.location = 0; a.location < 100; a.location += 25)
      for( b.location = 0; b.location < 100; b.location += 25)
         for( a.length = 0; a.length < 100; a.length += 25)
            for( b.length = 0; b.length < 100; b.length += 25)
            {
               c = mulle_range_union( a, b);
               mulle_range_print( a);
               printf( " union ");
               mulle_range_print( b);
               printf( " = ");
               mulle_range_print( c);
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
