# mulle-time Library Documentation for AI
<!-- Keywords: time, timespec, timeval, interval, absolute, relative, calendar -->
## 1. Introduction & Purpose

- mulle-time provides small, portable time types and helper operations around POSIX `struct timespec`/`struct timeval`, plus a unified `double`-based interval type (`mulle_timeinterval_t`) for arithmetic and conversions.
- Solves: interval math (add/sub/compare), conversion between `double` seconds and canonical `struct timespec`, monotonic vs. wall-clock `now()` queries, sleeping, and domain separation between absolute, calendar, and relative times.
- Key features: inline `timespec`/`timeval` arithmetic with carry/borrow and canonicalization, `mulle_timeinterval_now()` / `mulle_timeinterval_now_monotonic()`, `mulle_relativetime_sleep()`, typed aliases (`mulle_absolutetime_t`, `mulle_calendartime_t`, `mulle_relativetime_t`), range structs, and value snapping (`mulle_timeinterval_quantize` / `mulle_timeinterval_mod`).
- Relationship: a component of `mulle-core`. It is a library with no runtime executable; it depends on `mulle-c11`. `mulle_timeinterval_t` is typedef'd to `NSTimeInterval` in mulle-objc.

## 2. Key Concepts & Design Philosophy

- **Unified interval representation**: `mulle_timeinterval_t` is `typedef double`, holding seconds. All typed time values are semantic aliases of this type, so the compiler cannot distinguish them — the domains are a naming convention, not separate types.
- **Three time domains** (all `double` seconds):
  - `mulle_absolutetime_t` — monotonic timestamp relative to system boot, for animation/timers. Do NOT mix with calendar time in arithmetic.
  - `mulle_calendartime_t` — wall-clock timestamp relative to the Unix epoch (1970-01-01T00:00:00Z); may "jump" with clock/timezone changes.
  - `mulle_relativetime_t` — a duration/delay (e.g. 10s). Negative values are legal; the API never rejects them.
- **The six useful operations**: typed values support a fixed legal arithmetic matrix (documented in each header), e.g. `absolutetime - absolutetime = relativetime`, `absolutetime + relativetime = absolutetime`; adding two absolute times is invalid and mixing `mulle_calendartime_t` and `mulle_absolutetime_t` is undefined.
- **Canonical inputs**: all `timespec`/`timeval` arithmetic and comparison helpers require canonical inputs (`tv_nsec` in `[0, 1e9)`; `tv_usec` in `[0, 1e6)`). This is enforced with `assert()` in debug builds; release builds (`NDEBUG`) assume canonical inputs without checking.
- **Failure policy for system clocks**: if a clock/sleep syscall fails, `now`-style functions return `0.0` (which is also a plausible valid reading; the failure is indistinguishable at the call site) and sleeps return early. No error propagation through return values.
- **Precision caveat**: `double` seconds lose sub-microsecond precision at large magnitudes; prefer `struct timespec` for storage and ordering where nanoseconds matter at large values.
- **Header-only by design**: most of the API is `static inline` in public headers; only `mulle_timeinterval_now`, `mulle_timeinterval_now_monotonic`, `mulle_relativetime_sleep`, and `mulle_relativetime_now` are compiled functions in `mulle-timespec.c` / `mulle-relativetime.c`.
- **Feature-test macro handling**: on glibc, the inline helpers need `CLOCK_*` constants, so `mulle-timetype.h` defines `_GNU_SOURCE` only if glibc's feature detection has NOT run yet (`_FEATURES_H`/`_POSIX_C_SOURCE` undefined). Consumers that include other system headers before `mulle-time.h` must define `_GNU_SOURCE` (or equivalent) before their first system header.

## 3. Core API & Data Structures

All public headers live in `src/` and are aggregated by the umbrella header `src/mulle-time.h`, which includes (on non-Windows) timespec, timeval, absolutetime, calendartime, and relativetime. Version: `MULLE__TIME_VERSION ((1UL << 20) | (3 << 8) | 2)`.

### 3.1. `src/mulle-timetype.h`

- **Purpose:** Core types, constants, and interval arithmetic; the base of everything else.

#### `typedef double mulle_timeinterval_t`
- Absolute or relative time stored as seconds in a `double`.

