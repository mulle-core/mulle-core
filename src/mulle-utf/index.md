# mulle-utf Library Documentation for AI
<!-- Keywords: unicode, utf8, utf16, utf32, conversion, char5, char7 -->

## 1. Introduction & Purpose

- mulle-utf is a small C (C99) Unicode analysis and conversion library (current version 6.0.0, `MULLE__UTF_VERSION`). It provides validation, classification, conversion, and primitive string utilities for UTF-8, UTF-16 and UTF-32 as well as the compact encodings char5/char7. It is the backbone of **NSString**.
- Solves: detect/validate encodings, compute exact lengths, convert between UTF encodings, iterate code points, perform length-aware `<string.h>`-like operations on UTF-8/16/32 buffers, and compactly pack small ASCII tokens into machine words for use as keys/hashes.
- Key features: validation helpers, length calculators, low-level fast converters (preallocated `dst`), streaming bufferconverters (`mulle_utf_add_bytes_function_t` callback), allocator-based convenience `_string` routines, rover iterator abstraction, char5/char7 packing with FNV-1A hashing, legacy 8-bit (iso1/macroman/nextstep) converters.
- Relationship: a component of the `mulle-core` family; depends on `mulle-allocator` (allocation) and `mulle-data` (`struct mulle_data`/`struct mulle_range` interop). Tests also use `mulle-allocator`'s `struct mulle_allocator`.

### Changes since the previous index.md (v5.2.1 -> v6.0.0)
- **Breaking:** `mulle_utf16_length()` was removed.
- All input-only pointers are now `const`-qualified (e.g. `mulle_utf8_utf16length( const char *src, size_t len)`). Call sites are unaffected.
- UTF-16 edge-case fixes: `mulle_utf16_validate()` no longer reads past the end when the buffer ends in a high surrogate; `mulle_utf16_information()` `utf8len` and `mulle_utf16_convert_to_utf8_string()` no longer overcount astral characters by one byte.
- `mulle_utf32_is_surrogatecharacter()` no longer misclassifies U+E000 as a surrogate (upper bound is now exclusive `< 0xE000`); `mulle_utf32_is_invalidcharacter()` now also rejects surrogate code points.
- `mulle_utf8_utf16length()` returns `(size_t) -1` on a truncated sequence instead of silently returning 0; `mulle_utf32_utf16length()` counts U+FFFF as one UTF-16 unit.
- `_mulle_utf8_convert_to_iso1()` now decodes two-byte sequences (U+0080..U+00FF); `mulle_utf8_strnstr()` finds overlapping matches; `mulle_utf8data_range_of_utf32_range()` handles multi-byte characters correctly.
- validate()/information() treat an embedded NUL as a valid terminator; `mulle_char5_is_char5string32/64()` reject strings containing zero bytes.
- Added a fuzzing harness under `fuzz/` and Unicode data tables under `src/unicode/` (used by tooling; not part of the public API).

## 2. Key Concepts & Design Philosophy

- Types: explicit fixed-width types: `mulle_utf16_t` (uint16_t), `mulle_utf32_t` (int32_t, deliberately *signed* so `-1` can serve as an error/end sentinel from iteration functions), `mulle_char5_t`/`mulle_char7_t` (uintptr_t).
- Two-layer API: low-level, unchecked conversion primitives (fast, write into a caller-supplied `dst`, return the end-of-dst pointer) and convenience `_string`/context functions that allocate (with a `struct mulle_allocator`, NULL allowed) and NUL-terminate.
- Validation-first: many routines expect sane input. Validate once (`mulle_utf8_validate` / `mulle_utf8_information`) and reuse the verdict; the fast paths do not re-check validity.
- `-1` length convention: passing `len == (size_t) -1` to length/validate/convert-string routines means "NUL-terminated string, determine length internally".
- Range abstraction: `struct mulle_utf_information` reports byte lengths (`utf8len`, `utf16len`, `utf32len`), validity (`invalid`), BOM/ASCII/char5/utf15 classification and whether a terminating zero is present (`has_terminating_zero`).
- Rover abstraction: `struct mulle_utf_rover` is a uniform, encoding-agnostic code-point iterator with init helpers per encoding.
- Compact encodings: char5 (32-char reduced charset, up to 6 chars in 32-bit / 12 in 64-bit) and char7 (7-bit ASCII, up to 4 chars / 8 chars) pack small strings into `uintptr_t` for fast keys and hashing.

## 3. Core API & Data Structures

All public headers are reachable via `#include <mulle-utf/mulle-utf.h>` (umbrella, `src/mulle-utf.h`). Signatures below are copied verbatim from the headers.

### 3.1. `mulle-utf-type.h`

```c
typedef uintptr_t  mulle_char5_t;
typedef uintptr_t  mulle_char7_t;
typedef uint16_t   mulle_utf16_t;
typedef int32_t    mulle_utf32_t;  //  0 - 0x10FFFF.

enum { mulle_utf32_max = 0x10FFFF };

struct mulle_utf_information
{
   size_t   utf8len;
   size_t   utf16len;
   size_t   utf32len;
   void     *start;          // behind BOM if bommed, otherwise start
   void     *invalid;        // first fail char
   int      has_bom;
   int      is_ascii;
   int      is_char5;
   int      is_utf15;
   int      has_terminating_zero;
};

static inline int   mulle_utf_information_is_valid( struct mulle_utf_information *info);
// returns( info->invalid == NULL);
```

- `mulle_utf32_max` is the maximum valid code point. Negative `mulle_utf32_t` values are never valid code points.

