//
//  mulle-timeval.h
//  mulle-time
//
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
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