#### `typedef enum { MulleTimeAscending = -1, MulleTimeSame = 0, MulleTimeDescending = 1 } mulle_time_comparison_t`
- Comparison result. `MulleTimeAscending` means the first operand is smaller; `MulleTimeDescending` means it is larger.

#### Constants
- `#define MULLE_TIMEINTERVAL_SINCE_1970 978307200.0` — seconds between the Unix epoch (1970-01-01) and the Cocoa reference date (2001-01-01). `mulle_timeinterval_now` uses the Unix epoch, so this constant is only for Cocoa conversions.
- `#define MULLE_TIMEINTERVAL_DISTANT_FUTURE 63113904000.0`
- `#define MULLE_TIMEINTERVAL_DISTANT_PAST -63114076800.0`

#### Core operations (all `static inline`)
- `mulle_timeinterval_add( mulle_timeinterval_t a, mulle_timeinterval_t b)` — returns `a + b`; can produce absolute or relative time.
- `mulle_timeinterval_subtract( mulle_timeinterval_t a, mulle_timeinterval_t b)` — returns `a - b`.
- `mulle_timeinterval_is_finite( mulle_timeinterval_t value)` — returns nonzero if `value` is neither NaN nor infinite. Implemented without `math.h` (uses `value == value && value - value == 0.0`).
- `mulle_timeinterval_mod( mulle_timeinterval_t value, mulle_timeinterval_t m)` — floor-based mod, result in `[0, m)` for `m > 0`. NaN/infinite values or invalid `m` (nonpositive, NaN, infinite) return `0.0`; a quotient that would overflow `long long` also returns `0.0` (no UB).
- `mulle_timeinterval_quantize( mulle_timeinterval_t value, mulle_timeinterval_t rate)` — "snaps to rate": returns the nearest value evenly divisible by `rate` (e.g. rate 0.3: value 1.0 → 0.9, 1.1 → 1.2). The remainder is a positive distance within the rate bucket, so negative values quantize symmetrically; ties round up (away from zero). An invalid rate (nonpositive/NaN/infinite) and NaN/infinite values are returned unchanged.

#### `struct mulle_timeintervalrange`
- **Key Fields:** `mulle_timeinterval_t start;` and `mulle_timeinterval_t end;`
- **Constructor:** `mulle_timeintervalrange_make( mulle_timeinterval_t start, mulle_timeinterval_t end)`.

### 3.2. `src/mulle-timespec.h`

- **Purpose:** wall-clock/monotonic `now()` queries, sleeping, and `struct timespec` arithmetic and conversion helpers (the preferred representation where available).

#### Compiled functions (global, `MULLE__TIME_GLOBAL`)
- `mulle_timeinterval_t mulle_timeinterval_now( void);` — wall-clock time as seconds since the Unix epoch. On POSIX this is `CLOCK_REALTIME`; a failed clock call returns `0.0`.
- `mulle_timeinterval_t mulle_timeinterval_now_monotonic( void);` — monotonic time for animations/timewatch. On POSIX this is `CLOCK_MONOTONIC`; on Windows `QueryPerformanceCounter`. Same failure policy: returns `0.0`.
- `void mulle_relativetime_sleep( mulle_relativetime_t time);` — best-effort sleep. Returns immediately for `time <= 0.0`. POSIX: retries `nanosleep` on `EINTR` until the duration fully elapses; non-EINTR failures return early. Windows: uses a high-resolution waitable timer when available, falling back to chunked millisecond `Sleep()` (chunked to avoid overflowing the DWORD argument for very long delays; NaN/infinity also fall back to the chunked loop).

#### Inline `struct timespec` helpers
- `timespec_compare( struct timespec a, struct timespec b)` → `mulle_time_comparison_t`. Requires canonical inputs (`tv_nsec` in `[0, 1e9)`), asserted in debug builds.
- `timespec_add( struct timespec a, struct timespec b)` → `struct timespec`. Normalizes nanosecond carry; canonical output.
- `timespec_sub( struct timespec a, struct timespec b)` → `struct timespec`. Handles borrow; canonical output. Result may be negative (canonical form, e.g. `-1.25 = {sec=-2, nsec=750000000}`).
- `timespec_make_with_relativetime( mulle_relativetime_t time)` → `struct timespec`. Converts a `double` interval to a canonical timespec (floor semantics for `tv_sec`). NaN, infinities, or values outside the `long long` range become a zero timespec; seconds not representable in `time_t` (e.g. 32-bit `time_t`, post-2038) also become a zero timespec rather than silently truncating; rounding that would produce `tv_nsec == 1e9` is normalized by carrying into `tv_sec`.
- `mulle_relativetime_get_timespec( mulle_relativetime_t time)` → `struct timespec` — **deprecated alias** of `timespec_make_with_relativetime`.
- `mulle_relativetime_make_with_timespec( struct timespec a)` → `mulle_relativetime_t`. Converts a canonical timespec to `double` seconds, using `a.tv_sec` and `a.tv_nsec`.

