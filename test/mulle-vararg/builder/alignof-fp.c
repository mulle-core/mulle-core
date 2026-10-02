//
// Test for bug #3.3: mulle_vararg_alignof_fp uses sizeof(int) cutoff
// instead of sizeof(double), producing wrong alignment for double.
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>
#include <stdalign.h>


int   main( void)
{
   int   align_fp_float;
   int   align_fp_double;
   int   align_fp_longdouble;
   int   expected_double_align;

   align_fp_float      = mulle_vararg_alignof_fp( float);
   align_fp_double     = mulle_vararg_alignof_fp( double);
   align_fp_longdouble = mulle_vararg_alignof_fp( long double);
   expected_double_align = alignof( double);

   //
   // alignof_fp(double) should be alignof(double), not alignof(long double).
   // A double is promoted to double (no change), so it should align to
   // alignof(double).
   //
   if( align_fp_double != expected_double_align)
   {
      printf( "FAIL: alignof_fp(double)=%d != alignof(double)=%d\n",
              align_fp_double, expected_double_align);
      return( 1);
   }

   //
   // alignof_fp(float) should be alignof(double), since float promotes
   // to double.
   //
   if( align_fp_float != expected_double_align)
   {
      printf( "FAIL: alignof_fp(float)=%d != alignof(double)=%d\n",
              align_fp_float, expected_double_align);
      return( 1);
   }

   //
   // alignof_fp(long double) should be alignof(long double)
   //
   if( align_fp_longdouble != (int) alignof( long double))
   {
      printf( "FAIL: alignof_fp(long double)=%d != alignof(long double)=%d\n",
              align_fp_longdouble, (int) alignof( long double));
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
