//
// Test for bug #3.2: mulle_vararg_sizeof_float() returns 4 instead of 8
// A float vararg occupies 8 bytes (promoted to double), but sizeof_float()
// incorrectly routes through mulle_vararg_sizeof_integer which returns
// sizeof(int) = 4.
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


int   main( void)
{
   int   sizeof_float;
   int   sizeof_double;
   int   sizeof_fp_float;
   int   sizeof_fp_double;

   sizeof_float     = mulle_vararg_sizeof_float();
   sizeof_double    = mulle_vararg_sizeof_double();
   sizeof_fp_float  = mulle_vararg_sizeof_fp( float);
   sizeof_fp_double = mulle_vararg_sizeof_fp( double);

   //
   // sizeof_float must equal sizeof_fp(float) because float promotes to
   // double in varargs. Both should be 8 (sizeof(double)).
   //
   if( sizeof_float != sizeof_fp_float)
   {
      printf( "FAIL: sizeof_float()=%d != sizeof_fp(float)=%d\n",
              sizeof_float, sizeof_fp_float);
      return( 1);
   }

   //
   // sizeof_double must equal sizeof_fp(double).
   //
   if( sizeof_double != sizeof_fp_double)
   {
      printf( "FAIL: sizeof_double()=%d != sizeof_fp(double)=%d\n",
              sizeof_double, sizeof_fp_double);
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