### 3.3. `src/mulle-timeval.h`

- **Purpose:** `struct timeval` arithmetic as a fallback for platforms without `struct timespec`.

#### Inline helpers (same arithmetic guarantees as the timespec ones, with `tv_usec` canonical in `[0, 1e6)`, asserted in debug builds)
- `timeval_compare( struct timeval a, struct timeval b)` → `mulle_time_comparison_t`
- `timeval_add( struct timeval a, struct timeval b)` → `struct timeval` (handles microsecond carry)
- `timeval_sub( struct timeval a, struct timeval b)` → `struct timeval` (handles borrow)

Note: `mulle_time_now()` exists in the header but is compiled out (`#if 0`) — do not use it.

### 3.4. `src/mulle-calendartime.h`

- **Type:** `typedef mulle_timeinterval_t mulle_calendartime_t;` — wall-clock/calendar timestamps (seconds since the Unix epoch). "Jumps" on clock/timezone changes.

#### Functions (all `static inline`)
- `mulle_calendartime_now( void)` → `mulle_calendartime_t` (wraps `mulle_timeinterval_now`).
- `_mulle_calendartime_init( mulle_calendartime_t *p, mulle_timeinterval_t value)` / `mulle_calendartime_init( ...)` (the latter NULL-checks `p`).
- `mulle_calendartime_make( mulle_timeinterval_t value)` → `mulle_calendartime_t`.

#### `struct mulle_calendartimerange`
- **Key Fields:** `mulle_calendartime_t start;`, `mulle_calendartime_t end;` (end is inclusive).
- **Constructor macros/functions:** `#define MULLE_CALENDARTIMERANGE_DATA( start, end)`, `mulle_calendartimerange_make( mulle_calendartime_t start, mulle_calendartime_t end)`, `_mulle_calendartimerange_init( struct mulle_calendartimerange *p, ...)`, `mulle_calendartimerange_init( ...)`.

### 3.5. `src/mulle-absolutetime.h`

- **Type:** `typedef mulle_timeinterval_t mulle_absolutetime_t;` — monotonic timestamp relative to system boot (like a diff on `uptime`). Suspend semantics: monotonic while the process runs; POSIX `CLOCK_MONOTONIC` typically excludes suspend time, Windows `QueryPerformanceCounter` is hardware-dependent — do not rely on it across a suspend/resume.

#### Functions (all `static inline`)
- `mulle_absolutetime_now( void)` → `mulle_absolutetime_t` (wraps `mulle_timeinterval_now_monotonic`).
- `_mulle_absolutetime_init( mulle_absolutetime_t *p, mulle_timeinterval_t value)` / `mulle_absolutetime_init( ...)`.
- `mulle_absolutetime_make( mulle_timeinterval_t value)` → `mulle_absolutetime_t`.
- Construction from a timespec: `mulle_absolutetime_init_with_timespec( struct timespec a)` → `mulle_absolutetime_t`; `mulle_absolutetime_init_with_s_ns( time_t tv_sec, long tv_nsec)` → `mulle_absolutetime_t`.

#### `struct mulle_absolutetimerange`
- **Key Fields:** `mulle_absolutetime_t start;`, `mulle_absolutetime_t end;` (end is inclusive).
- **Constructor macros/functions:** `#define MULLE_ABSOLUTETIMERANGE_DATA( start, end)`, `mulle_absolutetimerange_make( mulle_absolutetime_t start, mulle_absolutetime_t end)`, `_mulle_absolutetimerange_init( struct mulle_absolutetimerange *p, ...)`, `mulle_absolutetimerange_init( ...)`.

