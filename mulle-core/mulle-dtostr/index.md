# mulle-dtostr Library Documentation for AI
<!-- Keywords: float-conversion, formatting, parsing, doubles, ieee754 -->

## 1. Introduction & Purpose

`mulle-dtostr` provides fast, accurate, locale-independent conversion between IEEE-754 `double` values and decimal strings, in both directions. It implements the Schubfach algorithm via the zmij variant, originally developed by Victor Zverovich in C++ and ported to C by Nat! for integration with `mulle-sprintf`. The library guarantees round-trip compatibility: converting a double to string with `mulle_dtostr` and back with `mulle_strtod` yields the exact same bit-level representation (verified over a wide deterministic sample in `test_strtod_roundtrip`). It is designed to be faster than standard library alternatives while maintaining correctness and precision. The `mulle_strtod` parser is locale independent unlike libc `strtod`: accepted syntax is given explicitly by the caller through a `struct mulle__strtod_syntax`. This library is part of the `mulle-core` ecosystem and provides foundational string conversion capabilities.

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

The output format is designed to ensure that `mulle_strtod(mulle_dtostr(x))` produces a bit-identical double to the original value `x`. This is critical for serialization and debugging. libc `strtod` also roundtrips on the platforms tested, but only `mulle_strtod` is guaranteed to.

### Locale-Independent Parser

Unlike libc `strtod`, `mulle_strtod` never looks at `localeconv`. Instead the caller supplies a hand-made locale: a `struct mulle__strtod_syntax` describing the decimal point, optional grouping, the exponent marker, and a flag mask for which token syntax is permitted (leading whitespace, plus/minus, `inf`, `nan`, empty integer/fraction, case sensitivity, hex). Pre-made syntax constructors (`mulle__strtod_syntax_make_c`, `..._make_json`, `..._make_css`, `..._make_en`, `..._make_de`, `..._make_ch`) cover common formats.

### C89 Compatibility

The implementation provides fallback code for systems without 128-bit integer support, ensuring broad compatibility while leveraging native `__int128` operations when available.

## 3. Core API & Data Structures

### 3.1. `mulle-dtostr.h`

#### Constants

##### `MULLE__DTOSTR_VERSION`
- **Purpose:** Version identifier macro for the library
- **Value:** `((0 << 24) | (2 << 8) | 0)` representing version 0.2.0

##### `MULLE__DTOSTR_BUFFER_SIZE`
- **Purpose:** Minimum buffer size required for `mulle_dtostr` output
- **Value:** 25 bytes
- **Usage:** Allocate buffers of at least this size to safely hold any conversion result

#### `struct mulle_dtostr_decimal`

- **Purpose:** Intermediate decimal representation of a double value, used both as the output of `mulle_dtostr_decompose` and the output of the `mulle_strtod` parser
- **Size:** 128 bits (16 bytes) - fits in two 64-bit registers on most architectures
- **Key Fields:**
  - `uint64_t significand`: The decimal mantissa/significand (16 or 17 decimal digits; the parser keeps at most `MULLE__STRTOD_MAX_DIGITS` = 19)
  - `int16_t exponent`: The decimal exponent (power of 10)
  - `uint8_t sign`: Sign flag (0 = positive, 1 = negative)
  - `uint8_t special`: Special value indicator (see the enum below)
  - `uint8_t digits`: Significand digit count; parser output only, `mulle_dtostr_decompose` does not fill it
  - `uint8_t truncated`: Set when the parser dropped a non-zero digit beyond `MULLE__STRTOD_MAX_DIGITS`; parser output only
  - `uint16_t _padding`: Reserved for alignment (private field, do not use)

- **Interpretation:** For normal values (`special == mulle_dtostr_normal_e`), the original double equals:
  ```
  value = (-1)^sign × significand × 10^exponent
  ```
  where significand is a 16 or 17-digit integer.

#### Enum (anonymous) — `mulle_dtostr_*_e` special value constants

- **Purpose:** Values of the `special` field of `struct mulle_dtostr_decimal`:
  - `mulle_dtostr_normal_e = 0`: finite normal (or subnormal) number
  - `mulle_dtostr_inf_e = 1`: infinity
  - `mulle_dtostr_nan_e = 2`: NaN
  - `mulle_dtostr_zero_e = 3`: zero
  - `mulle_dtostr_invalid_e = 4`: nothing was consumed by the parser; never produced by `mulle_dtostr_decompose`

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
  - Infinity: `sign` indicates positive/negative, `special` = `mulle_dtostr_inf_e` (1)
  - NaN: `special` = `mulle_dtostr_nan_e` (2), `significand` and `exponent` are undefined
  - Zero: `sign` indicates positive/negative zero, `special` = `mulle_dtostr_zero_e` (3)

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
- **Round-Trip Guarantee:** `mulle_strtod(buffer, NULL)` reconstructs the exact original double bit for bit
- **Thread Safety:** Thread-safe (no shared mutable state)

