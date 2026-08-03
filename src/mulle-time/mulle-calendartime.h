//
//  mulle-calendartime.h
//  mulle-time
//
//  Copyright (c) 2022 Nat! - Mulle kybernetiK.
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
#ifndef mulle_calendartime_h__
#define mulle_calendartime_h__


#include "mulle-timetype.h"


// mulle_calendartime_t is a FP number containing seconds as a timestamp
// relative to a reference date which is defined in mulle-time.h.
// But it is still an absolute time.
// It "jumps" when there  are changes in the GMT calendar. 
//
// Arithmetic on mulle_calendartime_t and mulle_relativetime_t has six useful
// operations
//
// 1. mulle_calendartime_t + mulle_calendartime_t = invalid
// 2. mulle_calendartime_t + mulle_relativetime_t = mulle_calendartime_t
// 3. mulle_relativetime_t + mulle_calendartime_t = mulle_calendartime_t
// 4. mulle_relativetime_t + mulle_relativetime_t = mulle_relativetime_t
//
// 5. mulle_calendartime_t - mulle_calendartime_t = mulle_relativetime_t
// 6. mulle_calendartime_t - mulle_relativetime_t = mulle_calendartime_t
// 7. mulle_relativetime_t - mulle_calendartime_t = invalid
// 8. mulle_relativetime_t - mulle_relativetime_t = mulle_relativetime_t
//
// You must not mix mulle_calendartime_t and mulle_absolutetime_t in 
// arithmetic.

typedef mulle_timeinterval_t   mulle_calendartime_t;


static inline mulle_calendartime_t   mulle_calendartime_now( void)
{
   return( (mulle_calendartime_t) mulle_timeinterval_now());
}


static inline void
   _mulle_calendartime_init( mulle_calendartime_t *p, mulle_timeinterval_t value)
{
   *p = value;
}


static inline void
   mulle_calendartime_init( mulle_calendartime_t *p, mulle_timeinterval_t value)
{
   if( p)
      _mulle_calendartime_init( p, value);
}



static inline mulle_calendartime_t
   mulle_calendartime_make( mulle_timeinterval_t value)
{
   return( (mulle_calendartime_t) value);
}

//
// struct mulle_calendartimerange is a... range of time with a start and an end.
// The end is thought of as being inclusive.
//
struct mulle_calendartimerange
{
   mulle_calendartime_t   start;
   mulle_calendartime_t   end;
};


#define MULLE_CALENDARTIMERANGE_DATA( start, end)  \
   ((struct mulle_calendartimerange)               \
   {                                               \
      (start),                                     \
      (end)                                        \
   })

static inline struct mulle_calendartimerange
   mulle_calendartimerange_make( mulle_calendartime_t start,
                                 mulle_calendartime_t end)
{
    struct mulle_calendartimerange result = { start, end };
    return( result);
}


static inline void
   _mulle_calendartimerange_init( struct mulle_calendartimerange *p,
                                  mulle_calendartime_t start,
                                  mulle_calendartime_t end)
{
   p->start = start;
   p->end   = end;
}


static inline void
   mulle_calendartimerange_init( struct mulle_calendartimerange *p,
                                 mulle_calendartime_t start,
                                 mulle_calendartime_t end)
{
   if( p)
      _mulle_calendartimerange_init( p, start, end);
}


#endif
