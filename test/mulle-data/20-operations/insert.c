#include <mulle-data/mulle-data.h>

#include <stdio.h>


static void  mulle_range_print( struct mulle_range a)
{
   if( ! mulle_range_is_valid( a))
      printf( "INVALID");
   else
      if( ! a.length)
         printf( "NULL");
      else
         printf( "%tu-%tu (%tu)", a.location, a.location + a.length - 1, a.length);
}


static void  test( struct mulle_range a, struct mulle_range b)
{
   struct mulle_range   result[ 2];
   unsigned int         i;
   unsigned int         n;
   char                 *sep = " ";

   mulle_range_print( a);
   printf( " <-insert- ");
   mulle_range_print( b);

   n = mulle_range_insert( a, b, result);
   printf( " : %u {", n);
   for( i = 0; i < n; i++)
   {
      printf( "%s", sep);
      mulle_range_print( result[ i]);
      sep =", ";
   }
   printf( "}\n");
}


int   main( int argc, char *argv[])
{
   test( mulle_range_make( 0, 0),   mulle_range_make( 0, 0));
   test( mulle_range_make( 0, 0),   mulle_range_make( 18, 48));
   test( mulle_range_make( 18, 48), mulle_range_make( 0, 0));

   test( mulle_range_make( 0, 9),   mulle_range_make( 2, 3));
   test( mulle_range_make( 2, 4),   mulle_range_make( 0, 2));
   test( mulle_range_make( 2, 4),   mulle_range_make( 6, 2));
   test( mulle_range_make( 2, 4),   mulle_range_make( 8, 2));
   test( mulle_range_make( 2, 4),   mulle_range_make( 3, 5));
   test( mulle_range_make( 2, 4),   mulle_range_make( 1, 2));

   test( mulle_range_make_invalid(), mulle_range_make( 18, 48));
   test( mulle_range_make( 18, 48),  mulle_range_make_invalid());
   test( mulle_range_make_invalid(), mulle_range_make_invalid());

   return( 0);
}
