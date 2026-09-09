//
//  mulle-timespec.c
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
#define _GNU_SOURCE

#include "mulle-timetype.h"

#include "mulle-timespec.h"

#include <errno.h>

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


mulle_timeinterval_t   mulle_timeinterval_now( void)
{
   FILETIME        filetime;
   ULARGE_INTEGER  wintime;

   GetSystemTimeAsFileTime( &filetime);
   wintime.LowPart  = filetime.dwLowDateTime;
   wintime.HighPart = filetime.dwHighDateTime;
   wintime.QuadPart -= 116444736000000000LL;  // 1jan1601 to 1jan1970
   return( (mulle_timeinterval_t) wintime.QuadPart / 10000000.0);
}


// QueryPerformanceCounter is the recommended high-resolution monotonic clock
// on Windows. Its origin is a fixed arbitrary point (typically system boot),
// it is not affected by wall-clock changes, and it does not require any
// shared state, so it is inherently thread-safe.
mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void)
{
   LARGE_INTEGER   counter;
   LARGE_INTEGER   frequency;

   if( ! QueryPerformanceFrequency( &frequency))
      return( 0.0);
   if( ! QueryPerformanceCounter( &counter))
      return( 0.0);
   if( frequency.QuadPart == 0)
      return( 0.0);
   return( (mulle_timeinterval_t) counter.QuadPart / (mulle_timeinterval_t) frequency.QuadPart);
}


// fallback to Sleep(), chunked to avoid overflowing the DWORD millisecond
// argument for very long delays
static void   mulle_sleep_ms( mulle_relativetime_t time)
{
   DWORD   max;
   DWORD   ms;

   max = (DWORD) -1;   // 0xFFFFFFFF, about 49.7 days

   while( time > 0.0)
   {
      if( time >= (mulle_relativetime_t) max / 1000.0)
         ms = max;
      else
         ms = (DWORD)( time * 1000.0 + 0.999);
      Sleep( ms);
      time -= (mulle_relativetime_t) ms / 1000.0;
   }
}


void   mulle_relativetime_sleep( mulle_relativetime_t time)
{
   HANDLE          timer;
   LARGE_INTEGER   li;
   LONGLONG        due_time_100ns;
   ULONG           flags;

   if( time <= 0.0)
      return;

   // Reject NaN, infinity and values whose 100 ns representation would not
   // fit into LONGLONG (about 29247 years): converting those to LONGLONG
   // would be undefined behavior. Fall back to the chunked Sleep() loop.
   if( ! (time == time) || time >= 9223372036854775808.0 / 10000000.0)
   {
      mulle_sleep_ms( time);
      return;
   }

   due_time_100ns = (LONGLONG)( time * 10000000.0);  // 1 second = 10,000,000 × 100 ns
   if( due_time_100ns <= 0)
      return;

   // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION needs the Windows 10 2004+ SDK
   // and OS; without it (or if creation fails) we fall back to Sleep()
#if defined( CREATE_WAITABLE_TIMER_HIGH_RESOLUTION)
   flags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
#else
   flags = 0;
#endif

   timer = CreateWaitableTimerExW( NULL, NULL, flags, TIMER_ALL_ACCESS);
   if( timer == NULL)
   {
      mulle_sleep_ms( time);
      return;
   }

   li.QuadPart = -due_time_100ns;   // negative value means "relative to now"
   if( ! SetWaitableTimer( timer, &li, 0, NULL, NULL, FALSE))
   {
      CloseHandle( timer);
      mulle_sleep_ms( time);
      return;
   }

   WaitForSingleObject( timer, INFINITE);
   CloseHandle( timer);
}

#else

// CLOCK_REALTIME and CLOCK_MONOTONIC are required by POSIX.1-2001 and
// available on all supported platforms. If a clock call still fails, the
// result is defined as 0.0 rather than an uninitialized value.
static mulle_timeinterval_t   mulle_timeinterval_now_with_clock( clockid_t clock)
{
   struct timespec   now;

   if( clock_gettime( clock, &now))
      return( 0.0);
   return( now.tv_sec + now.tv_nsec / (1000.0 * 1000 * 1000));
}


mulle_timeinterval_t   mulle_timeinterval_now( void)
{
   return( mulle_timeinterval_now_with_clock( CLOCK_REALTIME));
}


mulle_timeinterval_t   mulle_timeinterval_now_monotonic( void)
{
   return( mulle_timeinterval_now_with_clock( CLOCK_MONOTONIC));
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


// Best-effort sleep: retries a nanosleep interrupted by a signal until the
// requested duration has fully elapsed. Non-EINTR failures are treated the
// same in debug and release builds: the function simply returns early.
void   mulle_relativetime_sleep( mulle_relativetime_t time)
{
   struct timespec   delay;
   struct timespec   remain;

   if( time <= 0.0)
      return;

   delay = mulle_relativetime_get_timespec( time);

   while( nanosleep( &delay, &remain) && errno == EINTR)
      delay = remain;
}

#endif
