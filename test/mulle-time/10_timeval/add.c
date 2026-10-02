#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
#ifndef _WIN32
   struct timeval   a;
   struct timeval   b;
   struct timeval   c;

   a.tv_sec = 1;
   b.tv_sec = 2;
   a.tv_usec = 999999;
   b.tv_usec = 1;

   c = timeval_add( a, b);
   printf( "%lld.%lld\n", (long long) c.tv_sec, (long long) c.tv_usec);
#endif
   return( 0);
}