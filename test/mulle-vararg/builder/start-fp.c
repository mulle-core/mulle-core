//
// Test: _mulle_vararg_start_fp
// A float first argument occupies a double-sized slot in the vararg layout,
// so the list must start at the first byte after sizeof( double).
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


struct
{
   float    a;   // double-sized slot in vararg layout
   double   b;
   double   c;
} value =
{
   1.0,
   2.0,
   3.0
};


int   main( void)
{
   mulle_vararg_list   list;
   double              b;
   double              c;

   _mulle_vararg_start_fp( list, value.a);

   b = mulle_vararg_next_fp( list, double);
   c = mulle_vararg_next_fp( list, double);

   if( b != 2.0 || c != 3.0)
   {
      printf( "FAIL: _mulle_vararg_start_fp: got %g %g\n", b, c);
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
