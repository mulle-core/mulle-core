#ifndef mulle_relativetime_h__
#define mulle_relativetime_h__

#include "mulle-timetype.h"


typedef mulle_timeinterval_t   mulle_relativetime_t;


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
   mulle_relativetime_init_with_s_ns( int tv_sec, long tv_nsec)
{
   return( tv_sec + tv_nsec / (double) (1000L*1000*1000));
}


// this gives elapsed seconds since the load of the program
MULLE__TIME_GLOBAL
mulle_relativetime_t   mulle_relativetime_now( void);




struct mulle_relativetimerange
{
   mulle_relativetime_t   delay;
   mulle_relativetime_t   duration;
};


#define MULLE_RELATIVETIMERANGE_INIT( delay, duration)  \
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
