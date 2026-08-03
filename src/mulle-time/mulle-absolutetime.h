//
//  mulle-absolutetime.h
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
#ifndef mulle_absolutetime_h__
#define mulle_absolutetime_h__


#include "mulle-timetype.h"

#include "mulle-timespec.h"

// It's simple. mulle_absolutetime_t is a FP number containing seconds as a
// timestamp relative to the start of system boot (like a diff on `uptime`).
// It should not "jump" if the computer is put into sleep mode.
//
// Arithmetic on mulle_absolutetime_t and mulle_relativetime_t has six useful
// operations
//
// 1. mulle_absolutetime_t + mulle_absolutetime_t = invalid
// 2. mulle_absolutetime_t + mulle_relativetime_t = mulle_absolutetime_t
// 3. mulle_relativetime_t + mulle_absolutetime_t = mulle_absolutetime_t
// 4. mulle_relativetime_t + mulle_relativetime_t = mulle_relativetime_t
//
// 5. mulle_absolutetime_t - mulle_absolutetime_t = mulle_relativetime_t
// 6. mulle_absolutetime_t - mulle_relativetime_t = mulle_absolutetime_t
// 7. mulle_relativetime_t - mulle_absolutetime_t = invalid
// 8. mulle_relativetime_t - mulle_relativetime_t = mulle_relativetime_t
//
typedef mulle_timeinterval_t   mulle_absolutetime_t;


static inline mulle_absolutetime_t   mulle_absolutetime_now( void)
{
   return( (mulle_absolutetime_t) mulle_timeinterval_now_monotonic());
}



static inline void
   _mulle_absolutetime_init( mulle_absolutetime_t *p, mulle_timeinterval_t value)
{
   *p = value;
}


static inline void
   mulle_absolutetime_init( mulle_absolutetime_t *p, mulle_timeinterval_t value)
{
   if( p)
      _mulle_absolutetime_init( p, value);
}


static inline mulle_absolutetime_t
   mulle_absolutetime_make( mulle_timeinterval_t value)
{
   return( (mulle_absolutetime_t) value);
}

//
// struct mulle_absolutetimerange is a... range of time with a start and an end.
// The end is thought of as being inclusive.
//
struct mulle_absolutetimerange
{
   mulle_absolutetime_t   start;
   mulle_absolutetime_t   end;
};


#define MULLE_ABSOLUTETIMERANGE_DATA( start, end)  \
   ((struct mulle_absolutetimerange)               \
   {                                               \
      (start),                                     \
      (end)                                        \
   })


static inline struct mulle_absolutetimerange
   mulle_absolutetimerange_make( mulle_absolutetime_t start,
                                 mulle_absolutetime_t end)
{
    struct mulle_absolutetimerange result = { start, end };
    return result;
}


static inline void
   _mulle_absolutetimerange_init( struct mulle_absolutetimerange *p,
                                  mulle_absolutetime_t start,
                                  mulle_absolutetime_t end)
{
   p->start = start;
   p->end   = end;
}


static inline void
   mulle_absolutetimerange_init( struct mulle_absolutetimerange *p,
                                 mulle_absolutetime_t start,
                                 mulle_absolutetime_t end)
{
   if( p)
      _mulle_absolutetimerange_init( p, start, end);
}


static inline mulle_absolutetime_t
   mulle_absolutetime_init_with_timespec( struct timespec a)
{
   return( a.tv_sec + a.tv_nsec / (double) (1000L*1000*1000));
}


static inline mulle_absolutetime_t
   mulle_absolutetime_init_with_s_ns( int tv_sec, long tv_nsec)
{
   return( tv_sec + tv_nsec / (double) (1000L*1000*1000));
}



#endif
