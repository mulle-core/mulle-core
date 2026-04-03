# mulle-dtostr Library Documentation for AI
<!-- Keywords: float-conversion, formatting -->

## 1. Introduction & Purpose

`mulle-dtostr` provides fast and accurate conversion of IEEE-754 `double` values to decimal strings. It implements the Schubfach algorithm via the zmij variant, originally developed by Victor Zverovich in C++ and ported to C by Nat! for integration with `mulle-sprintf`. The library guarantees round-trip compatibility: converting a double to string and back with `strtod` yields the exact same bit-level representation. It is designed to be faster than standard library alternatives while maintaining correctness and precision. This library is part of the `mulle-core` ecosystem and provides foundational string formatting capabilities.

## 2. Key Concepts & Design Philosophy

### Schubfach Algorithm

The library implements the Schubfach algorithm, a modern approach to double-to-string conversion that computes the shortest decimal representation that uniquely identifies the original floating-point value. The zmij variant optimizes this for performance.

### Decimal Decomposition

The conversion process separates concerns: `mulle_dtostr_decompose` breaks a double into its decimal components (significand, exponent, sign, and special case flags), while `mulle_dtostr` handles the formatting. This decomposition allows for custom formatting strategies when needed.

### Fixed-Size Buffer

The algorithm guarantees a maximum output length of 25 bytes (including null terminator) via `MULLE__DTOSTR_BUFFER_SIZE`. This enables safe stack allocation and predictable memory usage.

### Format Selection

The library automatically chooses between fixed-point notation (e.g., "12.2", "0.01") and scientific notation (e.g., "1.23e+20") based on the exponent value, preferring the shorter representation. The threshold follows standard conventions: use fixed-point for exponents in the range [-4, 6], scientific notation otherwise.

### Round-Trip Guarantee

The output format is designed to ensure that `strtod(mulle_dtostr(x))` produces a bit-identical double to the original value `x`. This is critical for serialization and debugging.

### C89 Compatibility

The implementation provides fallback code for systems without 128-bit integer support, ensuring broad compatibility while leveraging native `__int128` operations when available.

## 3. Core API & Data Structures

### 3.1. `mulle-dtostr.h`

#### Constants

##### `MULLE__DTOSTR_VERSION`
- **Purpose:** Version identifier macro for the library
- **Value:** `((0 << 24) | (1 << 8) | 0)` representing version 0.1.0

##### `MULLE__DTOSTR_BUFFER_SIZE`
- **Purpose:** Minimum buffer size required for `mulle_dtostr` output
- **Value:** 25 bytes
- **Usage:** Allocate buffers of at least this size to safely hold any conversion result

#### `struct mulle_dtostr_decimal`

- **Purpose:** Intermediate decimal representation of a double value, optimized for efficient return by value
- **Size:** 128 bits (16 bytes) - fits in two 64-bit registers on most architectures
- **Key Fields:**
  - `uint64_t significand`: The decimal mantissa/significand (16 or 17 decimal digits)
  - `int16_t exponent`: The decimal exponent (power of 10)
  - `uint8_t sign`: Sign flag (0 = positive, 1 = negative)
  - `uint8_t special`: Special value indicator:
    - 0 = normal finite number
    - 1 = infinity
    - 2 = NaN (Not a Number)
    - 3 = zero
  - `uint32_t _padding`: Reserved for alignment (private field, do not use)

- **Interpretation:** For normal values (special == 0), the original double equals:
  ```
  value = (-1)^sign × significand × 10^exponent
  ```
  where significand is a 16 or 17-digit integer.

#### Core Functions

##### `struct mulle_dtostr_decimal mulle_dtostr_decompose(double value)`
- **Purpose:** Decomposes a double into its decimal representation components
- **Parameters:**
  - `value`: The IEEE-754 double to decompose
- **Returns:** A `struct mulle_dtostr_decimal` containing the decimal representation
- **Performance:** O(1) with small constant factor
- **Use Cases:**
  - Custom formatting beyond what `mulle_dtostr` provides
  - Inspecting the internal decimal representation
  - Implementing alternative output formats (e.g., fixed precision, alignment)
- **Special Values:**
  - Infinity: `sign` indicates positive/negative, `special` = 1
  - NaN: `special` = 2, `significand` and `exponent` are undefined
  - Zero: `sign` indicates positive/negative zero, `special` = 3

##### `size_t mulle_dtostr(double value, char *buffer)`
- **Purpose:** Converts a double to its shortest correctly rounded decimal string representation
- **Parameters:**
  - `value`: The IEEE-754 double to convert
  - `buffer`: Pointer to output buffer (must be at least `MULLE__DTOSTR_BUFFER_SIZE` bytes)