### 3.6. `src/mulle-relativetime.h`

- **Type:** `typedef mulle_timeinterval_t mulle_relativetime_t;` — durations/delays.

#### Functions
- `mulle_relativetime_t mulle_relativetime_now( void);` — compiled (`MULLE__TIME_GLOBAL`); elapsed seconds since the load of the program (via a `MULLE_C_CONSTRUCTOR` capture of `mulle_absolutetime_now`). Same failure policy as `mulle_timeinterval_now_monotonic`: a failed clock call returns `0.0`.
- `_mulle_relativetime_init( mulle_relativetime_t *p, mulle_timeinterval_t value)` / `mulle_relativetime_init( ...)` — `static inline`.
- `mulle_relativetime_make_with_s_ns( time_t tv_sec, long tv_nsec)` → `mulle_relativetime_t` — build from seconds/nanoseconds in a single `double` second value.
- `mulle_relativetime_init_with_s_ns( time_t tv_sec, long tv_nsec)` → `mulle_relativetime_t` — **deprecated naming** (identical behavior).

#### `struct mulle_relativetimerange`
- **Key Fields:** `mulle_relativetime_t delay;` and `mulle_relativetime_t duration;` (note: named `delay`/`duration`, not `start`/`end`).
- **Constructor macros/functions:** `#define MULLE_RELATIVETIMERANGE_DATA( delay, duration)`, `mulle_relativetimerange_make( mulle_relativetime_t delay, mulle_relativetime_t duration)`, `mulle_relativetimerange_init( struct mulle_relativetimerange *p, mulle_relativetime_t delay, mulle_relativetime_t duration)`.

## 4. Performance Characteristics

- All arithmetic/comparison/conversion helpers in the headers are `static inline`, O(1), with minimal branching and no dynamic allocation.
- `mulle_timeinterval_mod`, `mulle_timeinterval_quantize`, and `mulle_timeinterval_is_finite` are O(1) double arithmetic (no `fmod`/`-lm` dependency); they are built to avoid undefined behavior on NaN/infinite/overflowing inputs rather than to be maximally fast on the hot path.
- `mulle_timeinterval_now*` and `mulle_relativetime_now` are O(1) system clock reads; `mulle_relativetime_sleep` blocks for the requested duration (POSIX `nanosleep` with `EINTR` retry; Windows waitable timer or chunked `Sleep`).
- Memory footprint is negligible (no allocation; a single static `load_timestamp` in `mulle-relativetime.c`, set by a constructor).
- Thread-safety: the compiled `now()`/`sleep()` functions call thread-safe libc/system facilities. The inline arithmetic is pure (no shared state) and thread-safe on distinct data. The Windows `mulle_timeinterval_now_monotonic` uses `QueryPerformanceCounter` with no shared state, so it is inherently thread-safe. There is no internal locking; callers that share mutable range structs across threads must coordinate themselves.

## 5. AI Usage Recommendations & Patterns

- **Best Practices:**
  - Use the typed aliases (`mulle_absolutetime_t`, `mulle_calendartime_t`, `mulle_relativetime_t`) to encode domain intent and prefer their `_init`/`_make` constructors over raw `double` assignment.
  - Use `mulle_absolutetime_now()` for animation/timers and `mulle_calendartime_now()` / `mulle_timeinterval_now()` for wall-clock timestamps. Remember `mulle_relativetime_now()` is seconds since program load.
  - Keep `timespec`/`timeval` operands canonical (`tv_nsec`/`tv_usec` in range) before passing them to the arithmetic/comparison helpers, and use `timespec_make_with_relativetime` whenever you build a timespec from a `double` — it guarantees canonical output and safe handling of negative values.
  - For sub-second precision, use `timespec_add`/`timespec_sub`/`timespec_compare` for interval math; for long-lived storage or ordering at large magnitudes prefer `struct timespec` over `double`.
  - When using `< 0` or `<= 0` relative times, be deliberate: `mulle_relativetime_sleep(time <= 0)` returns immediately, and negative relative times otherwise have the meaning you give them.
