# mulle-utf Library Documentation for AI
<!-- Keywords: unicode, utf8, utf16, utf32, conversion, char5, char7 -->

## 1. Introduction & Purpose

- mulle-utf is a small C (C99) Unicode analysis and conversion library. It provides validation, conversion, and primitive string utilities for UTF-8, UTF-16 and UTF-32 and compact encodings (char5/char7).
- Solves: detect/validate encodings, convert between UTF encodings, perform Unicode-aware string operations, and compactly encode short ASCII subsets.
- Key features: validation helpers, length calculators, low-level fast converters, convenience allocators, rover iterator, char5/char7 packing.
- Relationship: component of the mulle-core family; depends on mulle-allocator and mulle-data for allocation and hash/lookup utilities.

## 2. Key Concepts & Design Philosophy

- Types: uses explicit fixed-width types: mulle_utf16_t (uint16_t) and mulle_utf32_t (int32_t, signed so -1 is error). char5/char7 compress small strings into uintptr_t.
- Two-layer API: low-level, unchecked conversion primitives (fast, require preallocated buffers) and convenience "_string" functions that allocate and NUL-terminate results.
- Validation-first: many routines expect sane/validated input. Validate once (mulle_utf8_validate / mulle_utf8_information) then reuse for fast paths.
- Rover abstraction: a small iterator (mulle_utf_rover) to walk different encodings uniformly.
- Compact encodings: mulle_char5 and mulle_char7 provide fast packing/decoding for small ASCII subsets; useful for keys and hashing.

## 3. Core API & Data Structures

### 3.1. [mulle-utf-type.h]

struct mulle_utf_information
- Purpose: describe a buffer's encoding and derived lengths.
- Key fields: utf8len, utf16len, utf32len, start, invalid (first invalid char), has_bom, is_ascii, is_char5, is_utf15.
- Helpers: mulle_utf_information_is_valid(info).

Enums & typedefs
- mulle_utf32_t: signed 32-bit code point, mulle_utf32_max = 0x10FFFF.
- mulle_utf_add_bytes_function_t: callback for streaming conversion into buffers.
- mulle_utf_scan_return: return codes for scanning helpers.

### 3.2. [mulle-utf8.h]

struct mulle_utf8data
- Purpose: represent a byte buffer (+ length) for UTF-8 operations.
- Fields: char *characters; size_t length;
- Lifecycle: mulle_utf8data_make, mulle_utf8data_make_empty, mulle_utf8data_make_invalid, mulle_utf8data_init(data, chars, len, allocator), mulle_utf8data_done(data, allocator), mulle_utf8data_copy.
- Core ops:
  - Validation & info: mulle_utf8_validate(src,len) -> NULL if ok, mulle_utf8_information(src,len, &info), mulle_utf8_is_ascii.
  - Conversion lengths: mulle_utf8_utf16length, mulle_utf8_utf32length.
  - Iteration: mulle_utf8_next_utf32character (and internal _mulle_utf8_next_utf32character), data-rover helpers like mulle_utf8data_next_utf32character.
  - Low-level converters: _mulle_utf8_convert_to_utf16/_utf32 (write into provided dst), bufferconvert_to_utf16/utf32 (use add_bytes callback).
  - Convenience high-level allocators: mulle_utf8_convert_to_utf16_string(src,len,allocator), mulle_utf8_convert_to_utf32_string(...)
- String-like helpers: mulle_utf8_strncpy, mulle_utf8_strnstr, mulle_utf8_strnchr, mulle_utf8_strspn/strcspn and memcpy/memmove wrappers tailored for UTF-8.

### 3.3. [mulle-utf16.h / mulle-utf16-string.h]

struct mulle_utf16data
- Purpose: container for UTF-16 buffers and lengths.
- Lifecycle: mulle_utf16data_make, mulle_utf16_strdup, mulle_utf16_strncpy, mulle_utf16_strnlen.
- Core ops:
  - Validation/info: mulle_utf16_validate, mulle_utf16_information
  - Lengths & conversions: mulle_utf16_utf8length, mulle_utf16_utf32length, mulle_utf16_utf8maxlength
  - Iteration: _mulle_utf16_next_utf32character / previous
  - Low-level convert: _mulle_utf16_convert_to_utf32 / _mulle_utf16_convert_to_utf8
  - Buffer streaming: mulle_utf16_bufferconvert_to_utf8 / to_utf32 with add_bytes callback
  - Surrogate handling: mulle_utf16_is_valid_surrogatepair
  - charinfo: _mulle_utf16_charinfo

### 3.4. [mulle-utf32.h / mulle-utf32-string.h]