```c
enum mulle_utf_scan_return
{
   mulle_utf_has_overflown           = -2,
   mulle_utf_is_invalid              = -1,
   mulle_utf_is_valid                = 0x0,
   mulle_utf_is_too_large_for_signed = 0x1, // too large for a signed number
   mulle_utf_has_trailing_garbage    = 0x2
};

typedef void   mulle_utf_add_bytes_function_t( void *userinfo, void *bytes, size_t length);

enum mulle_utf_charinfo
{
   mulle_utf_is_not_char5_or_char7 = 0,
   mulle_utf_is_char5              = 1,
   mulle_utf_is_char7              = 2
};
```

- `mulle_utf_add_bytes_function_t` is the sink used by all `*_bufferconvert_*` streaming functions; `userinfo` is the `void *buffer` argument passed in.

### 3.2. `mulle-utf8.h`

Character classification (all `static inline`, operate on bytes of a UTF-8 sequence):

```c
static inline int   mulle_utf8_is_asciicharacter( char c);             // (unsigned char) c < 0x80
enum { mulle_utf8_ascii_start_character, mulle_utf8_multiple_start_character, mulle_utf8_invalid_start_character };
static inline int   mulle_utf8_is_invalidstartcharacter( char c);      // 0x80..0xC1 and >= 0xF5 are invalid start bytes
static inline int   mulle_utf8_get_startcharactertype( char c);
static inline int   mulle_utf8_is_validcontinuationcharacter( char c); // 0x80..0xBF
static inline size_t  mulle_utf8_get_extracharacterslength( char c);   // 1/2/3 continuation bytes
```

Lengths & BOM:

```c
size_t  mulle_utf8_utf16length( const char *src, size_t len);   // len == -1 => NUL-terminated; returns (size_t)-1 on truncated input
size_t  mulle_utf8_utf32length( const char *src, size_t len);
static inline size_t  mulle_utf8_utf16maxlength( size_t len);   // len * 4
static inline int  mulle_utf8_has_leading_bomcharacter( const char *src, size_t len);  // EF BB BF
```

Validation / information (all `MULLE__UTF_GLOBAL`):

```c
int   mulle_utf8_are_valid_extracharacters( const char *s, size_t len, mulle_utf32_t *p_c);
int  mulle_utf8_information( const char *s, size_t len, struct mulle_utf_information *info);  // 0 on success, fills info
int  mulle_utf8_is_ascii( const char *s, size_t len);
char  *mulle_utf8_validate( const char *src, size_t len);       // NULL if OK, otherwise address of offending character
```

Iteration (fast, assumes valid UTF-8; `mulle_utf8_next_utf32character` returns -2 if malformed and -1 at end-of-buffer for the `data` variant):

```c
mulle_utf32_t   _mulle_utf8_next_utf32character( char **s_p);
mulle_utf32_t   _mulle_utf8_previous_utf32character( char **s_p);
static inline mulle_utf32_t   mulle_utf8_next_utf32character( char **s_p);
```

