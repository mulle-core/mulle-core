#include <mulle-time/mulle-time.h>

#include <math.h>
#include <stdio.h>

int   main( int argc, char *argv[])
{
   mulle_relativetime_t   start;
   mulle_relativetime_t   elapsed;
   int                    failures;

   failures = 0;

   // invalid inputs return immediately
   mulle_relativetime_sleep( 0.0);
   mulle_relativetime_sleep( -1.0);

   // a small sleep should actually sleep roughly that long
   start = mulle_relativetime_now();
   mulle_relativetime_sleep( 0.05);
   elapsed = mulle_relativetime_now() - start;

   if( elapsed < 0.04)
   {
      fprintf( stderr, "slept only %f\n", elapsed);
      failures = 1;
   }
   else if( elapsed > 0.5)
   {
      fprintf( stderr, "slept too long %f\n", elapsed);
      failures = 1;
   }

   printf( "sleep ok\n");
   return( failures);
}