struct mulle_utf32data
- Purpose: container for UTF-32 buffers. Useful for word-level and character-level operations.
- Core ops:
  - Information & validation: mulle_utf32_information, mulle_utf32_validate
  - Conversions: mulle_utf32_utf8length, mulle_utf32_utf16length, _mulle_utf32_convert_to_utf16/utf8
  - Iteration helpers: _mulle_utf32_next_utf32character/_previous
  - Buffer streaming: mulle_utf32_bufferconvert_to_utf8/utf16
  - charinfo: _mulle_utf32_charinfo

### 3.5. [mulle-utf-scan.h]

- Decimal scanning helpers for ASCII digits across encodings: _mulle_utf8_scan_longlong_decimal, _mulle_utf16_scan_longlong_decimal, _mulle_utf32_scan_longlong_decimal. Return codes indicate overflow, invalid, or trailing garbage.

### 3.6. [mulle-utf-rover.h]

struct mulle_utf_rover
- Purpose: uniform iterator over UTF buffers (utf8/16/32). Has function pointers next/dialback and pointers s/sentinel.
- Init helpers: _mulle_utf8_rover_init, _mulle_utf16_rover_init, _mulle_utf32_rover_init.

### 3.7. [mulle-char5.h / mulle-char7.h]

- Purpose: compactly encode short strings into uintptr_t (char5) or into 7-bit packed words (char7).
- Core functions: mulle_char5_encode / mulle_char7_encode (utf8, utf16, utf32 variants), is_char5/is_char7string checks, encode32/64 and decode32/64, getters get/next/substring, length helpers, FNV hashing helpers.
- Use-case: fast key encoding for hash tables or small-key comparisons.

### 3.8. [mulle-ascii.h]

- Low-level helpers to convert ASCII to UTF-16/32 and streaming versions (bufferconvert) for ASCII inputs.

### 3.9. [mulle-utf-convenience.h]

- High-level convenience allocation routines that append NUL and allocate with provided allocator. Mogrification helpers (character/word transformations) that apply user-provided conversion/is_white callbacks; used for case-folding, trimming, and word transformations.

## 4. Performance Characteristics

- Most operations are linear time O(n) in input length for full-buffer conversions or scans.
- Many inline helpers (is_ascii, get start-type, small string pack/unpack) are O(1) and optimized.
- Low-level conversion routines assume preallocated destination buffers; this avoids repeated allocations and improves throughput.
- Trade-offs: validation is separate from fast conversion — skipping validation speeds up but requires callers to ensure correctness.
- Thread-safety: library is not internally synchronized. Memory allocation depends on provided allocator; use thread-safe allocator or external synchronization if used concurrently.

## 5. AI Usage Recommendations & Patterns

- Best practices:
  - Validate input once with mulle_utf?_validate() before heavy processing.
  - Prefer convenience *_string functions for simple code (they allocate and NUL-terminate). Use bufferconvert + add_bytes callback for streaming or zero-copy.
  - Use rover (mulle_utf_rover) when writing encoding-agnostic iterators.
  - Use char5/char7 for compact keys and hashing of small ASCII-like tokens.
- Common pitfalls:
  - Do not pass unvalidated data to low-level converters; they may read past buffers or mis-handle BOMs.
  - Returned pointers from low-level routines often point into user buffers; do not free them. Use provided copy helpers when ownership is needed.
  - Beware UTF-16 surrogate pairs: prefer UTF-32 for simple character-level logic.

## 6. Integration Examples

### Example 1: Converting UTF8 to UTF16 string

```c
#include "mulle-utf.h"

char *src;
size_t len;
mulle_utf16_t *utf16;

// assume src/len are set and src is valid UTF-8
utf16 = mulle_utf8_convert_to_utf16_string( src, len, NULL /*allocator*/);
if( ! utf16)
{
   // handle out of memory
}
// use utf16 ...
// allocator-specific free required (not shown)
```

### Example 2: Finding a UTF8 substring (safe, length-aware)

```c
#include "mulle-utf.h"

char *haystack;
char *needle;
char *found;

found = mulle_utf8_strnstr( haystack, (size_t) -1, needle);
if( found)
{
   // found points into haystack at start of match
}
```

### Example 3: Encode short ASCII key with char5

```c
#include "mulle-utf.h"

mulle_char5_t key;

if( mulle_char5_is_char5string( "Token", 5))
{
   key = mulle_char5_encode( "Token", 5);
   // key can be used as a compact lookup key
}
```

(Tests under test/ show many real usages: conversion/backandforth, strstr/strchr/strncpy variations and mogrify examples.)

## 7. Dependencies

- Direct components referenced in README and headers:
  - mulle-allocator  -- allocator API used by convenience allocation helpers
  - mulle-data       -- string/hash utilities used for lookup tables
  - mulle-core / mulle-sde (integration/runtime tooling)


---

References: public headers under src/ and tests under test/ provide canonical usage examples (conversion/, strstr/, strchr/, mogrify/). See README.md and dox/ for generated API pages.
