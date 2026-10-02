#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timespec   a;
   struct timespec   b;
   struct timespec   c;

   a.tv_sec = 1;
   b.tv_sec = 2;
   a.tv_nsec = 999999999;
   b.tv_nsec = 1;

   c = timespec_add( a, b);
   printf( "%lld.%lld\n", (long long) c.tv_sec, (long long) c.tv_nsec);
   return( 0);
}