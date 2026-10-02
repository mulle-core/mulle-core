#include <mulle-time/mulle-time.h>

#include <stdio.h>
#include <unistd.h>
#include <math.h>

int   main( int argc, char *argv[])
{
   mulle_relativetime_t   t1;
   mulle_relativetime_t   t2;
   mulle_relativetime_t   diff;

   t1 = mulle_relativetime_now();
   sleep( 2);
   t2 = mulle_relativetime_now();
   
   diff = t2 - t1;
   printf( "%.1f\n", diff);
   
   return( fabs( diff - 2.0) < 0.1 ? 0 : 1);
}
