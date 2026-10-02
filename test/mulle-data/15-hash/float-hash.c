#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// The floating point hashes funnel everything through mulle_float_hash, so
// this checks how much resolution actually survives. Only comparisons are
// printed, so the output is word size independent.
//
int   main( int argc, char *argv[])
{
   printf( "float  0.0 == -0.0            : %d\n",
           mulle_float_hash( 0.0f) == mulle_float_hash( -0.0f));
   printf( "double 0.0 == -0.0            : %d\n",
           mulle_double_hash( 0.0) == mulle_double_hash( -0.0));
   printf( "double 1.0 == float 1.0       : %d\n",
           mulle_double_hash( 1.0) == mulle_float_hash( 1.0f));
   printf( "double 1.0 == 1.0 + 1e-12     : %d\n",
           mulle_double_hash( 1.0) == mulle_double_hash( 1.0 + 1e-12));
   printf( "double 1e300 == 2e300         : %d\n",
           mulle_double_hash( 1e300) == mulle_double_hash( 2e300));
   printf( "double 1e-300 == 2e-300       : %d\n",
           mulle_double_hash( 1e-300) == mulle_double_hash( 2e-300));
   printf( "long double 1e300 == 2e300    : %d\n",
           mulle_long_double_hash( 1e300L) == mulle_long_double_hash( 2e300L));
   printf( "double 1.0 == integer 1       : %d\n",
           mulle_double_hash( 1.0) == mulle_integer_hash( 1));
   printf( "long long 1 == integer 1      : %d\n",
           mulle_long_long_hash( 1) == mulle_integer_hash( 1));
   printf( "double 2.0 != double 3.0      : %d\n",
           mulle_double_hash( 2.0) != mulle_double_hash( 3.0));

   return( 0);
}
