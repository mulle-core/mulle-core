#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timespec   a;
   struct timespec   b;
   struct timespec   c;

   // timespec_sub with borrow
   a.tv_sec  = 2;
   a.tv_nsec = 0;
   b.tv_sec  = 1;
   b.tv_nsec = 500000000;
   c = timespec_sub( a, b);
   printf( "timespec_sub %lld.%09ld\n", (long long) c.tv_sec, (long) c.tv_nsec);

   // timespec_sub negative result
   a.tv_sec  = 1;
   a.tv_nsec = 0;
   b.tv_sec  = 2;
   b.tv_nsec = 0;
   c = timespec_sub( a, b);
   printf( "timespec_sub_neg %lld.%09ld\n", (long long) c.tv_sec, (long) c.tv_nsec);

   // timespec_compare
   a.tv_sec  = 1;
   a.tv_nsec = 0;
   b.tv_sec  = 1;
   b.tv_nsec = 1;
   printf( "timespec_compare %d\n", timespec_compare( a, b));
   a.tv_nsec = 1;
   b.tv_nsec = 0;
   printf( "timespec_compare2 %d\n", timespec_compare( a, b));
   a.tv_nsec = 1;
   b.tv_nsec = 1;
   printf( "timespec_compare3 %d\n", timespec_compare( a, b));

   // timespec_add with carry
   a.tv_sec  = 1;
   a.tv_nsec = 999999999;
   b.tv_sec  = 1;
   b.tv_nsec = 1;
   c = timespec_add( a, b);
   printf( "timespec_add_carry %lld.%09ld\n", (long long) c.tv_sec, (long) c.tv_nsec);

   return( 0);
}