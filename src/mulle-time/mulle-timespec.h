//
//  mulle-timespec.h
//  mulle-time
//
//  Copyright (c) 2019 Nat! - Mulle kybernetiK.
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
#ifndef mulle_timespec_h__
#define mulle_timespec_h__

#include <assert.h>

#include "mulle-timetype.h"
#include "mulle-relativetime.h"


// use this for getting "date" "time"
// On POSIX this is CLOCK_REALTIME. If the system clock call fails, the
// result is 0.0 (a valid Unix epoch timestamp), so a failure is not
// distinguishable from a real reading at the call site.
MULLE__TIME_GLOBAL
mulle_timeinterval_t   mulle_timeinterval_now( void);

// use this for animations, timewatch etc.
// https://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
// On POSIX this is CLOCK_MONOTONIC. Same failure policy as
// mulle_timeinterval_now: a failed clock call returns 0.0, which is also a
// plausible monotonic origin.
MULLE__TIME_GLOBAL
mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void);


MULLE__TIME_GLOBAL
void   mulle_relativetime_sleep( mulle_relativetime_t time);


// timespec as used by nanosleep

// All arithmetic and comparison helpers below require canonical inputs:
// tv_nsec in [0, 1e9). This is enforced with assert() in debug builds;
// release builds (NDEBUG) assume canonical inputs without checking.
static inline mulle_time_comparison_t   timespec_compare( struct timespec a,
                                                          struct timespec b)
{
   assert( a.tv_nsec >= 0 && a.tv_nsec < (1000L*1000*1000));
   assert( b.tv_nsec >= 0 && b.tv_nsec < (1000L*1000*1000));
   if( a.tv_sec > b.tv_sec)
      return( MulleTimeDescending);
   if( a.tv_sec < b.tv_sec)
      return( MulleTimeAscending);
   if( a.tv_nsec > b.tv_nsec)
      return( MulleTimeDescending);
   if( a.tv_nsec < b.tv_nsec)
      return( MulleTimeAscending);
   return( MulleTimeSame);
}

static inline struct timespec   timespec_add( struct timespec a,
                                              struct timespec b)
{
   struct timespec   result;
   int               carry;

   assert( a.tv_nsec >= 0 && a.tv_nsec < (1000L*1000*1000));
   assert( b.tv_nsec >= 0 && b.tv_nsec < (1000L*1000*1000));

   result.tv_nsec = a.tv_nsec + b.tv_nsec;
   carry = result.tv_nsec >= (1000L*1000*1000);
   if( carry)
      result.tv_nsec -= (1000L*1000*1000);
   result.tv_sec = a.tv_sec + b.tv_sec + carry;
   return( result);
}


static inline struct timespec   timespec_sub( struct timespec a,
                                              struct timespec b)
{
   struct timespec   result;
   int               carry;

   assert( a.tv_nsec >= 0 && a.tv_nsec < (1000L*1000*1000));
   assert( b.tv_nsec >= 0 && b.tv_nsec < (1000L*1000*1000));

   result.tv_nsec = a.tv_nsec - b.tv_nsec;
   carry = result.tv_nsec < 0;
   if( carry)
      result.tv_nsec += (1000L*1000*1000);
   result.tv_sec = a.tv_sec - b.tv_sec - carry;
   return( result);
}


static inline struct timespec
   timespec_make_with_relativetime( mulle_relativetime_t time)
{
   struct timespec   result;
   long long         sec;
   double            frac;
   long              nsec;

   // invalid: NaN, infinities and values outside the long long range become
   // a zero timespec. The bounds are +/-2^63, the representable long long
   // range; converting a value at or above 2^63 to long long would be
   // undefined behavior.
   if( ! (time == time) || time >= 9223372036854775808.0
       || time < -9223372036854775808.0)
   {
      result.tv_sec  = 0;
      result.tv_nsec = 0;
      return( result);
   }

   // canonical: tv_nsec in [0, 1e9), tv_sec = floor( time)
   sec  = (long long) time;   // truncates toward zero
   frac = time - (double) sec;
   if( frac < 0.0)
   {
      frac += 1.0;
      sec  -= 1;
   }
   // for very large values the double has no fractional bits left, so frac
   // can still be >= 1.0 or an exact multiple: normalize it into tv_sec
   if( frac >= 1.0)
   {
      sec  += (long long) frac;
      frac -= (double) (long long) frac;
   }
   nsec = (long) (frac * (double) (1000L*1000*1000));
   if( nsec >= (1000L*1000*1000))   // rounding can produce exactly 1e9
   {
      nsec = 0;
      sec  += 1;
   }

   result.tv_sec  = (time_t) sec;
   result.tv_nsec = nsec;

   // seconds not representable in time_t (e.g. 32-bit time_t) become a zero
   // timespec instead of silently truncating
   if( (long long) result.tv_sec != sec)
   {
      result.tv_sec  = 0;
      result.tv_nsec = 0;
   }

   return( result);
}



// deprecated version of above
static inline struct timespec
   mulle_relativetime_get_timespec( mulle_relativetime_t time)
{
   return( timespec_make_with_relativetime( time));
}



static inline mulle_relativetime_t
   mulle_relativetime_make_with_timespec( struct timespec a)
{
   return( mulle_relativetime_init_with_s_ns( a.tv_sec, a.tv_nsec));
}


#endif
