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
   int                  flag;

   for( a.location = 0; a.location < 4; a.location += 1)
      for( b.location = 0; b.location < 4; b.location += 1)
         for( a.length = 0; a.length < 4; a.length += 1)
            for( b.length = 0; b.length < 4; b.length += 1)
            {
               flag = mulle_range_contains( a, b);
               mulle_range_print( a);
               printf( " contains ");
               mulle_range_print( b);
               printf( " = %s\n", flag ? "YES" : "NO");
            }

   return( 0);
}


/*
 * extension : mulle-sde/c-test-library-demo
 * directory : demo/all
 * template  : .../hello.c
 * Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
 */
