# mulle-time Library Documentation for AI
<!-- Keywords: time, timespec -->

## 1. Introduction & Purpose

mulle-time provides simple, high-precision time types and arithmetic operations on timespec and timeval structures. It defines `timespec_add`, `timespec_sub`, and `timespec_compare` functions, as well as their timeval counterparts. The library introduces semantic type aliases (`mulle_absolutetime_t`, `mulle_calendartime_t`, `mulle_relativetime_t`) built on top of `mulle_timeinterval_t` (a double) to make code more readable and prevent mixing incompatible time types. It serves as the foundation for NSTimeInterval compatibility in mulle-objc.

## 2. Key Concepts & Design Philosophy

- **Semantic Type Safety**: Uses typedef'd doubles with distinct names (absolutetime, calendartime, relativetime) to prevent mixing incompatible time semantics at compile time through documentation and naming conventions
- **Arithmetic Rules**: Defines strict rules for valid operations:
  - `absolutetime + relativetime = absolutetime` (adding delay to a moment)
  - `absolutetime - absolutetime = relativetime` (computing elapsed time)
  - `relativetime + relativetime = relativetime` (combining delays)
  - Invalid operations are documented to prevent misuse
- **Monotonic vs. Wall-Clock Time**: Distinguishes between `mulle_timeinterval_now()` (wall-clock, may jump) and `mulle_timeinterval_now_monotonic()` (monotonic, suitable for animations and timers)
- **Precision**: Prefers `struct timespec` (nanosecond precision) over `timeval` (microsecond precision) where available
- **Minimal Dependencies**: No floating-point math library requirement (implements mod without -lm)

## 3. Core API & Data Structures

### mulle-timespec.h

#### struct timespec
- **Purpose**: POSIX time structure with seconds and nanosecond components
- **Fields**: `tv_sec` (seconds), `tv_nsec` (nanoseconds, 0-999,999,999)
- **Lifecycle**: No creation/destruction needed; inline initialization via make functions

#### Core Comparison & Arithmetic Functions

- `timespec_compare(a, b)` → `mulle_time_comparison_t`: Returns MulleTimeAscending, MulleTimeSame, or MulleTimeDescending
- `timespec_add(a, b)` → `struct timespec`: Adds two timespec values with proper carry handling
- `timespec_sub(a, b)` → `struct timespec`: Subtracts b from a with proper borrow handling
- `timespec_make_with_relativetime(time)` → `struct timespec`: Converts mulle_relativetime_t to timespec

#### Helper Functions

- `mulle_relativetime_get_timespec(time)`: Deprecated alias for timespec_make_with_relativetime
- `mulle_relativetime_sleep(time)`: Sleeps for specified relative time duration
- `mulle_relativetime_make_with_timespec(a)`: Converts timespec to mulle_relativetime_t

### mulle-timetype.h

#### mulle_timeinterval_t (typedef double)
- **Purpose**: Primary time storage type; can represent both absolute and relative times
- **Constants**:
  - `MULLE_TIMEINTERVAL_SINCE_1970`: 978307200.0 (reference point)
  - `MULLE_TIMEINTERVAL_DISTANT_FUTURE`: 63113904000.0
  - `MULLE_TIMEINTERVAL_DISTANT_PAST`: -63114076800.0

#### Core Interval Arithmetic

- `mulle_timeinterval_add(a, b)` → `mulle_timeinterval_t`: Simple double addition
- `mulle_timeinterval_subtract(a, b)` → `mulle_timeinterval_t`: Simple double subtraction
- `mulle_timeinterval_mod(value, m)` → `mulle_timeinterval_t`: Modulo operation (avoids -lm dependency)
- `mulle_timeinterval_quantize(value, rate)` → `mulle_timeinterval_t`: Snaps value to nearest multiple of rate

#### mulle_timeintervalrange struct
- **Purpose**: Represents a time interval with start and end points
- **Fields**: `start`, `end` (both mulle_timeinterval_t)
- **Factory**: `mulle_timeintervalrange_make(start, end)` → `struct mulle_timeintervalrange`

#### Global Functions

- `mulle_timeinterval_now()` → `mulle_timeinterval_t`: Returns current wall-clock time (may jump)
- `mulle_timeinterval_now_monotonic()` → `mulle_timeinterval_t`: Returns monotonic time (best for animations/timers)

### mulle-absolutetime.h

#### mulle_absolutetime_t (typedef double)
- **Purpose**: Semantic wrapper for absolute (monotonic) timestamps relative to system boot
- **Lifetime**: Ideally doesn't jump when system sleeps (compare with wall-clock time)

#### Core Operations

- `mulle_absolutetime_now()` → `mulle_absolutetime_t`: Gets current absolute time (uses monotonic clock)
- `mulle_absolutetime_init(p, value)`: Safe initialization with NULL check
- `_mulle_absolutetime_init(p, value)`: Unchecked initialization
- `mulle_absolutetime_make(value)` → `mulle_absolutetime_t`: Direct casting
- `mulle_absolutetime_init_with_timespec(ts)`: Converts timespec to absolutetime
- `mulle_absolutetime_init_with_s_ns(sec, nsec)`: Creates absolutetime from components

