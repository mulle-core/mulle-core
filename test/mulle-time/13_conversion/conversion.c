#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timespec      ts;
   mulle_relativetime_t t;

   // timespec -> relativetime
   ts.tv_sec  = 0;
   ts.tv_nsec = 0;
   t = mulle_relativetime_make_with_timespec( ts);
   printf( "0 -> %.9f\n", t);

   ts.tv_sec  = 1;
   ts.tv_nsec = 250000000;
   t = mulle_relativetime_make_with_timespec( ts);
   printf( "1.25 -> %.9f\n", t);

   ts.tv_sec  = 1;
   ts.tv_nsec = 999999999;
   t = mulle_relativetime_make_with_timespec( ts);
   printf( "1.999999999 -> %.9f\n", t);

   // relativetime -> timespec
   t  = 0.0;
   ts = timespec_make_with_relativetime( t);
   printf( "0 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = 0.5;
   ts = timespec_make_with_relativetime( t);
   printf( "0.5 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = 1.25;
   ts = timespec_make_with_relativetime( t);
   printf( "1.25 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   // negative values normalize to canonical timespec
   t  = -0.5;
   ts = timespec_make_with_relativetime( t);
   printf( "-0.5 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = -1.25;
   ts = timespec_make_with_relativetime( t);
   printf( "-1.25 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   // deprecated alias agrees with the make variant
   ts.tv_sec  = 3;
   ts.tv_nsec = 123456789;
   t = mulle_relativetime_init_with_s_ns( ts.tv_sec, ts.tv_nsec);
   printf( "alias -> %.9f\n", t);

   // post-2038 timestamps must not be truncated to 32-bit
   ts.tv_sec  = (time_t) 4102444800;        // 2100-01-01
   ts.tv_nsec = 0;
   t = mulle_relativetime_make_with_timespec( ts);
   printf( "2100 -> %.9f\n", t);
   t = mulle_absolutetime_init_with_s_ns( (time_t) 4102444800, 0);
   printf( "2100 abs -> %.9f\n", t);

   // large values round-trip: no fractional bits, canonical (sec, 0)
   t  = 9007199254740992.0;                 // 2^53, exact in double
   ts = timespec_make_with_relativetime( t);
   printf( "2^53 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = 1e18;
   ts = timespec_make_with_relativetime( t);
   printf( "1e18 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   // NaN, infinities and out-of-range values become a zero timespec
   t  = (mulle_relativetime_t) 0.0 / 0.0;   // NaN
   ts = timespec_make_with_relativetime( t);
   printf( "nan -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = 1.0 / 0.0;                          // +inf
   ts = timespec_make_with_relativetime( t);
   printf( "+inf -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = -1.0 / 0.0;                         // -inf
   ts = timespec_make_with_relativetime( t);
   printf( "-inf -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = 1e19;                               // >= 2^63 seconds
   ts = timespec_make_with_relativetime( t);
   printf( "1e19 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   t  = -1e19;                              // < -2^63 seconds
   ts = timespec_make_with_relativetime( t);
   printf( "-1e19 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   // values just inside the long long range still convert
   t  = 9.0e18;
   ts = timespec_make_with_relativetime( t);
   printf( "9e18 -> %lld %09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);

   return( 0);
}