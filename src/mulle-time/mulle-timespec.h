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

#include "mulle-timetype.h"
#include "mulle-relativetime.h"


// use this for getting "date" "time"
MULLE__TIME_GLOBAL
mulle_timeinterval_t   mulle_timeinterval_now( void);

// use this for animations, timewatch etc.
// https://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
MULLE__TIME_GLOBAL
mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void);


MULLE__TIME_GLOBAL
void   mulle_relativetime_sleep( mulle_relativetime_t time);


// timespec as used by nanosleep

static inline mulle_time_comparison_t   timespec_compare( struct timespec a,
                                                          struct timespec b)
{
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

   result.tv_sec  = (unsigned long) time;
   result.tv_nsec = (unsigned long) ((time - result.tv_sec) * (double) (1000L*1000*1000));

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
   return( mulle_relativetime_init_with_s_ns( a.tv_sec, a.tv_sec));
}


#endif
