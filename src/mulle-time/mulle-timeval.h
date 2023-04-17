#ifndef mulle_timeval_h__
#define mulle_timeval_h__

#include "mulle-timetype.h"

#include <sys/time.h>

// use, if clock is not available (in timespec)
#if 0
static inline mulle_timeinterval_t   mulle_time_now( void)
{
   mulle_timeinterval_t   seconds;
   struct timeval         tv;

   gettimeofday( &tv, NULL);  // is known to be 1970 UTC
   seconds = (double) tv.tv_sec  +  (double) tv.tv_usec / (1000.0*1000);
   return( seconds - MULLE_TIMEINTERVAL_SINCE_1970);
}
#endif


//
// struct timeval is what gettimeofday returns
//
static inline mulle_time_comparison_t   timeval_compare( struct timeval a,
                                                         struct timeval b)
{
   if( a.tv_sec > b.tv_sec)
      return( MulleTimeDescending);
   if( a.tv_sec < b.tv_sec)
      return( MulleTimeAscending);
   if( a.tv_usec > b.tv_usec)
      return( MulleTimeDescending);
   if( a.tv_usec < b.tv_usec)
      return( MulleTimeAscending);
   return( MulleTimeSame);
}


static inline struct timeval   timeval_add( struct timeval a,
                                            struct timeval b)
{
   struct timeval   result;
   int              carry;

   result.tv_usec = a.tv_usec + b.tv_usec;
   carry = result.tv_usec >= (1000*1000);
   if( carry)
      result.tv_usec -= (1000*1000);
   result.tv_sec = a.tv_sec + b.tv_sec + carry;
   return( result);
}


static inline struct timeval   timeval_sub( struct timeval a,
                                            struct timeval b)
{
   struct timeval   result;
   int               carry;

   result.tv_usec = a.tv_usec - b.tv_usec;
   carry = result.tv_usec < 0;
   if( carry)
      result.tv_usec += (1000*1000);
   result.tv_sec = a.tv_sec - b.tv_sec - carry;
   return( result);
}

#endif