### 3.2. `_mulle-strtod.h`

String to double conversion: the inverse of `mulle-dtostr`. The parser is locale independent. Everything that follows is declared in `_mulle-strtod.h`, which is included by the public umbrella header `mulle-dtostr.h`.

#### Constants

##### `MULLE__STRTOD_NUL_TERMINATED`
- **Purpose:** Pass as `len` to the parse/scan functions to indicate `s` is a NUL terminated C string
- **Value:** `((size_t) -1)`

##### `MULLE__STRTOD_MAX_DIGITS`
- **Purpose:** Maximum number of decimal digits kept in the significand
- **Value:** 19. Digits beyond this are dropped and flagged in the `truncated` field.

##### `MULLE__STRTOD_EXPONENT_LIMIT`
- **Purpose:** The decimal exponent is clamped to this range
- **Value:** 9999 (both ends are far outside the double range, so the converter can decide overflow or underflow without re-examining the digits)

#### Enum `mulle_strtod_*_e` flags (anonymous)

- **Purpose:** Flag bits for `struct mulle__strtod_syntax.flags`. Anything not allowed simply terminates the conversion; it is *not* an error. `"+1"` with `mulle_strtod_allow_plus_e` clear is therefore not a number at all, whereas `"1e5"` with `mulle_strtod_allow_exponent_plus_e` clear is the number `1` followed by the text `"e5"`.
  - `mulle_strtod_allow_leading_whitespace_e = 0x0001`: accepts leading whitespace (`" 1.0"`)
  - `mulle_strtod_allow_plus_e = 0x0002`: accepts `"+1.0"`
  - `mulle_strtod_allow_minus_e = 0x0004`: accepts `"-1.0"`
  - `mulle_strtod_allow_exponent_plus_e = 0x0008`: accepts `"1e+5"`
  - `mulle_strtod_allow_exponent_minus_e = 0x0010`: accepts `"1e-5"`
  - `mulle_strtod_allow_inf_e = 0x0020`: accepts `"inf"` and `"infinity"`
  - `mulle_strtod_allow_nan_e = 0x0040`: accepts `"nan"` and `"nan(_1a)"`
  - `mulle_strtod_allow_empty_integer_e = 0x0080`: accepts `".5"`
  - `mulle_strtod_allow_empty_fraction_e = 0x0100`: accepts `"5."`
  - `mulle_strtod_case_sensitive_e = 0x0200`: `"INF"` and `"1E5"` fail
  - `mulle_strtod_allow_hex_e = 0x0400`: reserved, unused

#### `struct mulle__strtod_syntax`

- **Purpose:** A hand made "locale" describing what characters may appear in a number text. Multi byte separators (e.g. U+00A0 for French/SI) are not supported.
- **Key Fields:**
  - `char decimal_point`: `.` or `,`
  - `char grouping`: `,` `.` `'` `_` ` ` or `0` for none
  - `uint8_t group_size`: 3, or 0 for "don't check positions" (accept separator anywhere in the integer part)
  - `char exponent[ 4]`: NUL padded, e.g. `"e"`, `"ed"` (Fortran), or `""` for no exponent (CSS)
  - `uint32_t flags`: OR of the `mulle_strtod_allow_*` bits above

#### Syntax Factory Functions (all `static inline`)

- `mulle__strtod_syntax_make_dtostr( void)`: exactly what `mulle_dtostr` emits and nothing else (decimal point `.`, exponent `e`, flags = minus + exponent plus/minus + inf + nan)
- `mulle__strtod_syntax_make_c( void)`: the C locale, as libc `strtod` does it (adds leading whitespace, plus, empty integer, empty fraction on top of dtostr)
- `mulle__strtod_syntax_make_json( void)`: JSON numbers — no `+1`, no `inf`, no `nan`, no `.5`, no `5.`
- `mulle__strtod_syntax_make_css( void)`: CSS 2.1 numbers — no exponent, `.5` ok, `5.` not
- `mulle__strtod_syntax_make_en( void)`: `1,234,567.89` (C syntax plus `,` grouping of 3)
- `mulle__strtod_syntax_make_de( void)`: `1.234.567,89` (decimal point `,`, grouping `.`)
- `mulle__strtod_syntax_make_ch( void)`: `1'234'567.89` (grouping `'`)

#### `int mulle__strtod_syntax_is_valid( const struct mulle__strtod_syntax *syntax)`
- **Purpose:** Returns 1 if the syntax is usable, catches combinations that cannot work (e.g. a grouping character that is also the decimal point)