#### mulle_absolutetimerange struct
- **Purpose**: Range with start and end absolute times
- **Fields**: `start`, `end` (both mulle_absolutetime_t)
- **Factory**: `mulle_absolutetimerange_make(start, end)`

### mulle-calendartime.h

#### mulle_calendartime_t (typedef double)
- **Purpose**: Semantic wrapper for calendar/wall-clock timestamps (may jump)
- **Usage**: For timestamps related to actual calendar dates/times, as opposed to monotonic boot time

### mulle-relativetime.h

#### mulle_relativetime_t (typedef double)
- **Purpose**: Semantic wrapper for time delays and relative durations
- **Usage**: For time intervals that represent delays, durations, or differences

#### Core Functions

- `mulle_relativetime_now()`: Relative time since some reference point
- `mulle_relativetime_sleep(time)`: Suspends execution for specified duration

## 4. Performance Characteristics

- **O(1) for all operations**: All arithmetic and comparison functions are inline operations on doubles or simple integer arithmetic
- **Memory**: Negligible; timespec/timeval are fixed-size stack structures
- **Thread-Safety**: All operations are on local variables; no shared state (thread-safe by design)
- **Precision**:
  - timespec: nanosecond precision (10^-9 seconds)
  - timeval: microsecond precision (10^-6 seconds)
  - timeinterval_t (double): ~15-17 significant decimal digits of precision
- **No External Dependencies**: Implements mod without linking to math library

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Use Semantic Types**: Prefer `mulle_absolutetime_t`, `mulle_calendartime_t`, and `mulle_relativetime_t` over bare doubles to document intent
- **Choose Correct Clock**: Use `mulle_timeinterval_now_monotonic()` for animations, timers, and performance measurements; use `mulle_timeinterval_now()` for calendar/logging timestamps
- **Carry Handling**: The timespec_add/sub functions handle carry/borrow automatically; always use these rather than manual field manipulation
- **Ranges**: When storing time intervals with start/end, use the provided range structs for clarity

### Common Pitfalls

- **Do Not Mix Time Types**: Avoid operations between calendartime and absolutetime without explicit conversion (semantically invalid)
- **Sleep Precision**: `mulle_relativetime_sleep()` depends on OS scheduler; precision may not be guaranteed below ~10ms
- **Double Precision**: While doubles work, be aware of precision loss in very large time values
- **Overflow**: timespec arithmetic doesn't guard against overflow; ensure inputs are reasonable

### Idiomatic Usage

- Use timespec for nanosleep and precise timing code
- Use timeinterval_t for high-level APIs and NSTimeInterval compatibility
- Use the semantic types (absolute/calendar/relative) in function signatures for self-documenting code
- Store durations as mulle_relativetime_t, moments as mulle_absolutetime_t

## 6. Integration Examples

### Example 1: Basic Timespec Arithmetic

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    struct timespec start, duration, end;
    
    start.tv_sec = 100;
    start.tv_nsec = 500000000;  // 100.5 seconds
    
    duration.tv_sec = 0;
    duration.tv_nsec = 250000000;  // 0.25 seconds
    
    end = timespec_add(start, duration);
    printf("End: %ld.%09ld\n", end.tv_sec, end.tv_nsec);  // 100.750000000
    
    return 0;
}
```

### Example 2: Time Comparison

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    struct timespec a, b;
    
    a.tv_sec = 100;
    a.tv_nsec = 100;
    
    b.tv_sec = 100;
    b.tv_nsec = 200;
    
    mulle_time_comparison_t result = timespec_compare(a, b);
    if (result == MulleTimeAscending)
        printf("a < b\n");
    else if (result == MulleTimeSame)
        printf("a == b\n");
    else
        printf("a > b\n");
    
    return 0;
}
```

### Example 3: Getting Current Time (Monotonic)

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    mulle_absolutetime_t start = mulle_absolutetime_now();
    
    // Simulate work
    for (int i = 0; i < 1000000; i++) {
        asm volatile("");
    }
    
    mulle_absolutetime_t end = mulle_absolutetime_now();
    mulle_relativetime_t elapsed = end - start;
    
    printf("Elapsed: %.6f seconds\n", elapsed);
    return 0;
}
```

### Example 4: Time Quantization for Animations

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    // Snap animation frame times to 60 FPS (1/60 = ~0.0167s)
    mulle_timeinterval_t frame_rate = 1.0 / 60.0;
    
    mulle_timeinterval_t timestamp = 1.0;  // Could be 1.15
    mulle_timeinterval_t quantized = mulle_timeinterval_quantize(timestamp, frame_rate);
    
    printf("Quantized: %.4f\n", quantized);
    return 0;
}
```

### Example 5: Time Range Checking

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    struct mulle_absolutetimerange range;
    range.start = 100.0;
    range.end = 200.0;
    
    mulle_absolutetime_t current = 150.0;
    
    if (current >= range.start && current <= range.end) {
        printf("Time is within range\n");
    }
    
    return 0;
}
```

### Example 6: Sleep with Relative Time

```c
#include <mulle-time/mulle-time.h>
#include <stdio.h>

int main() {
    printf("Sleeping for 0.5 seconds...\n");
    
    mulle_relativetime_t delay = 0.5;
    mulle_relativetime_sleep(delay);
    
    printf("Done sleeping\n");
    return 0;
}
```

## 7. Dependencies

- mulle-c11