- **Returns:** Length of the generated string excluding null terminator
- **Output Format:**
  - Always null-terminated
  - Negative numbers prefixed with '-'
  - Infinity: "inf" or "-inf"
  - NaN: "nan" or "-nan"
  - Zero: "0" or "-0"
  - Normal values: shortest representation (fixed-point or scientific)
- **Performance:** O(1) with highly optimized constant factor
- **Round-Trip Guarantee:** `strtod(buffer, NULL)` will reconstruct the exact original double
- **Thread Safety:** Thread-safe (no shared mutable state)

## 4. Performance Characteristics

### Time Complexity
- **mulle_dtostr:** O(1) - worst case ~25 operations for a full conversion
- **mulle_dtostr_decompose:** O(1) - fixed number of operations regardless of input

### Performance Profile
- Optimized for modern CPUs with fast integer arithmetic
- Uses lookup tables (pow10_significands) for power-of-10 calculations
- Leverages native 128-bit integer multiplication when available (`__int128`)
- Falls back to software 128-bit emulation on 32-bit or older systems
- Two-digit at a time writing for fast string generation
- No memory allocation - all operations use stack or provided buffer

### Memory Usage
- **Stack:** Temporary buffer of ~32 bytes in `write_significand`
- **Data segment:** Pre-computed power-of-10 table (~8KB)
- **Heap:** None - no dynamic allocation

### Thread Safety
- Fully thread-safe
- No shared mutable state
- All operations are pure or use only stack/caller-provided memory
- Power-of-10 table is read-only constant data

### Accuracy
- Produces the shortest decimal string that round-trips exactly through `strtod`
- Correctly handles all IEEE-754 special cases (±0, ±∞, NaN)
- Correctly handles subnormal numbers
- Maintains sign bit information even for zero

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Always Use Correct Buffer Size:** Allocate buffers with `MULLE__DTOSTR_BUFFER_SIZE` to ensure safety
2. **Trust the Round-Trip:** The output is designed for `strtod` compatibility; don't modify formatting
3. **Use Decompose for Custom Formatting:** If you need different formatting than the default, use `mulle_dtostr_decompose` and implement your own formatter
4. **Check Return Length:** The return value gives the actual string length, useful for building larger strings
5. **No Error Checking Needed:** These functions cannot fail given valid inputs (any IEEE-754 double, valid buffer)

### Common Pitfalls

1. **Buffer Too Small:** Always use `MULLE__DTOSTR_BUFFER_SIZE`, not arbitrary sizes like 20 or 24
2. **Don't Parse the Output:** If you need numerical components, use `mulle_dtostr_decompose` instead of parsing the string
3. **Sign of Zero:** Be aware that negative zero produces "-0", not "0"
4. **NaN Variations:** All NaN values produce "nan" or "-nan" - specific NaN payloads are lost
5. **Don't Access _padding:** The `_padding` field in `struct mulle_dtostr_decimal` is private

### Idiomatic Usage

```c
/* Simple conversion */
char   buf[ MULLE__DTOSTR_BUFFER_SIZE];
mulle_dtostr( 3.14159, buf);

/* Building larger strings */
char     result[ 256];
char     *p;
size_t   len;

p = result;
memcpy( p, "Value: ", 7);
p += 7;
len = mulle_dtostr( measurement, p);
p += len;
*p = '\0';

/* Custom formatting via decomposition */
struct mulle_dtostr_decimal   dec;

dec = mulle_dtostr_decompose( value);
if( dec.special == 0)
{
   /* Format with custom rules using dec.significand and dec.exponent */
}
```

## 6. Integration Examples

### Example 1: Basic Conversion

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>

int   main( void)
{
   char   buf[ MULLE__DTOSTR_BUFFER_SIZE];

   mulle_dtostr( 6.62607015e-34, buf);
   puts( buf);  /* Outputs: 6.62607015e-34 */

   return( 0);
}
```

### Example 2: Round-Trip Verification

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

void   verify_roundtrip( double original)
{
   char       buffer[ MULLE__DTOSTR_BUFFER_SIZE];
   double     parsed;
   uint64_t   original_bits;
   uint64_t   parsed_bits;

   mulle_dtostr( original, buffer);
   parsed = strtod( buffer, NULL);

   /* Compare bit representations for exact match */
   memcpy( &original_bits, &original, sizeof( original));
   memcpy( &parsed_bits, &parsed, sizeof( parsed));

   assert( original_bits == parsed_bits);
}

int   main( void)
{
   verify_roundtrip( 3.14159265358979323846);
   verify_roundtrip( 1.7976931348623157e+308);  /* DBL_MAX */
   verify_roundtrip( 2.2250738585072014e-308);  /* DBL_MIN */

   return( 0);
}
```

