#ifndef mulle_timetype_h__
#define mulle_timetype_h__

#include <mulle-c11/mulle-c11.h>

#ifdef MULLE_TIME_BUILD
# define MULLE_TIME_GLOBAL    MULLE_C_GLOBAL
#else
# if defined( MULLE_TIME_INCLUDE_DYNAMIC) || (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE_TIME_INCLUDE_STATIC))
#  define MULLE_TIME_GLOBAL   MULLE_C_EXTERN_GLOBAL
# else
#  define MULLE_TIME_GLOBAL   extern
# endif
#endif


typedef enum
{
   MulleTimeAscending  = -1,
   MulleTimeSame       = 0,
   MulleTimeDescending = 1
} mulle_time_comparison_t;


#define MULLE_TIMEINTERVAL_SINCE_1970            978307200.0

// compatible values
#define MULLE_TIMEINTERVAL_DISTANT_FUTURE   63113904000.0
#define MULLE_TIMEINTERVAL_DISTANT_PAST    -63114076800.0


// will be typedef to NSTimeInterval
typedef double   mulle_timeinterval_t;

// can produce absolute time or relative
static inline mulle_timeinterval_t
   mulle_timeinterval_add( mulle_timeinterval_t a, mulle_timeinterval_t b)
{
   return( a + b);
}


// can produce absolute time or relative
static inline mulle_timeinterval_t
   mulle_timeinterval_subtract( mulle_timeinterval_t a, mulle_timeinterval_t b)
{
   return( a - b);
}


// don't want fmod/-lm in this library, so use fmod adapted from
// https://stackoverflow.com/questions/26342823/implementation-of-fmod-function
static mulle_timeinterval_t   mulle_timeinterval_mod( mulle_timeinterval_t value,
                                                      mulle_timeinterval_t m)
{
   return( value - (double) (long long) (value / m) * m);
}


//
// this returns either the next lowest or the next highest timeinterval that
// is evenly divisibly by rate. "snaps to reate"
// A rate of 0.3 produces the following valid value sequence
// 0, 0.3, 0.6, 0.9, 1.2, ..., INFINITY
// Given a 1.0 for value, this will return 0.9 given a 1.1 it will return
// 1.2.
//
//
static inline mulle_timeinterval_t
   mulle_timeinterval_quantize( mulle_timeinterval_t value,
                                mulle_timeinterval_t rate)
{
   mulle_timeinterval_t   loss;
   mulle_timeinterval_t   quantized;

   loss      = mulle_timeinterval_mod( value, rate);
   quantized = value - loss;        // quantize to lower
   if( loss >= rate / 2)            // or quantize to higher
      quantized += rate;

   // quantized may have encurred a small error here due to fmod
   return( quantized);
}


// no helper functions yet
struct mulle_timeintervalrange
{
   mulle_timeinterval_t   start;
   mulle_timeinterval_t   end;
};


static inline struct mulle_timeintervalrange
   mulle_timeintervalrange_make( mulle_timeinterval_t start,
                                 mulle_timeinterval_t end)
{
    struct mulle_timeintervalrange result = { start, end };

    return( result);
}



// timespec is preferable, timeval is like a fallback for unix
// it's usually not a problem to comment it out for platforms
// that don't support it

//
// TODO: make this a fail, if _GNU_SOURCE is undefined on linux
//       instead of secretly setting it
//
#ifndef  _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <time.h>

#endif
