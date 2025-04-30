#define _GNU_SOURCE

#include "mulle-timetype.h"

#include "mulle-timespec.h"

#include <errno.h>
#include <assert.h>

//
// this could be an inline function as well, and the whole library a header
// but due to the retarded way linux headers are super-restrictive by default
// we MUST define something like _GNU_SOURCE to get the CLOCK_REALTIME
// constant.
//
// Forcing everyone else to set _GNU_SOURCE who wants to include mulle-time.h
// is misery.
//
#ifdef _WIN32

#include <windows.h>

// https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows

mulle_timeinterval_t   mulle_timeinterval_now( void)
{  
	__int64   wintime; 

	GetSystemTimeAsFileTime((FILETIME*)&wintime);
   wintime -= (__int64) 116444736000000000LL;  //1jan1601 to 1jan1970
   return( (mulle_timeinterval_t) wintime / 10000000.0);
}


mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void)
{
   __int64   wintime;
   static __int64   old_wintime;

   GetSystemTimeAsFileTime((FILETIME*)&wintime);
   // TODO: needs to be atomic probably
   if( ! old_wintime)
      old_wintime = wintime;

   return( (mulle_timeinterval_t) (wintime - old_wintime) / 10000000.0);
}

#else

mulle_timeinterval_t   mulle_timeinterval_now( void)
{
   struct timespec   now;

   clock_gettime( CLOCK_REALTIME, &now);
   return( now.tv_sec + now.tv_nsec / (1000.0*1000*1000));
}


mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void)
{
   struct timespec   now;

   clock_gettime( CLOCK_MONOTONIC, &now);
   return( now.tv_sec + now.tv_nsec / (1000.0*1000*1000));
}


//
// maybe adopt this, if we run into problems with nanosleep
//
// void sleep_ms(int milliseconds)
// {
//     #ifdef WIN32
//         Sleep(milliseconds); // Suspends the execution of the current thread until the time-out interval elapses.
//     #elif _POSIX_C_SOURCE >= 199309L
//         struct timespec ts;
//         ts.tv_sec = milliseconds / 1000;
//         ts.tv_nsec = (milliseconds % 1000) * 1000000;
//         nanosleep(&ts, NULL); // nanosleep() suspends the execution of the calling thread
//     #else
//         usleep(milliseconds * 1000); // The usleep() function suspends execution of the calling thread
//     #endif
// }

void   mulle_relativetime_sleep( mulle_relativetime_t time)
{
   struct timespec   delay;
   struct timespec   remain;

   if( time <= 0.0)
      return;

   delay = mulle_relativetime_get_timespec( time);

restart:
   if( nanosleep( &delay, &remain))
   {
      assert( errno == EINTR);
      if( errno == EINTR && (remain.tv_sec > 0 || remain.tv_nsec > 500))
      {
         delay = remain;
         goto restart;
      }
   }
}

#endif

