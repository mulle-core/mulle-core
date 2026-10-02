//
// Test: mulle_vararg_next_long_double
// (the dedicated long-double reader; buffer-alignment.c already covers
//  the mulle_vararg_next_longdouble convenience)
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( sizeof( long double) * 2)];
   mulle_vararg_list              list;
   mulle_vararg_list              q;
   long double                    a;
   long double                    b;
   long double                    r1;
   long double                    r2;

   a = 18.48e10L;
   b = 48.18e10L;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_longdouble( q, a);
   mulle_vararg_push_longdouble( q, b);

   r1 = mulle_vararg_next_long_double( list, long double);
   r2 = mulle_vararg_next_long_double( list, long double);

   if( r1 != a || r2 != b)
   {
      printf( "FAIL: long double round-trip: got %Lg %Lg\n", r1, r2);
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
