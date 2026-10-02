#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timeval   u;
   struct timeval   v;
   struct timeval   w;

   // timeval_add with carry
   u.tv_sec  = 1;
   u.tv_usec = 999999;
   v.tv_sec  = 1;
   v.tv_usec = 1;
   w = timeval_add( u, v);
   printf( "timeval_add %lld.%06ld\n", (long long) w.tv_sec, (long) w.tv_usec);

   // timeval_sub with borrow
   u.tv_sec  = 2;
   u.tv_usec = 0;
   v.tv_sec  = 1;
   v.tv_usec = 500000;
   w = timeval_sub( u, v);
   printf( "timeval_sub %lld.%06ld\n", (long long) w.tv_sec, (long) w.tv_usec);

   // timeval_compare
   u.tv_sec  = 1;
   u.tv_usec = 0;
   v.tv_sec  = 1;
   v.tv_usec = 1;
   printf( "timeval_compare %d\n", timeval_compare( u, v));

   // timeval_add with carry
   u.tv_sec  = 1;
   u.tv_usec = 999999;
   v.tv_sec  = 1;
   v.tv_usec = 1;
   w = timeval_add( u, v);
   printf( "timeval_add_carry %lld.%06ld\n", (long long) w.tv_sec, (long) w.tv_usec);

   return( 0);
}