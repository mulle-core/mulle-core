#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   // positive values
   printf( "q(0.0, 0.3)   -> %.1f\n", mulle_timeinterval_quantize( 0.0, 0.3));
   printf( "q(0.3, 0.3)   -> %.1f\n", mulle_timeinterval_quantize( 0.3, 0.3));
   printf( "q(0.9, 0.3)   -> %.1f\n", mulle_timeinterval_quantize( 0.9, 0.3));
   printf( "q(1.0, 0.3)   -> %.1f\n", mulle_timeinterval_quantize( 1.0, 0.3));
   printf( "q(1.1, 0.3)   -> %.1f\n", mulle_timeinterval_quantize( 1.1, 0.3));
   printf( "q(0.14, 0.3)  -> %.2f\n", mulle_timeinterval_quantize( 0.14, 0.3));
   printf( "q(0.16, 0.3)  -> %.2f\n", mulle_timeinterval_quantize( 0.16, 0.3));

   // negative values must quantize symmetrically
   printf( "q(-1.0, 0.3)  -> %.1f\n", mulle_timeinterval_quantize( -1.0, 0.3));
   printf( "q(-1.1, 0.3)  -> %.1f\n", mulle_timeinterval_quantize( -1.1, 0.3));
   printf( "q(-0.14, 0.3) -> %.2f\n", mulle_timeinterval_quantize( -0.14, 0.3));
   printf( "q(-0.16, 0.3) -> %.2f\n", mulle_timeinterval_quantize( -0.16, 0.3));

   // invalid rates are rejected (returned unchanged)
   printf( "q(1.0, 0.0)   -> %.1f\n", mulle_timeinterval_quantize( 1.0, 0.0));
   printf( "q(1.0, -0.3)  -> %.1f\n", mulle_timeinterval_quantize( 1.0, -0.3));
   printf( "q(1.0, nan)   -> %.1f\n", mulle_timeinterval_quantize( 1.0, 0.0 / 0.0));
   printf( "q(1.0, +inf)  -> %.1f\n", mulle_timeinterval_quantize( 1.0, 1.0 / 0.0));
   printf( "q(1.0, -inf)  -> %.1f\n", mulle_timeinterval_quantize( 1.0, -1.0 / 0.0));

   // NaN and infinite values are returned unchanged
   printf( "q(nan, 0.3)   -> finite=%d\n",
           mulle_timeinterval_is_finite( mulle_timeinterval_quantize( 0.0 / 0.0, 0.3)));
   printf( "q(+inf, 0.3)  -> finite=%d\n",
           mulle_timeinterval_is_finite( mulle_timeinterval_quantize( 1.0 / 0.0, 0.3)));
   printf( "q(-inf, 0.3)  -> finite=%d\n",
           mulle_timeinterval_is_finite( mulle_timeinterval_quantize( -1.0 / 0.0, 0.3)));

   // large values remain well-behaved
   printf( "q(1e6, 0.3)   -> %.3f\n", mulle_timeinterval_quantize( 1000000.0, 0.3));
   printf( "q(3.5e7, 0.1) -> %.3f\n", mulle_timeinterval_quantize( 35000000.0, 0.1));

   // huge value / tiny rate would overflow long long: no UB, returns unchanged
   printf( "q(1e308, 1e-9) -> %g\n", mulle_timeinterval_quantize( 1e308, 1e-9));

   // mod itself rejects NaN and infinite operands
   printf( "mod(nan, 0.3)  -> %.1f\n", mulle_timeinterval_mod( 0.0 / 0.0, 0.3));
   printf( "mod(1.0, nan)  -> %.1f\n", mulle_timeinterval_mod( 1.0, 0.0 / 0.0));
   printf( "mod(1.0, 0.0)  -> %.1f\n", mulle_timeinterval_mod( 1.0, 0.0));
   printf( "mod(1e308, 1e-9) -> %g\n", mulle_timeinterval_mod( 1e308, 1e-9));
   printf( "mod(1.5, 0.3)  -> %.3f\n", mulle_timeinterval_mod( 1.5, 0.3));

   return( 0);
}