#### `struct mulle_dtostr_decimal mulle_strtod_parse( const char *s, size_t len, const struct mulle__strtod_syntax *syntax, char **endptr)`
- **Purpose:** Reads the longest prefix of `s` that is a number in `syntax` and returns it as a decimal — the same intermediate form `mulle_dtostr_decompose` produces. Does not convert to a double.
- **Parameters:**
  - `s`: input text
  - `len`: `MULLE__STRTOD_NUL_TERMINATED` for a C string, otherwise bounds the text and no NUL is needed
  - `syntax`: may be NULL, which means `mulle__strtod_syntax_make_c`
  - `endptr`: may be NULL; set to the first character not consumed (which is `s` itself if nothing was consumed)
- **Result semantics:**
  - Nothing consumed is reported as `special` being `mulle_dtostr_invalid_e` (4)
  - The exponent is clamped to `+/- MULLE__STRTOD_EXPONENT_LIMIT`
  - The significand holds at most `MULLE__STRTOD_MAX_DIGITS` digits; `truncated` says whether a non-zero digit was dropped

#### `double mulle_strtod_compose( struct mulle_dtostr_decimal decimal)`
- **Purpose:** Composes a double from a canonical decimal (the output of `mulle_dtostr_decompose` or `mulle_strtod_parse`). This is a bisection converter: it calls `mulle_dtostr_decompose` on candidate doubles (powers of two stepping) until one roundtrips exactly.
- **Note:** For decimals that are not the shortest representation of any double, the closest double is returned (possibly 1 ulp off). `mulle_dtostr_inf_e`, `mulle_dtostr_nan_e`, `mulle_dtostr_zero_e`, and `mulle_dtostr_invalid_e` are handled directly without searching.

#### `int mulle_strtod_scan( const char *s, size_t len, const struct mulle__strtod_syntax *syntax, double *p_value, char **endptr)`
- **Purpose:** Combined parse + convert in one call. Returns a status code (see below) and sets `*p_value` on success. `endptr`, `syntax`, `len` behave as in `mulle_strtod_parse`. `errno` is never touched.
- **Status codes:**
  - `mulle_strtod_ok_e = 0`
  - `mulle_strtod_no_conversion_e = 1`
  - `mulle_strtod_overflow_e = 2`
  - `mulle_strtod_underflow_e = 3`
- **Known limitation:** the parser keeps at most 19 significant digits. Input produced by `mulle_dtostr` (at most 17 digits) roundtrips bit exactly. Arbitrary text with more than 19 significant digits may land 1 ulp away from the correctly rounded double in about 0.1% of cases (when the true value falls very close to the midpoint between two adjacent doubles and the distinguishing information is in the dropped digits).

#### `double mulle_strtod( const char *s, char **endptr)` (static inline)
- **Purpose:** Drop-in for libc `strtod`, using the C locale syntax (NULL `syntax`)

#### `double mulle_strtod_len( const char *s, size_t len, char **endptr)` (static inline)
- **Purpose:** Like `mulle_strtod` but with an explicit length bound, no NUL needed

## 4. Performance Characteristics

### Time Complexity
- **mulle_dtostr:** O(1) - worst case ~25 operations for a full conversion
- **mulle_dtostr_decompose:** O(1) - fixed number of operations regardless of input
- **mulle_strtod_parse / mulle_strtod_scan:** O(n) - linear in the number of input characters, with a constant-time conversion step
- **mulle_strtod_compose:** O(log(2^53)) bounded bisection - a fixed maximum number of `mulle_dtostr_decompose` calls (each O(1))

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
- Produces the shortest decimal string that roundtrips exactly through `mulle_strtod` (and libc `strtod` on tested platforms)
- Correctly handles all IEEE-754 special cases (±0, ±∞, NaN)
- Correctly handles subnormal numbers
- Maintains sign bit information even for zero

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Always Use Correct Buffer Size:** Allocate buffers with `MULLE__DTOSTR_BUFFER_SIZE` to ensure safety
2. **Trust the Round-Trip:** The output is designed for `mulle_strtod` compatibility; don't modify formatting
3. **Use Decompose for Custom Formatting:** If you need different formatting than the default, use `mulle_dtostr_decompose` and implement your own formatter
4. **Check Return Length:** The return value gives the actual string length, useful for building larger strings
5. **No Error Checking Needed:** `mulle_dtostr` cannot fail given valid inputs (any IEEE-754 double, valid buffer)
6. **Prefer `mulle_strtod_scan`:** Use it instead of checking an `errno` code from libc; it is locale independent, never touches `errno`, and reports overflow/underflow/no-conversion explicitly
7. **Reuse Pre-Built Syntax:** Use the `mulle__strtod_syntax_make_*` factories rather than hand-rolling a `struct mulle__strtod_syntax`

