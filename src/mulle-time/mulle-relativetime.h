//
//  mulle-relativetime.h
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
#ifndef mulle_relativetime_h__
#define mulle_relativetime_h__

#include "mulle-timetype.h"


typedef mulle_timeinterval_t   mulle_relativetime_t;


// this gives elapsed seconds since the load of the program
MULLE__TIME_GLOBAL
mulle_relativetime_t   mulle_relativetime_now( void);


static inline void   _mulle_relativetime_init( mulle_relativetime_t *p, 
                                               mulle_timeinterval_t value)
{
   *p = value;
}


static inline void   mulle_relativetime_init( mulle_relativetime_t *p, 
                                              mulle_timeinterval_t value)
{
   if( p)
      _mulle_relativetime_init( p, value);
}


static inline mulle_relativetime_t
   mulle_relativetime_make_with_s_ns( int tv_sec, long tv_nsec)
{
   return( tv_sec + tv_nsec / (double) (1000L*1000*1000));
}


// deprecated naming
static inline mulle_relativetime_t
   mulle_relativetime_init_with_s_ns( int tv_sec, long tv_nsec)
{
   return( tv_sec + tv_nsec / (double) (1000L*1000*1000));
}



struct mulle_relativetimerange
{
   mulle_relativetime_t   delay;
   mulle_relativetime_t   duration;
};


#define MULLE_RELATIVETIMERANGE_DATA( delay, duration)  \
   ((struct mulle_relativetimerange)                    \
   {                                                    \
      (delay),                                          \
      (duration)                                        \
   })


static inline struct mulle_relativetimerange
   mulle_relativetimerange_make( mulle_relativetime_t delay,
                                 mulle_relativetime_t duration)
{
    struct mulle_relativetimerange result = { delay, duration };
    return( result);
}


static inline void
   mulle_relativetimerange_init( struct mulle_relativetimerange *p,
                                 mulle_relativetime_t delay,
                                 mulle_relativetime_t duration)
{
   if( p)
   {
      p->delay    = delay;
      p->duration = duration;
   }
}


#endif