`struct mulle_utf8data` (a `char *` + `size_t` buffer pair; kept compatible with `mulle-data`'s `struct mulle_data`):

```c
struct mulle_utf8data
{
   char    *characters;
   size_t   length;
};

static inline size_t  mulle_utf8_strlen( const char *s);
static inline struct mulle_utf8data   mulle_utf8data_make( char *s, size_t length);
static inline struct mulle_utf8data   mulle_utf8data_make_empty( void);
static inline struct mulle_utf8data   mulle_utf8data_make_invalid( void);
static inline void   mulle_utf8data_init( struct mulle_utf8data *data, const char *characters, size_t length, struct mulle_allocator *allocator);
static inline void   mulle_utf8data_done( struct mulle_utf8data *data, struct mulle_allocator *allocator);
static inline int   mulle_utf8data_is_empty( struct mulle_utf8data data);
static inline int   mulle_utf8data_is_invalid( struct mulle_utf8data data);
static inline void   mulle_utf8data_assert( struct mulle_utf8data data);
struct mulle_utf8data   mulle_utf8data_copy( struct mulle_utf8data data, struct mulle_allocator *allocator);
static inline struct mulle_utf8data   mulle_data_as_utf8data( struct mulle_data data);
static inline struct mulle_data   mulle_utf8data_as_data( struct mulle_utf8data data);
```

`mulle_utf8data_init`/`_done` allocate/free a NUL-terminated copy with the given allocator; when `length == 0`, `characters` points at a static `""` (`init` becomes a no-op copy). `mulle_utf8data_done` only frees when `data->length != 0`.

Rover-based iteration over a `struct mulle_utf8data` (returns -1 at end, -2 if malformed):

```c
mulle_utf32_t   _mulle_utf8data_next_utf32character( struct mulle_utf8data *rover);
mulle_utf32_t   __mulle_utf8data_next_utf32character( struct mulle_utf8data *rover, char c);
static inline mulle_utf32_t   mulle_utf8data_next_utf32character( struct mulle_utf8data *rover);
```

Low-level converters (no checks; `dst` must be wide enough; return end of `dst`; `len` may not be -1 for the `_convert_to_utf16` form):

```c
mulle_utf16_t   *_mulle_utf8_convert_to_utf16( const char *src, size_t len, mulle_utf16_t *dst);
mulle_utf32_t   *_mulle_utf8_convert_to_utf32( const char *src, size_t len, mulle_utf32_t *dst);
```

Streaming converters (do not skip BOMs, do not validate, do not add a trailing zero; `buffer`/`addbytes` are the sink; you may pass a mulle-buffer with `mulle_buffer_add_bytes`):

```c
void   mulle_utf8_bufferconvert_to_utf16( const char *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
void   mulle_utf8_bufferconvert_to_utf32( const char *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
```

Range helpers and legacy 8-bit converters (`struct mulle_range` comes from `mulle-data`):

```c
static inline char *   mulle_utf8data_characters_in_range( struct mulle_utf8data data, struct mulle_range range);
char   *_mulle_iso1_convert_to_utf8( const char *src, size_t len, char *dst);
char   *_mulle_macroman_convert_to_utf8( const char *macroman, size_t len, char *dst);
char   *_mulle_nextstep_convert_to_utf8( const char *nextstep, size_t len, char *dst);
char   *_mulle_utf8_convert_to_iso1( const char *src, size_t len, char *dst, int unknown); // unknown: -1 bail (NULL), 0 skip, else replace
struct mulle_utf8data   mulle_utf8data_range_of_utf32_range( struct mulle_utf8data, struct mulle_range range);
```

### 3.3. `mulle-utf8-string.h`

Length-aware `<string.h>` substitutes for UTF-8 (`len == -1` means NUL-terminated):

```c
static inline int  mulle_utf8_strcmp( const char *s, const char *other);
static inline int  mulle_utf8_strncmp( const char *s, const char *other, int len);
static inline char  *mulle_utf8_strdup( const char *s);                      // allocator_strdup( NULL, s)
static inline size_t  mulle_utf8_strnlen( const char *s, size_t len);
char   *mulle_utf8_strncpy( char *dst, size_t len, const char *src);         // terminates, does not zero-fill
char   *mulle_utf8_strnstr( const char *s, size_t len, const char *search);  // finds overlapping matches
static inline char  *mulle_utf8_strstr( const char *s, const char *search);
char   *mulle_utf8_strnchr( const char *s, size_t len, mulle_utf32_t c);
static inline char  *mulle_utf8_strchr( const char *s, mulle_utf32_t c);
size_t   mulle_utf8_strspn( const char *s, const char *search);
size_t   mulle_utf8_strcspn( const char *s, const char *search);
char   *mulle_utf8_skiputf32( char *s, size_t *p_n);                         // p_n: utf32 chars to skip, returns actually skipped
static inline size_t   mulle_utf8_strnspn( const char *s, size_t length, const char *search);
static inline size_t   mulle_utf8_strncspn( const char *s, size_t length, const char *search);
static inline void   mulle_utf8_memcpy( char *dst, const char *src, size_t len);
static inline void   mulle_utf8_memmove( char *dst, const char *src, size_t len);
```

(The `str*spn/cspn` variants are UTF-8/UTF-32 aware so they work when the buffer has no terminating NUL.)

### 3.4. `mulle-utf16.h`

```c
struct mulle_utf16data
{
   mulle_utf16_t   *characters;
   size_t          length;
};
```

- Lifecycle: there is no allocator-based `init/done`; wrap existing buffers with `mulle_utf16data_make( s, length)` (declared `static inline`, `len == -1` => `mulle_utf16_strlen`). `mulle_utf16_strlen` returns 0 for NULL.

Classification:

```c
static inline size_t   mulle_utf16_strlen( const mulle_utf16_t *s);
static inline struct mulle_utf16data   mulle_utf16data_make( mulle_utf16_t *s, size_t length);
static inline int   mulle_utf16_is_asciicharacter( mulle_utf16_t c);
static inline int   mulle_utf16_is_char5character( mulle_utf16_t c);
static inline int   mulle_utf16_is_ascii( const mulle_utf16_t *src, size_t len);
static inline int   mulle_utf16_is_utf15( const mulle_utf16_t *src, size_t len);
static inline size_t  mulle_utf16_utf8maxlength( size_t len);   // len * 4
int   mulle_utf16_contains_character_larger_or_equal( const mulle_utf16_t *src, size_t len, mulle_utf16_t d);
```

Validation / information (all take `const` input now):

```c
int     mulle_utf16_information( const mulle_utf16_t *src, size_t len, struct mulle_utf_information *info); // 0 on success
size_t  mulle_utf16_utf8length( const mulle_utf16_t *src, size_t len);
size_t  mulle_utf16_utf32length( const mulle_utf16_t *src, size_t len);
mulle_utf16_t  *mulle_utf16_validate( const mulle_utf16_t *src, size_t len);
int  mulle_utf16_is_valid_surrogatepair( mulle_utf16_t hi, mulle_utf16_t lo);
```

Surrogate-pair iteration (notes: `_next`/`_previous` walk whole UTF-16 characters, `mulle_utf16_t **s_p`):

```c
mulle_utf32_t   _mulle_utf16_next_utf32character( mulle_utf16_t **s_p);
mulle_utf32_t   _mulle_utf16_previous_utf32character( mulle_utf16_t **s_p);
```

Low-level converters and streaming:

```c
mulle_utf32_t  *_mulle_utf16_convert_to_utf32( const mulle_utf16_t *src, size_t len, mulle_utf32_t *dst);
char  *_mulle_utf16_convert_to_utf8( const mulle_utf16_t *src, size_t len, char *dst);
void   mulle_utf16_bufferconvert_to_utf8( const mulle_utf16_t *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
void   mulle_utf16_bufferconvert_to_utf32( const mulle_utf16_t *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
enum mulle_utf_charinfo   _mulle_utf16_charinfo( const mulle_utf16_t *src, size_t len);
```

### 3.5. `mulle-utf16-string.h`

```c
size_t  mulle_utf16_strnlen( mulle_utf16_t *src, size_t len);
mulle_utf16_t  *mulle_utf16_strdup( mulle_utf16_t *s);
mulle_utf16_t  *mulle_utf16_strncpy( mulle_utf16_t *dst, size_t len, mulle_utf16_t *src);
int             mulle_utf16_strncmp( mulle_utf16_t *s1, mulle_utf16_t *s2, size_t len);
mulle_utf16_t  *mulle_utf16_strchr( mulle_utf16_t *s, mulle_utf32_t c);  // sic
int            _mulle_utf16_atoi( mulle_utf16_t **s);
mulle_utf16_t  *mulle_utf16_strstr( mulle_utf16_t *s1, mulle_utf16_t *s2);
size_t    mulle_utf16_strspn( mulle_utf16_t *s1, mulle_utf16_t *s2);
size_t    mulle_utf16_strcspn( mulle_utf16_t *s1, mulle_utf16_t *s2);
static inline int   mulle_utf16_strcmp( mulle_utf16_t *s1, mulle_utf16_t *s2);  // strncmp( s1, s2, strlen( s2))
static inline int   mulle_utf16_atoi( mulle_utf16_t *s);
static inline void   mulle_utf16_memcpy( mulle_utf16_t *dst, mulle_utf16_t *src, size_t len);   // len is number of mulle_utf16_t units
static inline void   mulle_utf16_memmove( mulle_utf16_t *dst, mulle_utf16_t *src, size_t len);
```

### 3.6. `mulle-utf32.h`

```c
struct mulle_utf32data
{
   mulle_utf32_t   *characters;
   size_t    length;
};
```

- Lifecycle: `mulle_utf32data_make( s, length)` wraps a buffer (`len == -1` => `mulle_utf32_strlen`); `mulle_utf32data_make_null()` builds `{ NULL, 0 }` for invalid/empty. No allocator-based constructor.

Classification:

```c
static inline size_t   mulle_utf32_strlen( const mulle_utf32_t *s);
static inline struct mulle_utf32data   mulle_utf32data_make( mulle_utf32_t *s, size_t length);
static inline struct mulle_utf32data   mulle_utf32data_make_null( void);
static inline int   mulle_utf32_is_asciicharacter( mulle_utf32_t c);
static inline int   mulle_utf32_is_char5character( mulle_utf32_t c);
static inline int   mulle_utf32_get_unicodeplane( mulle_utf32_t c);
```

Validation / information / lengths:

```c
size_t   mulle_utf32_utf8length( const mulle_utf32_t *src, size_t len);
size_t   mulle_utf32_utf16length( const mulle_utf32_t *src, size_t len);
int   mulle_utf32_information( const mulle_utf32_t *src, size_t len, struct mulle_utf_information *info);
mulle_utf32_t  *mulle_utf32_validate( const mulle_utf32_t *src, size_t len);
```

Iteration (complete stubs for symmetry):

```c
static inline mulle_utf32_t   _mulle_utf32_next_utf32character( mulle_utf32_t **s_p);     // *(*s_p)++
static inline mulle_utf32_t   _mulle_utf32_previous_utf32character( mulle_utf32_t **s_p); // *--(*s_p)
```

Converters:

```c
mulle_utf16_t  *_mulle_utf32_convert_to_utf16_as_surrogatepair( mulle_utf32_t x, mulle_utf16_t *dst);
mulle_utf16_t   *_mulle_utf32_convert_to_utf16( const mulle_utf32_t *src, size_t len, mulle_utf16_t *dst);
char  *_mulle_utf32_convert_to_utf8( const mulle_utf32_t *src, size_t len, char *dst);
void   mulle_utf32_bufferconvert_to_utf8( const mulle_utf32_t *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
void   mulle_utf32_bufferconvert_to_utf16( const mulle_utf32_t *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
```

Single-character encoders (`x` must be a valid code point; write the ASCII case inline, else delegate):

```c
char   *_mulle_utf32_as_utf8_not_ascii( mulle_utf32_t x, char *dst);
mulle_utf16_t   *_mulle_utf32_as_utf16_not_ascii( mulle_utf32_t x, mulle_utf16_t *dst);
static inline char   *mulle_utf32_as_utf8( mulle_utf32_t x, char *dst);
static inline mulle_utf16_t   *mulle_utf32_as_utf16( mulle_utf32_t x, mulle_utf16_t *dst);
enum mulle_utf_charinfo   _mulle_utf32_charinfo( const mulle_utf32_t *src, size_t len);
```

### 3.7. `mulle-utf32-string.h`

```c
size_t  mulle_utf32_strnlen( mulle_utf32_t *src, size_t len);
mulle_utf32_t  *mulle_utf32_strdup( mulle_utf32_t *s);
mulle_utf32_t  *mulle_utf32_strncpy( mulle_utf32_t *dst, size_t len, mulle_utf32_t *src);
mulle_utf32_t  *mulle_utf32_strchr( mulle_utf32_t *s, mulle_utf32_t c);
int            _mulle_utf32_atoi( mulle_utf32_t **s);
mulle_utf32_t  *mulle_utf32_strstr( mulle_utf32_t *s1, mulle_utf32_t *s2);
int             mulle_utf32_strncmp( mulle_utf32_t *s1, mulle_utf32_t *s2, size_t len);
size_t          mulle_utf32_strspn( mulle_utf32_t *s1, mulle_utf32_t *s2);
size_t          mulle_utf32_strcspn( mulle_utf32_t *s1, mulle_utf32_t *s2);
static inline int      mulle_utf32_strcmp( mulle_utf32_t *s1, mulle_utf32_t *s2);  // strncmp( s1, s2, -1)
static inline int   mulle_utf32_atoi( mulle_utf32_t *s);
static inline void   mulle_utf32_memcpy( mulle_utf32_t *dst, mulle_utf32_t *src, size_t len);   // len is number of mulle_utf32_t units
static inline void   mulle_utf32_memmove( mulle_utf32_t *dst, mulle_utf32_t *src, size_t len);
```

### 3.8. `mulle-ascii.h`

- Pure-ASCII fast-path converters. `struct mulle_asciidata` is `{ char *characters; size_t length; }` with a `mulle_asciidata_make( s, length)` wrapper (no allocator, no NUL handling).

```c
static inline struct mulle_asciidata   mulle_asciidata_make( char *s, size_t length);
mulle_utf16_t   *_mulle_ascii_convert_to_utf16( const char *src, size_t len, mulle_utf16_t *dst);
mulle_utf32_t   *_mulle_ascii_convert_to_utf32( const char *src, size_t len, mulle_utf32_t *dst);
void   mulle_ascii_bufferconvert_to_utf16( const char *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
void   mulle_ascii_bufferconvert_to_utf32( const char *src, size_t len, void *buffer, mulle_utf_add_bytes_function_t *addbytes);
```

### 3.9. `mulle-utf-convenience.h`

Allocator-based, NUL-terminating high-level converters. Every result is allocated with the passed `struct mulle_allocator` (which may be NULL for the default allocator); free with `mulle_allocator_free`. `len == (size_t) -1` is honored as "NUL-terminated".

```c
mulle_utf16_t   *mulle_utf8_convert_to_utf16_string( const char *src, size_t len, struct mulle_allocator *allocator);
mulle_utf32_t   *mulle_utf8_convert_to_utf32_string( const char *src, size_t len, struct mulle_allocator *allocator);
char    *mulle_utf16_convert_to_utf8_string( const mulle_utf16_t *src, size_t len, struct mulle_allocator *allocator);
mulle_utf32_t   *mulle_utf16_convert_to_utf32_string( const mulle_utf16_t *src, size_t len, struct mulle_allocator *allocator);
char    *mulle_utf32_convert_to_utf8_string( const mulle_utf32_t *src, size_t len, struct mulle_allocator *allocator);
mulle_utf16_t   *mulle_utf32_convert_to_utf16_string( const mulle_utf32_t *src, size_t len, struct mulle_allocator *allocator);
```

Conversion contexts for streaming with proper alignment (fill a `struct mulle_X_conversion_context { buf, sentinel }` and chain these as the `buffer`/`addbytes` pair):

```c
struct mulle_utf8_conversion_context  { char   *buf; char   *sentinel; };
struct mulle_utf16_conversion_context { mulle_utf16_t *buf; mulle_utf16_t *sentinel; };
struct mulle_utf32_conversion_context { mulle_utf32_t *buf; mulle_utf32_t *sentinel; };

void  mulle_utf8_conversion_context_add_bytes( void *p, void *bytes, size_t len);    // len in bytes, bytes properly aligned
void  mulle_utf16_conversion_context_add_bytes( void *p, void *bytes, size_t len);
void  mulle_utf32_conversion_context_add_bytes( void *p, void *bytes, size_t len);
```

Mogrification (character/word transformations driven by user callbacks; e.g. case-folding, trimming, word-splitting):

```c
struct mulle_utf_mogrification_info
{
     mulle_utf32_t (*f1_conversion)( mulle_utf32_t);
     mulle_utf32_t (*f2_conversion)( mulle_utf32_t);
     int           (*is_white)( mulle_utf32_t);
};
```

Mogrifier results: `0` = no conversion took place, `-1` = `dst` was exhausted/dst too small. `_mulle_utf16_character_mogrify` converts UTF-16 input to UTF-32 output (different view!). `src` may equal `dst` only for the safe forms; `dst->length` is adjusted to the actual written length. Incoming data must be valid.

```c
int   _mulle_utf8_character_mogrify( struct mulle_utf8data *dst, struct mulle_utf8data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf16_character_mogrify( struct mulle_utf32data *dst, struct mulle_utf16data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf16_character_mogrify_unsafe( struct mulle_utf16data *dst, struct mulle_utf16data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf32_character_mogrify( struct mulle_utf32data *dst, struct mulle_utf32data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf8_word_mogrify( struct mulle_utf8data *dst, struct mulle_utf8data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf16_word_mogrify( struct mulle_utf32data *dst, struct mulle_utf16data *src, struct mulle_utf_mogrification_info *info);
int   _mulle_utf32_word_mogrify( struct mulle_utf32data *dst, struct mulle_utf32data *src, struct mulle_utf_mogrification_info *info);
```

### 3.10. `mulle-utf-scan.h`

- Decimal scanners for ASCII `0-9` optionally prefixed with `+`/`-`. They advance `p_s` past the scanned digits (`+1` position), never scan more than `len`, and store the value in `p_value`.
- Return values follow `enum mulle_utf_scan_return`: `mulle_utf_has_overflown` (-2, value wrapped an unsigned 64-bit accumulator), `mulle_utf_is_invalid` (-1, not a number at all), `mulle_utf_is_valid` (0, fits a signed long long), `mulle_utf_is_too_large_for_signed` (1, fits unsigned long long only), `mulle_utf_has_trailing_garbage` (2, OR-able with the "valid" results when non-digit characters follow the number).

```c
int   _mulle_utf8_scan_longlong_decimal( char **p_s, size_t len, long long *p_value);
int   _mulle_utf16_scan_longlong_decimal( mulle_utf16_t **p_s, size_t len, long long *p_value);
int   _mulle_utf32_scan_longlong_decimal( mulle_utf32_t **p_s, size_t len, long long *p_value);
```

### 3.11. `mulle-utf-rover.h`

- Uniform, encoding-agnostic code-point iterator. `s`/`sentinel` bound the buffer; `next` returns the next code point; `dialback` rewinds one character (used for lookahead).

```c
struct mulle_utf_rover
{
   void           *s;
   void           *sentinel;
   mulle_utf32_t  (*next)( struct mulle_utf_rover *rover);
   void           (*dialback)( struct mulle_utf_rover *rover);
};

static inline void   *_mulle_utf_rover_get_current( struct mulle_utf_rover *rover);
static inline int   _mulle_utf_rover_has_character( struct mulle_utf_rover *rover);
static inline int   _mulle_utf_rover_next_character( struct mulle_utf_rover *rover);
static inline void   _mulle_utf_rover_dial_back( struct mulle_utf_rover *rover);

void   _mulle_utf32_rover_init( struct mulle_utf_rover *rover, mulle_utf32_t *s, size_t len);
void   _mulle_utf16_rover_init( struct mulle_utf_rover *rover, mulle_utf16_t *s, size_t len);
void   _mulle_utf8_rover_init( struct mulle_utf_rover *rover, char *s, size_t len);
```

### 3.12. `mulle-utf-noncharacter.h`

- Noncharacter detection (U+FDD0..U+FDEF plus the last two code points of every plane) and BOM/surrogate helpers.

```c
int   mulle_utf16_is_noncharacter( mulle_utf16_t c);
int   mulle_utf32_is_noncharacter( mulle_utf32_t c);
int   mulle_utf_is_noncharacterplane( size_t plane);

static inline mulle_utf32_t  mulle_utf32_get_bomcharacter( void);          // 0xFEFF
static inline mulle_utf16_t  mulle_utf16_get_bomcharacter( void);          // 0xFEFF
static inline int  mulle_utf32_is_bomcharacter( mulle_utf32_t c);
static inline int  mulle_utf16_is_bomcharacter( mulle_utf16_t c);
static inline int   mulle_utf32_is_surrogatecharacter( mulle_utf32_t c);   // 0xD800 <= c < 0xE000
static inline int   mulle_utf32_is_highsurrogatecharacter( mulle_utf32_t c);
static inline int   mulle_utf32_is_lowsurrogatecharacter( mulle_utf32_t c);
static inline int   mulle_utf32_is_invalidcharacter( mulle_utf32_t c);     // surrogate OR noncharacter
```

Surrogate-pair encoding/decoding (in-place, `hi`/`lo` are outputs):

```c
static inline void  mulle_utf32_encode_surrogatepair( mulle_utf32_t x, mulle_utf16_t *hi, mulle_utf16_t *lo);
static inline mulle_utf32_t  mulle_utf16_decode_surrogatepair( mulle_utf16_t hi, mulle_utf16_t lo);
```

### 3.13. `mulle-utf-privatecharacter.h`

```c
int   mulle_utf16_is_privatecharacter( uint16_t c);
int   mulle_utf32_is_privatecharacter( int32_t c);
int   mulle_utf_is_privatecharacterplane( size_t plane);
```

### 3.14. `mulle-char5.h`

- char5 packs strings from a 32-character reduced charset (`.`, `A`, `C`, `D`, `E`, `I`, `N`, `O`, `P`, `S`, `T`, `_`, `a`-`y`) into 5-bit slots. 32-bit holds up to 6 chars, 64-bit up to 12. Charset lookup: `mulle_char5_lookup_table[128]` (fill -1 for unsupported chars) and `mulle_char5_get_charset()`.

Encoding/decoding core:

```c
int   mulle_char5_encode_character( int c);
char   mulle_char5_lookup_table[ 128];
static inline int   mulle_char5_lookup_character( int c);        // >= 0 iff encodable
static inline int   mulle_utf8_is_char5character( char c);
enum { mulle_char5_maxlength32 = 6, mulle_char5_maxlength64 = 12 };
static inline char   *mulle_char5_get_charset( void);
static inline int   mulle_char5_decode_character( int c);        // c 0..31
int   mulle_char5_is_char5string32( const char *src, size_t len);  // rejects embedded NULs
int   mulle_char5_is_char5string64( const char *src, size_t len);
uint32_t   mulle_char5_encode32( const char *src, size_t len);
uint64_t   mulle_char5_encode64( const char *src, size_t len);
uint32_t   mulle_char5_encode32_utf16( const mulle_utf16_t *src, size_t len);
uint64_t   mulle_char5_encode64_utf16( const mulle_utf16_t *src, size_t len);
uint32_t   mulle_char5_encode32_utf32( const mulle_utf32_t *src, size_t len);
uint64_t   mulle_char5_encode64_utf32( const mulle_utf32_t *src, size_t len);
size_t   mulle_char5_decode32( uint32_t value, char *dst, size_t len);
size_t   mulle_char5_decode64( uint64_t value, char *src, size_t len);
int   mulle_char5_get64( uint64_t value, size_t index);
int   mulle_char5_get32( uint32_t value, size_t index);
static inline int   mulle_char5_next64( uint64_t *value);        // destructive pop
static inline int   mulle_char5_next32( uint32_t *value);
static inline size_t   mulle_char5_strlen64( uint64_t value);
static inline size_t   mulle_char5_strlen32( uint32_t value);
static inline size_t   mulle_char5_fstrlen64( uint64_t value);   // faster, avoids the loop
static inline size_t   mulle_char5_fstrlen32( uint32_t value);
static inline uint64_t   mulle_char5_substring64( uint64_t value, size_t location, size_t length);
static inline uint32_t   mulle_char5_substring32( uint32_t value, size_t location, size_t length);
```

`uintptr_t` (word-size polymorphic) interface, auto-selects 32/64 depending on `sizeof( mulle_char5_t)`:

```c
static inline int   mulle_char5_is_char5string( const char *src, size_t len);
static inline mulle_char5_t   mulle_char5_encode( const char *src, size_t len);
static inline mulle_char5_t   mulle_char5_encode_utf16( const mulle_utf16_t *src, size_t len);
static inline mulle_char5_t   mulle_char5_encode_utf32( const mulle_utf32_t *src, size_t len);
static inline size_t   mulle_char5_decode( mulle_char5_t value, char *src, size_t len);
static inline int   mulle_char5_get( mulle_char5_t value, size_t index);
static inline int   mulle_char5_next( mulle_char5_t *value);
static inline size_t   mulle_char5_strlen( mulle_char5_t value);
static inline size_t   mulle_char5_fstrlen( mulle_char5_t value);
static inline size_t  mulle_char5_get_maxlength( void);
static inline mulle_char5_t  mulle_char5_substring( mulle_char5_t value, size_t location, size_t length);
static inline uint32_t   _mulle_char5_fnv1a_32( uint32_t value);
static inline uint64_t   _mulle_char5_fnv1a_64( uint64_t value);
static inline uintptr_t   _mulle_char5_fnv1a( uintptr_t value);
```

### 3.15. `mulle-char7.h`

- char7 packs plain 7-bit ASCII strings into 7-bit slots (32-bit holds 4 chars, 64-bit 8). Simpler than char5 (no charset), same family of operations:

```c
enum { mulle_char7_maxlength32 = 4, mulle_char7_maxlength64 = 8 };
int   mulle_char7_is_char7string32( const char *src, size_t len);
int   mulle_char7_is_char7string64( const char *src, size_t len);
uint32_t   mulle_char7_encode32_utf16( const mulle_utf16_t *src, size_t len);
uint64_t   mulle_char7_encode64_utf16( const mulle_utf16_t *src, size_t len);
uint32_t   mulle_char7_encode32_utf32( const mulle_utf32_t *src, size_t len);
uint64_t   mulle_char7_encode64_utf32( const mulle_utf32_t *src, size_t len);
uint32_t   mulle_char7_encode32( const char *src, size_t len);
uint64_t   mulle_char7_encode64( const char *src, size_t len);
size_t   mulle_char7_decode32( uint32_t value, char *dst, size_t len);
size_t   mulle_char7_decode64( uint64_t value, char *src, size_t len);
int   mulle_char7_get64( uint64_t value, size_t index);
int   mulle_char7_get32( uint32_t value, size_t index);
static inline int  mulle_char7_next64( uint64_t *value);
static inline int  mulle_char7_next32( uint32_t *value);
static inline size_t   mulle_char7_strlen64( uint64_t value);
static inline size_t  mulle_char7_strlen32( uint32_t value);
static inline size_t   mulle_char7_fstrlen64( uint64_t value);
static inline size_t   mulle_char7_fstrlen32( uint32_t value);
static inline uint64_t   mulle_char7_substring64( uint64_t value, size_t location, size_t length);
static inline uint32_t   mulle_char7_substring32( uint32_t value, size_t location, size_t length);
// uintptr_t interface:
static inline int   mulle_char7_is_char7string( const char *src, size_t len);
static inline mulle_char7_t   mulle_char7_encode( const char *src, size_t len);
static inline mulle_char7_t   mulle_char7_encode_utf16( const mulle_utf16_t *src, size_t len);
static inline mulle_char7_t   mulle_char7_encode_utf32( const mulle_utf32_t *src, size_t len);
static inline size_t   mulle_char7_decode( mulle_char7_t value, char *src, size_t len);
static inline int   mulle_char7_next( mulle_char7_t *value);
static inline int   mulle_char7_get( mulle_char7_t value, size_t index);
static inline size_t  mulle_char7_strlen( mulle_char7_t value);
static inline size_t  mulle_char7_fstrlen( mulle_char7_t value);
static inline size_t  mulle_char7_get_maxlength( void);
static inline mulle_char7_t  mulle_char7_substring( mulle_char7_t value, size_t location, size_t length);
static inline uint32_t   _mulle_char7_fnv1a_32( uint32_t value);
static inline uint64_t   _mulle_char7_fnv1a_64( uint64_t value);
static inline uintptr_t   _mulle_char7_fnv1a( uintptr_t value);
```

## 4. Performance Characteristics

- Encoding classification, `is_ascii`, and small-string pack/unpack helpers are O(1) inline.
- Conversions, length computations, scans and mogrification are O(n) in the number of input units/code points.
- Low-level converters (`_*_convert_to_*`) are allocation-free: they write into a caller-provided `dst` and only return the end pointer, giving the best raw throughput. Distinguish them from the `*_string` convenience routines which allocate (allocator or default) and NUL-terminate.
- Streaming `*_bufferconvert_*` variants let you feed output into mulle-buffer or aligned conversion contexts without intermediate strings, at the cost of callback-per-chunk overhead.
- char5/char7 encode/decode/strlen/hash operations are constant-time bit operations; they trade very compact representation for a restricted character set (char5: 32-char subset; char7: ASCII only).
- Validation is separate from conversion: skipping validation speeds up fast paths but then the caller must guarantee well-formed input (low-level/streaming converters do not check).
- Thread-safety: the library has no global mutable state (the only shared symbol is the read-only `mulle_char5_lookup_table`). Use a thread-safe allocator (or external locking) when allocating concurrently.
- Many functions are tagged `// fuzzed` in the headers; `fuzz/` contains harnesses run in CI (sanitizer, 32-bit, fuzz jobs).

## 5. AI Usage Recommendations & Patterns

- **Best practices:**
  - Validate once before heavy processing: `mulle_utf8_information( src, len, &info)` fills `utf8len/utf16len/utf32len` in one pass; verify with `mulle_utf_information_is_valid( &info)` (i.e. `info.invalid == NULL`). `mulle_utf8_validate()` returns NULL when OK, else the offending character address.
  - Compute exact destination sizes with `mulle_utf*_utf*length()` (or `info.utf*len`) before calling the low-level converters; use the `utf8maxlength`/`utf16maxlength` upper bounds when you would rather overallocate by 4x.
  - Prefer the allocator-based `_string` converters for simple code; they append a trailing NUL and allocate via the provided `struct mulle_allocator` (NULL = default). Free results with `mulle_allocator_free( allocator, p)`.
  - Pass `len == (size_t) -1` for NUL-terminated inputs to length/validate and the `_string` converters. Never pass `-1` to the low-level `_mulle_*_convert_to_*` routines (no NUL scanning there).
  - Use `struct mulle_utf_rover` (via `_mulle_utf8_rover_init`/`_mulle_utf16_rover_init`/`_mulle_utf32_rover_init`) to write encoding-agnostic iteration code.
  - Use char7 for arbitrary ASCII keys and char5 for the smaller reduced charset; both fit in a machine word and hash directly with `_mulle_char7_fnv1a` / `_mulle_char5_fnv1a`.
  - For streaming without allocation churn, pair a conversion context with its `*_conversion_context_add_bytes` as the `addbytes` callback, filling `buf`/`sentinel`.
- **Common pitfalls:**
  - Do not feed unvalidated data to low-level or streaming converters; they may read past logical buffers or mishandle BOMs. `mulle_utf8_bufferconvert_to_utf16`/`_utf32` explicitly "do not skip BOM characters and don't check for validity".
  - Truncated UTF-8: `mulle_utf8_utf16length` now reports truncation via `(size_t) -1`; check for it instead of assuming 0.
  - UTF-16 surrogate pairs: a lone high surrogate at end-of-buffer is invalid (fixed in 6.0.0 to not read past the end). Prefer UTF-32 for simple character-level logic.
  - `mulle_utf32_t` is signed deliberately; `-1` is an error/end sentinel from iteration (`mulle_utf8data_next_utf32character` returns `-1` at end, `-2` if malformed) and never a valid code point.
  - The `*_string` converters return freshly allocated buffers; the low-level converters return pointers into your own `dst`. Do not mix ownership models.
  - `mulle_utf16_memcpy`/`memmove`, `strnlen` etc. count in *units* (`mulle_utf16_t`/`mulle_utf32_t` elements), not bytes.
  - char5/char7 `is_char5string`/`is_char7string` reject strings that contain embedded NUL bytes; `encode`/`decode`/`strlen` treat a zero slot as end-of-string anyway ("whatever is_char5string accepts survives a round trip").
  - `mulle_utf8data_init` with `length == 0` does not copy; it points at a static `""`. `mulle_utf8data_done` only frees non-empty buffers.
  - `_mulle_utf16_character_mogrify`/`_mulle_utf16_word_mogrify` convert between different data types (UTF-16 src into a UTF-32 dst); don't assume matching element types.
- **Idiomatic usage (mulle-sde way):** wrap buffers in `mulle_utfXdata` structs via the `_make` helpers for uniform handling; allocate the real data with `mulle_utf8data_init` only when you need an owned, NUL-terminated copy; destroy with `mulle_utf8data_done` using the same allocator.

## 6. Integration Examples

### Example 1: Round-trip a UTF-32 code point through UTF-8 using low-level converters

```c
#include <mulle-utf/mulle-utf.h>

int   main( void)
{
   mulle_utf32_t   input[ 1];
   mulle_utf32_t   back[ 1];
   char            utf8buf[ 8];
   char            *utf8end;
   mulle_utf32_t   *backend;

   input[ 0] = 0x1F600;   /* U+1F600 GRINNING FACE */

   utf8end = _mulle_utf32_convert_to_utf8( input, 1, utf8buf);
   backend = _mulle_utf8_convert_to_utf32( utf8buf,
                                           (size_t)( utf8end - utf8buf),
                                           back);

   return( (backend - back) != 1 || back[ 0] != 0x1F600);
}
```

### Example 2: Analyze a UTF-8 buffer and iterate it as code points with a rover

```c
#include <mulle-utf/mulle-utf.h>

int   main( void)
{
   struct mulle_utf_information   info;
   struct mulle_utf_rover         rover;
   mulle_utf32_t                  c;
   int                            count;

   /* "hi" = h i */
   if( mulle_utf8_information( "hi", 2, &info))
      return( 1);
   if( ! mulle_utf_information_is_valid( &info))
      return( 1);

   count = 0;
   _mulle_utf8_rover_init( &rover, info.start, info.utf8len);
   while( _mulle_utf_rover_has_character( &rover))
   {
      c = _mulle_utf_rover_next_character( &rover);
      ++count;
   }

   return( count != 2);
}
```

### Example 3: Pack a small ASCII token with char7 and hash it

```c
#include <mulle-utf/mulle-utf.h>

int   main( void)
{
   mulle_char7_t   value;
   uintptr_t       hash;

   if( ! mulle_char7_is_char7string( "Token", 5))
      return( 1);

   value = mulle_char7_encode( "Token", 5);
   hash  = _mulle_char7_fnv1a( value);

   return( hash == 0);
}
```

(More working examples live under `test/`: `conversion/roundtrip-boundaries`, `conversion/astral-utf16-to-utf8`, `information/validate-agrees`, `information/utf16-validate-truncated`, `information/utf16-utf8len`, `range/utf8data-range`, `scan/overflow`, `strnstr/overlapping`, `char5/zero-byte`, plus pre-existing `backandforth*`, `mogrify/*`, `strchr`, `strncpy`, `strspn`, `strcspn`, `length/*`, `iteration`.)

## 7. Dependencies

Direct `mulle-sde` library dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-allocator`  (minimum version 8.1.0, checked via `_mulle-utf-versioncheck.h`) — allocator API used by the convenience/`_string`/`utf8data` helpers.
- `mulle-data`       (minimum version 0.6.0) — `struct mulle_data` / `struct mulle_range` interop (`mulle_data_as_utf8data`, `mulle_utf8data_range_of_utf32_range`, `mulle_utf8data_characters_in_range`).