### Example 3: Custom Formatting with Decomposition

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <string.h>

/* Custom formatter: always use scientific notation with fixed precision */
size_t   format_scientific( double value, char *buffer, int precision)
{
   struct mulle_dtostr_decimal   dec;
   char                          *p;
   uint64_t                      sig;
   int                           exp;
   int                           i;

   dec = mulle_dtostr_decompose( value);
   p   = buffer;

   /* Handle sign */
   if( dec.sign)
      *p++ = '-';

   /* Handle special cases */
   if( dec.special)
   {
      switch( dec.special)
      {
      case 1:
         memcpy( p, "inf", 4);
         return( p - buffer + 3);
      case 2:
         memcpy( p, "nan", 4);
         return( p - buffer + 3);
      case 3:
         memcpy( p, "0", 2);
         return( p - buffer + 1);
      }
   }

   /* Format: d.ddd...e±xxx */
   sig = dec.significand;
   exp = dec.exponent;

   /* Adjust exponent for significand scale */
   if( sig >= 10000000000000000ULL)
      exp += 16;  /* 17-digit significand */
   else
      exp += 15;  /* 16-digit significand */

   /* Write first digit */
   *p++ = '0' + (sig / 10000000000000000ULL);
   sig %= 10000000000000000ULL;

   *p++ = '.';

   /* Write precision digits */
   for( i = 0; i < precision && sig > 0; i++)
   {
      sig *= 10;
      *p++ = '0' + (sig / 10000000000000000ULL);
      sig %= 10000000000000000ULL;
   }

   /* Pad with zeros if needed */
   for( ; i < precision; i++)
      *p++ = '0';

   /* Write exponent */
   p += sprintf( p, "e%+03d", exp);

   return( p - buffer);
}

int   main( void)
{
   char   buf[ 100];

   format_scientific( 3.14159265358979, buf, 8);
   printf( "%s\n", buf);  /* Outputs: 3.14159265e+000 */

   return( 0);
}
```

### Example 4: Building Formatted Output Strings

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <string.h>

/* Build a CSV line with multiple doubles */
size_t   build_csv_line( char *buffer, double x, double y, double z)
{
   char     *p;
   size_t   len;

   p = buffer;

   len = mulle_dtostr( x, p);
   p += len;
   *p++ = ',';

   len = mulle_dtostr( y, p);
   p += len;
   *p++ = ',';

   len = mulle_dtostr( z, p);
   p += len;
   *p = '\0';

   return( p - buffer);
}

int   main( void)
{
   char   line[ 256];

   build_csv_line( line, 1.5, 2.7, 3.14159);
   printf( "%s\n", line);  /* Outputs: 1.5,2.7,3.14159 */

   return( 0);
}
```

### Example 5: Handling Special Values

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <math.h>

void   print_value( const char *label, double value)
{
   char                          buf[ MULLE__DTOSTR_BUFFER_SIZE];
   struct mulle_dtostr_decimal   dec;

   dec = mulle_dtostr_decompose( value);

   printf( "%-15s: ", label);

   if( dec.special)
   {
      switch( dec.special)
      {
      case 1:
         printf( "[INFINITY] ");
         break;
      case 2:
         printf( "[NAN] ");
         break;
      case 3:
         printf( "[ZERO] ");
         break;
      }
   }
   else
   {
      printf( "[NORMAL] sig=%llu exp=%d ",
              (unsigned long long) dec.significand,
              dec.exponent);
   }

   mulle_dtostr( value, buf);
   printf( "-> \"%s\"\n", buf);
}

int   main( void)
{
   print_value( "Positive zero", 0.0);
   print_value( "Negative zero", -0.0);
   print_value( "Infinity", INFINITY);
   print_value( "Neg Infinity", -INFINITY);
   print_value( "NaN", NAN);
   print_value( "Small number", 1.23e-200);
   print_value( "Large number", 9.87e+200);

   return( 0);
}
```

## 7. Dependencies

- **mulle-c11:** Provides C11 standard library compatibility and foundational types

The library has minimal dependencies and is designed to be a low-level component suitable for use in `mulle-sprintf` and other formatting libraries within the mulle-core ecosystem.
