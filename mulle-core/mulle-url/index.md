# mulle-url Library Documentation for AI
<!-- Keywords: url, validation, unicode, percent-escape, rfc, character -->

## 1. Introduction & Purpose

mulle-url is a C99 character classification library that determines which
Unicode characters are valid for the different parts of a URL: scheme, host,
user, password, path, query and fragment. It also provides a check for which
characters do not need percent-encoding. It solves the problem of RFC-compliant
URL component validation without requiring a full URL parser — each public
function answers a single question: "is this single code point legal in this
URL component?". Validation is pure, table/range-driven and side-effect free.

It is a foundational utility in the `mulle-c` ecosystem and is used by the
**NSCharacterSet** extensions of `MulleObjCInetFoundation`. It is part of the
`mulle-core` collection of libraries and depends on `mulle-c11`.

The current version is 2.4.0 (`MULLE__URL_VERSION ((2UL << 20) | (4 << 8) | 0)`).

## 2. Key Concepts & Design Philosophy

- **Component-Based Validation:** Each URL part (scheme, host, user, password,
  path, query, fragment) has its own narrowly-scoped valid character class.
- **Dual-Width API:** Every check exists in a UTF-16 form taking `uint16_t`
  (covers the BMP only) and a UTF-32 form taking `int32_t` (full Unicode).
  The `int32_t` form delegates to the `uint16_t` form for code points
  `<= 0xFFFF` and returns 0 above it.
- **Plane Queries:** A `*plane` function per component answers "does this
  Unicode plane contain any valid character?" in O(1), which allows pruning
  whole 0x10000-codepoint planes before scanning them. Currently only plane 0
  (BMP) ever reports valid characters.
- **ASCII-Centric Ranges:** All validators restrict themselves to the ASCII
  printable range (roughly `0x21`..`0x7e`) and exclude specific reserved
  characters for each component. Non-ASCII code points are always invalid
  except where component rules allow `%` (percent-encoding) or `~`.
- **Non-Destructive & Pure:** Functions only inspect a code point; they never
  modify state, allocate, or touch global memory.

## 3. Core API & Data Structures

There are no structs. The entire public API is a set of standalone,
stateless validation functions. Each URL component has exactly three functions,
all declared in its own header (`src/mulle-unicode-is-<component>.h`), all
prototypes use the same shape:

```c
int   mulle_unicode16_is_<component>( uint16_t c);
int   mulle_unicode_is_<component>( int32_t c);
int   mulle_unicode_is_<component>plane( unsigned int plane);
```

The main header is `src/mulle-url.h` (which defines `MULLE__URL_VERSION` and
includes `generic/include.h` and the reflect headers); it does not itself
declare any functions.

All functions return non-zero (true) if the character/plane is valid, 0
otherwise. Unless noted otherwise, the `int32_t` forms accept only code points
`<= 0xFFFF` (BMP) and return 0 for anything above.

### 3.1. `mulle-unicode-is-nonpercentescape.h`

Characters that **do not** need percent-encoding (RFC "unreserved" set:
`A-Z`, `a-z`, `0-9`, `-`, `_`, `.`, `~`).

- **`int mulle_unicode16_is_nonpercentescape( uint16_t c)`** — UTF-16 check.
- **`int mulle_unicode_is_nonpercentescape( int32_t c)`** — UTF-32 check;
  returns 0 for code points above the BMP.
- **`int mulle_unicode_is_nonpercentescapeplane( unsigned int plane)`** — returns
  1 only for plane 0.

### 3.2. `mulle-unicode-is-validurlscheme.h`

Valid scheme characters: `ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )` (RFC 3986
`scheme`). Range-restricted to `0x2b..0x7a` minus `, / : ; < = > ? @ [ \ ] ^ _ `` `.

- `int mulle_unicode16_is_validurlscheme( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlscheme( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlschemeplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.3. `mulle-unicode-is-validurlhost.h`

Valid host characters (RFC 3986 `reg-name`/IP-literal character class):
printable ASCII `0x21..0x7e` minus `" # % / < > ? @ \ ^ `` ` { | } ``.

- `int mulle_unicode16_is_validurlhost( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlhost( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlhostplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.4. `mulle-unicode-is-validurluser.h`

Valid user-information characters: printable ASCII `0x21..0x7e` minus
`" # % / : < > ? @ [ \ ] ^ `` ` { | } ``.

- `int mulle_unicode16_is_validurluser( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurluser( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurluserplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.5. `mulle-unicode-is-validurlpassword.h`

Valid password characters: same exclusions as the user class
(`" # % / : < > ? @ [ \ ] ^ `` ` { | } ``), range `0x21..0x7e`.

- `int mulle_unicode16_is_validurlpassword( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlpassword( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlpasswordplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.6. `mulle-unicode-is-validurlpath.h`

Valid path characters (RFC 3986 `pchar`): printable ASCII `0x21..0x7e` minus
`" # % ; < > ? [ \ ] ^ `` ` { | } ``. Note `/` **is** allowed, `?` is not
(query separator).

- `int mulle_unicode16_is_validurlpath( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlpath( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlpathplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.7. `mulle-unicode-is-validurlquery.h`

Valid query characters: printable ASCII `0x21..0x7e` minus `" # % < > [ \ ] ^ `` ` { | } ``.
Both `/` and `?` are allowed (unlike path).

