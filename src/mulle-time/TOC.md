# mulle-time Library Documentation for AI
<!-- Keywords: time, timespec, timeval, interval, absolute, relative, calendar -->
## 1. Introduction & Purpose

- mulle-time provides small, portable time types and helper operations around POSIX timespec/timeval and a unified double-based timeinterval type (mulle_timeinterval_t).
- Solves: simple arithmetic (add/sub/compare), conversions between representations, monotonic vs. calendar times, and small typed aliases for clarity (absolute, relative, calendar).
- Key features: timespec/timeval arithmetic, now() for monotonic/real time, typed wrappers (mulle_absolutetime_t, mulle_calendartime_t, mulle_relativetime_t), ranges and small helpers.
- Relationship: component of mulle-core; expects mulle-c11 and mulle-sde build tooling.

## 2. Key Concepts & Design Philosophy

- Lightweight, header-only helpers and small inline functions where possible for zero-overhead.
- Distinct semantic types for different time domains: absolute (monotonic), calendar (wall-clock), relative (durations). Prevents accidental mixing.
- Prefer struct timespec for precision; timeval supported as fallback.
- Inline operations (add/sub/compare) return canonical normalized results (nanosecond/microsecond carry handling).
- Minimal dependencies and C89-compatible style; exposes C linkage macros (MULLE__TIME_GLOBAL).

## 3. Core API & Data Structures

### 3.1. src/mulle-timetype.h
- typedefs:
  - mulle_timeinterval_t: double representing seconds (can be absolute or relative).
  - mulle_time_comparison_t: enum { MulleTimeAscending, MulleTimeSame, MulleTimeDescending } for compare results.
- Constants: MULLE_TIMEINTERVAL_SINCE_1970, MULLE_TIMEINTERVAL_DISTANT_FUTURE/PAST.
- Utilities:
  - mulle_timeinterval_add(a,b), mulle_timeinterval_subtract(a,b)
  - mulle_timeinterval_mod(value,m)
  - mulle_timeinterval_quantize(value,rate)
- Data struct: struct mulle_timeintervalrange { start, end } + constructor mulle_timeintervalrange_make

### 3.2. src/mulle-timespec.h
- Purpose: helpers for struct timespec and sleeping/now functions.
- Functions (global):
  - mulle_timeinterval_now(void): wall-clock time (seconds as double)
  - mulle_timeinterval_now_monotonic(void): monotonic time for animation/timers
  - mulle_relativetime_sleep(mulle_relativetime_t time)
- Inline operations on struct timespec:
  - timespec_compare(a,b) -> mulle_time_comparison_t
  - timespec_add(a,b) -> struct timespec (normalizes nsec carry)
  - timespec_sub(a,b) -> struct timespec
  - timespec_make_with_relativetime(time)
  - deprecated: mulle_relativetime_get_timespec
  - mulle_relativetime_make_with_timespec(struct timespec)

### 3.3. src/mulle-timeval.h
- Purpose: timeval arithmetic (fallback for platforms without timespec).
- Inline helpers:
  - timeval_compare(a,b)
  - timeval_add(a,b)
  - timeval_sub(a,b)

### 3.4. src/mulle-calendartime.h
- Type: typedef mulle_calendartime_t (alias of mulle_timeinterval_t) for wall-clock/calendar timestamps.
- Functions and helpers:
  - mulle_calendartime_now(void)
  - mulle_calendartime_init/_mulle_calendartime_init, mulle_calendartime_make
  - struct mulle_calendartimerange, helper macros and make/init functions
- Semantics: calendar times may "jump" with timezone/clock changes — do not mix with absolutetime.

### 3.5. src/mulle-absolutetime.h
- Type: typedef mulle_absolutetime_t (alias of mulle_timeinterval_t) for monotonic timestamps (since boot).
- Functions:
  - mulle_absolutetime_now(void)
  - init/make helpers and range struct (mulle_absolutetimerange)
  - conversion helpers from struct timespec: mulle_absolutetime_init_with_timespec, _with_s_ns

### 3.6. src/mulle-relativetime.h
- Type: typedef mulle_relativetime_t (alias of mulle_timeinterval_t) for durations/delays.
- Functions:
  - mulle_relativetime_now(void): elapsed seconds since program load
  - init/make helpers: mulle_relativetime_init/_mulle_relativetime_init, mulle_relativetime_make_with_s_ns
  - struct mulle_relativetimerange and helpers

## 4. Performance Characteristics

- Core operations are inline, O(1) cost (addition, subtraction, compare) with minimal branching.
- Conversions (timespec/timeval <-> double) are constant-time arithmetic.
- No dynamic allocation; memory overhead negligible.
- Not thread-synchronized: callers must coordinate if sharing mutable range structs. The now() functions call system timers and are thread-safe in typical libc implementations.

## 5. AI Usage Recommendations & Patterns

- Prefer using typed aliases (mulle_absolutetime_t, mulle_calendartime_t, mulle_relativetime_t) to document intent and avoid mixing domains.
- Use timespec helpers when sub-second precision is needed; use timespec_add/sub/compare for interval math.
- Use mulle_timeinterval_now_monotonic() for animation/timers; use mulle_calendartime_now() for wall-clock timestamps.
- Best practices:
  - Always use the provided init/make functions to construct typed values.
  - Treat returned struct pointers from system calls as ephemeral; copy into typed double if you need to store long-term.
- Pitfalls:
  - Do not mix calendar and absolute times in arithmetic — results are undefined.
  - Beware of deprecated helpers (mulle_relativetime_get_timespec) — prefer current names.

## 6. Integration Examples

### Example 1: Creating and comparing times (monotonic)

```c
#include "mulle-timespec.h"
#include <stdio.h>

int
main()
{
   struct timespec  a;
   struct timespec  b;
   struct timespec  sum;

   a = timespec_make_with_relativetime( 1.5);
   b = timespec_make_with_relativetime( 0.75);
   sum = timespec_add( a, b);

   if( timespec_compare( sum, a) == MulleTimeDescending)
   {
      printf( "sum > a\n");
   }
   return( 0);
}
```

### Example 2: Use monotonic now for animation timing

```c
#include "mulle-timespec.h"
#include "mulle-absolutetime.h"
#include <stdio.h>

int
main()
{
   mulle_absolutetime_t  t0;
   mulle_absolutetime_t  t1;

   t0 = mulle_absolutetime_now();
   /* do some work */
   t1 = mulle_absolutetime_now();

   printf( "elapsed: %f seconds\n", (double) (t1 - t0));
   return( 0);
}
```

### Example 3: Calendar time and ranges

```c
#include "mulle-calendartime.h"
#include <stdio.h>

int
main()
{
   mulle_calendartime_t   now;
   struct mulle_calendartimerange  r;

   now = mulle_calendartime_now();
   r = mulle_calendartimerange_make( now, now + 3600.0);

   printf( "now: %f, end: %f\n", now, r.end);
   return( 0);
}
```

## 7. Dependencies

- mulle-c11
- mulle-core (this component is part of the mulle-core collection)
- build tooling: mulle-sde (development), clib (optional for source-only install)

## 8. Shortcut

- If a prior TOC.md exists, inspect its commit timestamp in git to generate diffs. This TOC was generated from headers in src/ and examples in test/ on commit HEAD.

--
Generated by an AI assistant reading public headers and README.md; primary sources: src/*.h and test/ examples.
