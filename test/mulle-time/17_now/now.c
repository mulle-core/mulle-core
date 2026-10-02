#include <mulle-time/mulle-time.h>

#include <math.h>
#include <stdio.h>

int   main( int argc, char *argv[])
{
   mulle_calendartime_t   c1;
   mulle_calendartime_t   c2;
   mulle_timeinterval_t   i1;
   mulle_timeinterval_t   i2;
   mulle_absolutetime_t   a1;
   mulle_absolutetime_t   a2;
   int                    failures;

   failures = 0;

   // calendar time and plain interval "now" must agree (same epoch, same clock)
   i1 = mulle_timeinterval_now();
   c1 = mulle_calendartime_now();
   if( fabs( i1 - c1) > 0.001)
   {
      fprintf( stderr, "calendar/interval mismatch %f vs %f\n", i1, c1);
      failures = 1;
   }

   // two consecutive calendar reads must not go backwards
   c1 = mulle_calendartime_now();
   c2 = mulle_calendartime_now();
   if( c2 < c1)
   {
      fprintf( stderr, "calendar went backwards\n");
      failures = 1;
   }

   // absolute time must be monotonic
   a1 = mulle_absolutetime_now();
   a2 = mulle_absolutetime_now();
   if( a2 < a1)
   {
      fprintf( stderr, "absolute went backwards\n");
      failures = 1;
   }

   // interval "now" must be monotonic too
   i1 = mulle_timeinterval_now();
   i2 = mulle_timeinterval_now();
   if( i2 < i1)
   {
      fprintf( stderr, "interval went backwards\n");
      failures = 1;
   }

   printf( "now ok\n");
   return( failures);
}