- `int mulle_unicode16_is_validurlquery( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlquery( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlqueryplane( unsigned int plane)` — returns 1
  only for plane 0.

### 3.8. `mulle-unicode-is-validurlfragment.h`

Valid fragment characters: same exclusions as the query class
(`" # % < > [ \ ] ^ `` ` { | } ``), range `0x21..0x7e`.

- `int mulle_unicode16_is_validurlfragment( uint16_t c)` — UTF-16 check.
- `int mulle_unicode_is_validurlfragment( int32_t c)` — UTF-32 check (BMP only).
- `int mulle_unicode_is_validurlfragmentplane( unsigned int plane)` — returns 1
  only for plane 0.

## 4. Performance Characteristics

- **Per-character validation:** O(1) — a range check plus a short `switch`
  of excluded code points on ASCII, a single cast/delegation for the UTF-32
  wrapper. No tables, no allocation, no loop over character classes.
- **Plane queries:** O(1) — a single `switch` over the plane index.
- **Memory:** effectively zero additional footprint; no lookup tables are
  generated, all logic is in code and covers plane 0 (BMP) only.
- **Thread safety:** Fully thread-safe. Functions are pure and share no
  global or static state; safe to call concurrently from any thread.
- **Trade-offs:** Validation is limited to BMP code points; code points above
  `U+FFFF` (supplementary planes) are always reported invalid because the
  configurable character sets are ASCII-based. The `*plane` family exists
  specifically to quickly skip these non-BMP planes.

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Validate one component at a time** with the matching `mulle_unicode_is_validurl*`
  function; do not reuse the wrong component check (e.g. path vs. query allow
  different sets of reserved characters).
- **Use the `int32_t` (`mulle_unicode_is_*`) variants** when processing
  generic Unicode input; use the `uint16_t` (`mulle_unicode16_is_*`) variants
  only on input already known to be UTF-16.
- **Use the `*plane` functions for early-exit scanning:** before iterating a
  large code point range, skip whole planes where the plane function returns 0.
- **Use `mulle_unicode_is_nonpercentescape`** to decide whether a character can
  be emitted literally or must be percent-encoded.
- **Include the umbrella header** `#include <mulle-url/mulle-url.h>` to get all
  validators plus the version/reflect headers in one include.

### Common Pitfalls

- **BMP-only behavior:** `int32_t` functions return 0 for all code points
  `> 0xFFFF`. Do not expect validation of supplementary-plane characters.
- **`%` handling is per-component:** `%` is excluded by every
  `validurl*` validator; only `mulle_unicode_is_nonpercentescape` reports `%`-like
  characters. Percent-escaped triplets must be validated elsewhere.
- **Reserved characters differ per component:** e.g. `/` is valid in path and
  query but not in host/user/password; `?` is valid in query but excluded from
  path. Do not assume one component's allowed set for another.
- **Check semantics of `mulle_unicode_is_nonpercentescape`:** it is implemented
  by *positive* allow-listing (`A-Z a-z 0-9 - _ . ~`), whereas the
  `validurl*` checks are implemented by *negative* exclusion from the printable
  ASCII range; their return values are not complements of each other.

## 6. Integration Examples

Coding style: 3-space indent, Allman braces, C89 declarations (all variables
at top, one per line), `return( expr);`.

### Example 1: Validating Each Component of a URL

```c
#include <mulle-url/mulle-url.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

int   main( void)
{
   const char   *scheme   = "https";
   const char   *host     = "example.com";
   const char   *path     = "/path";
   size_t        i;
   int           ok;

   ok = 1;
   for( i = 0; i < strlen( scheme); i++)
      if( ! mulle_unicode_is_validurlscheme( scheme[ i]))
         ok = 0;

   for( i = 0; i < strlen( host); i++)
      if( ! mulle_unicode_is_validurlhost( host[ i]))
         ok = 0;

   for( i = 0; i < strlen( path); i++)
      if( ! mulle_unicode_is_validurlpath( path[ i]))
         ok = 0;

   printf( "URL components valid: %s\n", ok ? "yes" : "no");
   return( 0);
}
```

### Example 2: Skipping Planes That Contain No Valid Host Characters

```c
#include <mulle-url/mulle-url.h>

#include <stdio.h>

int   main( void)
{
   unsigned int   plane;

   for( plane = 0; plane <= 0x10; plane++)
   {
      printf( "host plane #%u: %s\n",
              plane,
              mulle_unicode_is_validurlhostplane( plane) ? "valid" : "invalid");
   }
   return( 0);
}
```

### Example 3: Percent-Escape Decision

```c
#include <mulle-url/mulle-url.h>

#include <stdio.h>
#include <stdint.h>

int   main( void)
{
   int32_t   c;

   for( c = 'A'; c <= 'z'; c++)
   {
      if( ! mulle_unicode_is_nonpercentescape( c))
         printf( "U+%04X '%c' must be percent-encoded\n", c, (int) c);
   }
   return( 0);
}
```

## 7. Dependencies

Direct mulle-sde dependencies (from `.mulle/etc/sourcetree/config`):
- `mulle-c11`: C99/C11 compatibility macros and utilities.