- **Common Pitfalls:**
  - Do not mix `mulle_calendartime_t` (wall clock) and `mulle_absolutetime_t` (monotonic) in arithmetic — results are undefined. Their arithmetic matrices explicitly forbid certain combinations (e.g. absolute + absolute).
  - Do not rely on `mulle_absolutetime_t` counting elapsed wall-clock time across a suspend/resume cycle.
  - `now()`-family functions return `0.0` on clock failure, indistinguishable from a genuine reading — do not plausibility-check for errors.
  - `mulle_relativetime_get_timespec` and `mulle_relativetime_init_with_s_ns` are deprecated aliases; prefer `timespec_make_with_relativetime` and `mulle_relativetime_make_with_s_ns`.
  - `timespec_make_with_relativetime` returns a zero timespec for NaN/infinity/out-of-range inputs — validate if those inputs are possible in your code.
  - `mulle_relativetime_make_with_timespec( a)` uses both `a.tv_sec` and `a.tv_nsec`; a timespec converted from a value ≥ 2^53 seconds has no fractional second precision.

## 6. Integration Examples

### Example 1: timespec arithmetic and comparison

```c
#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timespec   a;
   struct timespec   b;
   struct timespec   sum;

   a   = timespec_make_with_relativetime( 1.5);
   b   = timespec_make_with_relativetime( 0.75);
   sum = timespec_add( a, b);            // result is canonical: 2.25s

   if( timespec_compare( sum, a) == MulleTimeDescending)
   {
      printf( "sum is larger: %ld.%09ld\n", (long) sum.tv_sec, (long) sum.tv_nsec);
   }
   return( 0);
}
```

### Example 2: conversion between relativetime and timespec

```c
#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   struct timespec      ts;
   mulle_relativetime_t value;

   // negative values normalize to canonical timespec: tv_nsec in [0, 1e9)
   ts = timespec_make_with_relativetime( -1.25);   // tv_sec == -2, tv_nsec == 750000000

   value = mulle_relativetime_make_with_timespec( ts);   // back to -1.25
   printf( "%lld.%09ld -> %.9f\n", (long long) ts.tv_sec, (long) ts.tv_nsec, value);

   // invalid inputs become a zero timespec instead of undefined behavior
   ts = timespec_make_with_relativetime( 1.0 / 0.0);     // +inf
   printf( "inf -> %lld.%09ld\n", (long long) ts.tv_sec, (long) ts.tv_nsec);
   return( 0);
}
```

### Example 3: quantizing and modulo of intervals

```c
#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   mulle_timeinterval_t   value;
   mulle_timeinterval_t   quantized;
   mulle_timeinterval_t   loss;

   value     = 1.1;
   quantized = mulle_timeinterval_quantize( value, 0.3);   // snaps to 1.2
   loss      = mulle_timeinterval_mod( value, 0.3);        // 0.2, in [0, 0.3)

   printf( "quantized: %f, loss: %f\n", quantized, loss);
   return( 0);
}
```

### Example 4: monotonic timing and sleeping

```c
#include <mulle-time/mulle-time.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   mulle_absolutetime_t   start;
   mulle_absolutetime_t   stop;

   // monotonic clock: immune to wall-clock changes, safe against clock jumping
   start = mulle_absolutetime_now();
   mulle_relativetime_sleep( 0.05);     // sleeps ~50ms; <= 0 returns immediately
   stop  = mulle_absolutetime_now();

   printf( "elapsed: %f seconds\n", stop - start);
   return( 0);
}
```

## 7. Dependencies

- Direct `mulle-sde` dependency (from `.mulle/etc/sourcetree/config` and `clib.json`):
  - `mulle-c11` (cross-platform C compiler glue and C preprocessor conveniences)
- `mulle-time` is itself a component of the `mulle-core` collection; it does not depend on other mulle-core members. Optional tooling: `mulle-sde` (build/dev), `clib` (source-only install).

## 8. Shortcut

- If a prior `index.md` exists, inspect its last commit timestamp in git and consider only the changes since that commit. This revision was updated against the current `HEAD` (f7b0299 added the license headers and moved this file; the substantive API changes since then are in commit 8a5010b "fix: harden time arithmetic and fix timespec conversion bugs" and the new tests `13_conversion`, `14_quantize`, `15_arith`, `16_sleep`, `17_now`).

---
Generated by an AI assistant reading public headers (`src/*.h`), the README, and tests (`test/`). Signatures are copied verbatim from the headers.