//
//  mulle-timetype.h
//  mulle-time
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_timetype_h__
#define mulle_timetype_h__

#include <mulle-c11/mulle-c11.h>

#if defined( MULLE__TIME_BUILD) || defined( MULLE__CORE_BUILD)
# define MULLE__TIME_GLOBAL    MULLE_C_GLOBAL
#else
# if defined( MULLE_TIME_INCLUDE_DYNAMIC) || (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE_TIME_INCLUDE_STATIC))
#  define MULLE__TIME_GLOBAL   MULLE_C_EXTERN_GLOBAL
# else
#  define MULLE__TIME_GLOBAL   extern
# endif
#endif


typedef enum
{
   MulleTimeAscending  = -1,
   MulleTimeSame       = 0,
   MulleTimeDescending = 1
} mulle_time_comparison_t;


// Offset in seconds between the Unix epoch (1970-01-01) and the Cocoa
// reference date (2001-01-01). The calendar epoch used by
// mulle_timeinterval_now is the Unix epoch, so this constant is only for
// conversions to/from the Cocoa reference date.
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


// don't want fmod/-lm in this library, so use a floor based mod instead.
// result is in [0, m) for m > 0. NaN values and invalid m return 0.0.
//
// Returns nonzero if value is neither NaN nor infinite. Avoids a dependency
// on math.h: an infinite value is the only one where value - value is not 0.0
// (it is NaN), and NaN is the only value not equal to itself.
static inline int   mulle_timeinterval_is_finite( mulle_timeinterval_t value)
{
   return( value == value && value - value == 0.0);
}


static inline mulle_timeinterval_t   mulle_timeinterval_mod( mulle_timeinterval_t value,
                                                             mulle_timeinterval_t m)
{
   mulle_timeinterval_t   quotient;
   mulle_timeinterval_t   loss;
   long long              truncated;

   // reject NaN/infinite values and invalid (nonpositive/NaN/infinite) m
   if( ! mulle_timeinterval_is_finite( value) || ! mulle_timeinterval_is_finite( m)
       || ! (m > 0.0))
      return( 0.0);

   // quotient of value / m truncated towards zero, adjusted to floor.
   // Reject a quotient that does not fit into long long before converting,
   // as the conversion would be undefined behavior.
   quotient  = value / m;
   if( quotient >= 9223372036854775808.0 || quotient < -9223372036854775808.0)
      return( 0.0);
   truncated = (long long) quotient;
   if( (double) truncated > quotient)
      truncated -= 1;

   loss = value - (double) truncated * m;
   return( loss);
}


//
// this returns either the next lowest or the next highest timeinterval that
// is evenly divisible by rate. "snaps to rate"
// A rate of 0.3 produces the following valid value sequence
// 0, 0.3, 0.6, 0.9, 1.2, ..., INFINITY
// Given a 1.0 for value, this will return 0.9 given a 1.1 it will return
// 1.2.
//
// The remainder is measured as a positive distance within the rate bucket,
// so negative values quantize symmetrically to positive values. Ties round
// up (away from zero). A nonpositive, NaN or infinite rate is invalid and
// returns the value unchanged. NaN and infinite values are also returned
// unchanged.
//
static inline mulle_timeinterval_t
   mulle_timeinterval_quantize( mulle_timeinterval_t value,
                                mulle_timeinterval_t rate)
{
   mulle_timeinterval_t   loss;
   mulle_timeinterval_t   quantized;

   if( ! (rate > 0.0) || ! mulle_timeinterval_is_finite( rate)
       || ! mulle_timeinterval_is_finite( value))
      return( value);

   loss      = mulle_timeinterval_mod( value, rate);
   quantized = value - loss;        // quantize to lower
   if( loss >= rate / 2)            // or quantize to higher
      quantized += rate;

   // quantized may have encurred a small error here due to mod
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
// The inline functions in this header need struct timespec, struct timeval
// and the CLOCK_* constants, which on glibc require a POSIX feature-test
// macro. The library's own sources define _GNU_SOURCE before any include.
//
// For header-only consumers we only define _GNU_SOURCE here if glibc's
// feature detection (features.h) has not run yet. Defining it afterwards
// would silently change feature visibility for the rest of the translation
// unit and can break glibc itself. Consumers that include other system
// headers before mulle-time.h must define _GNU_SOURCE (or an equivalent
// feature-test macro) before their first system header include.
//
#ifndef  _GNU_SOURCE
# if ! defined( _FEATURES_H) && ! defined( _POSIX_C_SOURCE)
#  define _GNU_SOURCE
# endif
#endif

#include <time.h>

#endif
