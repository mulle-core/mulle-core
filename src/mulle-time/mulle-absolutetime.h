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


//
// struct mulle_absolutetimerange is a... range of time with a start and an end.
// The end is thought of as being inclusive.
//
struct mulle_absolutetimerange
{
   mulle_absolutetime_t   start;
   mulle_absolutetime_t   end;
};


#define MULLE_ABSOLUTETIMERANGE_INIT( start, end)  \
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