### Common Pitfalls

1. **Buffer Too Small:** Always use `MULLE__DTOSTR_BUFFER_SIZE`, not arbitrary sizes like 20 or 24
2. **Don't Parse the Output:** If you need numerical components, use `mulle_dtostr_decompose` instead of parsing the string
3. **Sign of Zero:** Be aware that negative zero produces "-0", not "0"
4. **NaN Variations:** All NaN values produce "nan" or "-nan" - specific NaN payloads are lost
5. **Don't Access _padding:** The `_padding` field in `struct mulle_dtostr_decimal` is private
6. **Strictness is Silent:** An unallowed character is not an error, it just ends the conversion — check `endptr` and the `special`/status code to know how much was consumed
7. **`digits`/`truncated` are Parser-Only:** `mulle_dtostr_decompose` does not fill them; only `mulle_strtod_parse` output carries them

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
if( dec.special == mulle_dtostr_normal_e)
{
   /* Format with custom rules using dec.significand and dec.exponent */
}

/* Parsing back a double with explicit status handling */
double     parsed;
char       *endptr;
int        status;
char       text[] = "3.14159";

status = mulle_strtod_scan( text,
                            MULLE__STRTOD_NUL_TERMINATED,
                            NULL,           /* C locale syntax */
                            &parsed,
                            &endptr);
if( status == mulle_strtod_ok_e)
{
   /* parsed holds the double */
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void   verify_roundtrip( double original)
{
   char       buffer[ MULLE__DTOSTR_BUFFER_SIZE];
   double     parsed;
   uint64_t   original_bits;
   uint64_t   parsed_bits;

   mulle_dtostr( original, buffer);
   parsed = mulle_strtod( buffer, NULL);

   /* Compare bit representations for exact match */
   memcpy( &original_bits, &original, sizeof( original));
   memcpy( &parsed_bits, &parsed, sizeof( parsed));

   if( original_bits != parsed_bits)
      printf( "FAIL: %s -> %s\n", buffer, buffer);
}

int   main( void)
{
   verify_roundtrip( 3.14159265358979323846);
   verify_roundtrip( 1.7976931348623157e+308);  /* DBL_MAX */
   verify_roundtrip( 2.2250738585072014e-308);  /* DBL_MIN */

   return( 0);
}
```

### Example 3: Parsing with Custom Syntax

`mulle_strtod_scan` is locale independent. Use the pre-built syntax
factories to parse German or JSON style numbers, and read the status code
for overflow/underflow handling.

```c
#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>

void   parse_with( const char *label,
                   const char *text,
                   const struct mulle__strtod_syntax *syntax)
{
   char   *endptr;
   double  value;
   int     status;

   status = mulle_strtod_scan( text,
                               MULLE__STRTOD_NUL_TERMINATED,
                               syntax,
                               &value,
                               &endptr);
   printf( "%-22s status=%d value=%.6g consumed=%ld\n",
           label, status, value, (long) (endptr - text));
}

int   main( void)
{
   struct mulle__strtod_syntax   syntax;
   int                           valid;

   /* German syntax accepts "1.234,5" */
   syntax = mulle__strtod_syntax_make_de();
   valid  = mulle__strtod_syntax_is_valid( &syntax);
   if( valid)
      parse_with( "German", "1.234,5", &syntax);

   /* JSON forbids "+1", "inf", "nan" and ".5" */
   syntax = mulle__strtod_syntax_make_json();
   parse_with( "JSON", "1e5", &syntax);
   parse_with( "JSON", "+1.5", &syntax);   /* ends at '+', no conversion */

   return( 0);
}
```

### Example 4: Custom Formatting with Decomposition

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
      case mulle_dtostr_inf_e:
         memcpy( p, "inf", 4);
         return( p - buffer + 3);
      case mulle_dtostr_nan_e:
         memcpy( p, "nan", 4);
         return( p - buffer + 3);
      case mulle_dtostr_zero_e:
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

### Example 5: Building Formatted Output Strings

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

### Example 6: Handling Special Values

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
      case mulle_dtostr_inf_e:
         printf( "[INFINITY] ");
         break;
      case mulle_dtostr_nan_e:
         printf( "[NAN] ");
         break;
      case mulle_dtostr_zero_e:
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

- **mulle-c11:** Provides C11 standard library compatibility and foundational types (bit layout helpers via `MULLE_C_EXTERN_GLOBAL`, etc.)

The library has minimal dependencies and is designed to be a low-level component suitable for use in `mulle-sprintf` and other formatting libraries within the mulle-core ecosystem.
