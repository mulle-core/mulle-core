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


//
// struct mulle_calendartimerange is a... range of time with a start and an end.
// The end is thought of as being inclusive.
//
struct mulle_calendartimerange
{
   mulle_calendartime_t   start;
   mulle_calendartime_t   end;
};


#define MULLE_CALENDARTIMERANGE_INIT( start, end)  \
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
    return result;
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
