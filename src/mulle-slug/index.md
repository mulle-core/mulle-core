# mulle-slug Library Documentation for AI
<!-- Keywords: slug, utf8, unicode, transliteration, buffer, url, normalization -->

## 1. Introduction & Purpose

mulle-slug converts arbitrary text into URL-safe slugs. A "slug" is a clean,
human-readable URL component such as `Take-me-home` derived from the title
"Take me home!". The library handles Unicode input, transliterates accented
letters and non-Latin scripts (Latin, Greek, Cyrillic, currency symbols, and
more) to ASCII, strips combining marks, and collapses whitespace/punctuation
runs into a single delimiter. Unlike many slug libraries, it **preserves case**.

Key capabilities:

- ASCII-only slugification that drops non-transliterable characters
  (CJK, Arabic, etc.) — the default.
- A "UTF-8 passthrough" variant that keeps non-transliterable letters as
  UTF-8 instead of dropping them.
- Configurable delimiter character (default `-`).
- Zero-malloc buffer building blocks for assembling URLs/HTML fragments.
- Convenience functions that write into a caller-provided buffer
  (heap, stack, or `alloca`), with no allocation if the output fits.
- Transliteration based on a hand-curated table plus an auto-generated
  Unicode canonical/compatibility decomposition table (generated from
  UnicodeData.txt), covering far more characters than a plain ASCII map.

mulle-slug is a component of the [`mulle-core`](https://github.com/mulle-core/mulle-core)
library and is a C port of C++ Slugify with a different algorithm.

## 2. Key Concepts & Design Philosophy

**Slug concept:** A slug is the URL-safe representation of a page title.
mulle-slug does *not* lowercase the input (unlike most slug libraries);
`"Hello World!"` becomes `"Hello-World"`, not `"hello-world"`.

**Two output modes:**

1. **ASCII drop mode** (default): every character either becomes an ASCII
   equivalent or is dropped. Non-transliterable characters (CJK, Arabic,
   emoji, control chars, combining marks) are removed.
2. **UTF-8 passthrough mode**: characters that cannot be transliterated are
   emitted as-is in UTF-8, as long as they are not whitespace, punctuation,
   or control characters.

**Transliteration pipeline** (per decoded UTF-32 character):

1. ASCII fast path: whitespace/`-` runs collapse into a single delimiter;
   `<` `>` `$` `#` map to `less` `greater` `dollar` `hash`; other punctuation
   becomes a delimiter; non-printable ASCII is dropped.
2. Combining marks (`mulle_unicode_is_nonbase`, e.g. U+0300) are skipped, so
   decomposed input (`e` + combining acute) yields `e` like precomposed `é`.
3. Lookup in the hand-curated semantic table (`map.inc`), falling back to the
   Unicode decomposition table (`decompose-map.inc`).
4. If no mapping and passthrough mode: emit the character as UTF-8.

**Delimiter hygiene:** leading delimiters are never emitted, runs of
whitespace/punctuation collapse to a single delimiter, and trailing
delimiters are trimmed.

**Buffer-centric design:** the core worker is the buffer API; the string
convenience functions are thin wrappers over it. `mulle_slugify*` use a
flexible buffer over caller storage and only allocate if the output would
overflow the destination (in which case the result is truncated to fit).

## 3. Core API & Data Structures

All public symbols live in the single public header `src/mulle-slug.h`
(reachable as `<mulle-slug/mulle-slug.h>`). The library integrates with
dependency-owned types `struct mulle_utf8data` (mulle-utf),
`struct mulle_buffer` (mulle-buffer) and `struct mulle_allocator`
(mulle-allocator).

### 3.1. `src/mulle-slug.h`

#### Version information

```c
#define MULLE__SLUG_VERSION  ((0UL << 20) | (2 << 8) | 0)

MULLE__SLUG_GLOBAL
uint32_t   mulle_slug_get_version( void);

static inline uint32_t   mulle_slug_get_version_major( void)
static inline uint32_t   mulle_slug_get_version_minor( void)
static inline uint32_t   mulle_slug_get_version_patch( void)
```

- **Purpose:** Runtime version access; the current version is 0.2.0.
- `mulle_slug_get_version()` returns the packed `MULLE__SLUG_VERSION`.
- The `_major`/`_minor`/`_patch` variants decode individual components and
  are declared `static inline` in the header (no `MULLE__SLUG_GLOBAL`
  prefix on those declarations).

#### String convenience functions (allocating)

```c
MULLE__SLUG_GLOBAL
char   *mulle_utf8_slugify( const char *s);

MULLE__SLUG_GLOBAL
char   *mulle_utf8_slugify_utf8( const char *s);
```

- **Purpose:** Slugify a NUL-terminated UTF-8 C string into a newly allocated
  string that the caller must free with `mulle_free()`.
- Both return `NULL` if `s` is `NULL`, and return an allocated empty string
  for `""`.
- `mulle_utf8_slugify()` uses **ASCII drop mode** (non-transliterable
  characters like CJK/Arabic are dropped; output is ASCII-only).
- `mulle_utf8_slugify_utf8()` uses **UTF-8 passthrough mode** (non-Latin
  letters are kept as UTF-8).
- Note the parameters are `const char *` (const-qualified).

```c
MULLE__SLUG_GLOBAL
struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                struct mulle_allocator *allocator);
```

- **Purpose:** Slugify `data` (explicit length, may contain bytes past a NUL;
  it is consumed until its first NUL byte or `length` is exhausted) into a
  `struct mulle_utf8data`. ASCII drop mode.
- The returned struct's `characters` must be freed; use the same
  `allocator` used for allocation (or the default when `allocator` is
  `NULL`).
- **NOTE:** the returned `length` includes the trailing NUL byte.
- The buffer is created via `mulle_buffer_do_allocator( buffer, allocator)`.

#### Buffer integration functions

```c
MULLE__SLUG_GLOBAL
void  mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                           struct mulle_utf8data data);

MULLE__SLUG_GLOBAL
void  mulle_buffer_add_utf8_slugified_utf8data( struct mulle_buffer *buffer,
                                                struct mulle_utf8data data);

MULLE__SLUG_GLOBAL
void  mulle_buffer_add_slugified_utf8data_with_delimiter( struct mulle_buffer *buffer,
                                                          struct mulle_utf8data data,
                                                          char delimiter);

MULLE__SLUG_GLOBAL
void  mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer,
                                     struct mulle_utf8data data);
```

- **Purpose:** Append (or in-place slugify) a slugified form of `data` onto an
  already-initialized `struct mulle_buffer`; the buffer grows as needed.
  Useful for building up an HTML page/URL incrementally.
- `mulle_buffer_add_slugified_utf8data()` — ASCII drop mode, delimiter `-`.
- `mulle_buffer_add_utf8_slugified_utf8data()` — UTF-8 passthrough mode,
  delimiter `-`.
- `mulle_buffer_add_slugified_utf8data_with_delimiter()` — ASCII drop mode
  with a caller-chosen `delimiter` character.
- `mulle_buffer_slugify_utf8data()` — ASCII drop mode, then calls
  `mulle_buffer_make_string()` so the buffer holds a NUL-terminated C string.
- All three `add_*` functions share an internal worker
  `_mulle_buffer_add_slugified_utf8data( buffer, data, delimiter, passthru)`;
  slugification stops at the first NUL byte in `data`.

#### Convenience functions writing into a caller-provided buffer

```c
MULLE__SLUG_GLOBAL
char  *mulle_slugify( char *dst, size_t dst_len, const char *src, size_t src_len);

MULLE__SLUG_GLOBAL
char  *mulle_slugify_with_delimiter( char *dst, size_t dst_len,
                                     const char *src, size_t src_len,
                                     char delimiter);
```

- **Purpose:** Slugify `src` directly into the caller-provided `dst` buffer of
  `dst_len` bytes. ASCII drop mode.
- Returns `dst` on success, `NULL` if `dst_len` is `0`.
- If `src` is `NULL`, writes `NUL` into `dst[0]` and returns `dst`.
- `src_len` may be `(size_t) -1` to slugify a NUL-terminated `src`
  (the implementation uses `strlen` in that case).
- Internally uses `mulle_buffer_do_flexible( buffer, dst, dst_len)`: while the
  slug fits in `dst` no allocation occurs; if it overflows into a malloc'd
  buffer, the result is truncated and copied back into `dst`
  (`dst_len - 1` bytes max + NUL).
- `mulle_slugify()` is `mulle_slugify_with_delimiter( ..., '-')`.
- Example outputs (from `mulle-slugify` test): `"Hello World!"` → `Hello-World`;
  with delimiter `'_'` → `Hello_World`; `"$100 < $200"` → `dollar100-less-dollar200`;
  smart quotes are removed (`"Hello"` → `Hello`).

## 4. Performance Characteristics

- **Time:** single pass over the input, O(n) overall.
  - ASCII path is O(1) per byte (switch on table of specials plus
    `isprint`/`ispunct`).
  - Non-ASCII path per character: O(log m) binary search over the
    hand-curated `map.inc` (≈200 entries) first, then the sorted
    `decompose-map.inc` (1664 entries). Both tables are searched only when
    the hand-curated lookup misses.
- **Space:** zero extra allocation for the transliteration itself (static
  readonly lookup tables). Memory depends on the chosen API:
  - `mulle_slugify*` into a caller buffer: zero malloc while output fits
    `dst_len`; otherwise one transient buffer allocation, then inline
    truncation.
  - `mulle_utf8_slugify*` / `mulle_utf8data_slugify`: one buffer allocation
    per call.
  - `mulle_buffer_add_*`: amortized O(1) appends onto the existing buffer.
- **Trade-offs:** comprehensiveness of the decomposition table adds binary
  size; the two binary searches add a small constant factor to the non-ASCII
  slow path; passthrough mode trades URL-safety for script preservation.
- **Thread-safety:** the lookup tables are initialized statically and never
  mutated, and no global mutable state exists (`__MULLE_SLUG_ranlib__` is
  inert), so independent calls on separate buffers are thread-safe.

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Pick the right mode deliberately:** use the default ASCII mode
  (`mulle_utf8_slugify`, `mulle_slugify`, `mulle_buffer_add_slugified_utf8data`)
  for URL paths/fragments. Use the `_utf8`/passthrough variants
  (`mulle_utf8_slugify_utf8`, `mulle_buffer_add_utf8_slugified_utf8data`)
  only when non-Latin scripts must survive (e.g. multilingual display text).
- **Use the buffer API when assembling strings:** `mulle_buffer_add_slugified_utf8data`
  is the zero-malloc building block for constructing a URL or HTML page from
  multiple pieces.
- **Prefer `mulle_slugify` for bounded output:** pass a stack or `alloca`
  buffer and a computed `dst_len`; no heap allocation occurs while the slug
  fits. `mulle_buffer_do_flexible` handles overflow by truncation.
- **Pass `(size_t) -1` as `src_len`** to slugify a NUL-terminated string
  (saves an explicit `strlen`).
- **Remember the case is preserved:** lowercase explicitly after slugifying if
  your URL scheme requires lowercase slugs.

### Common Pitfalls

- **Free the allocated results.** `mulle_utf8_slugify` and
  `mulle_utf8_slugify_utf8` return an allocated string for `mulle_free`. The
  `characters` of the `struct mulle_utf8data` returned by
  `mulle_utf8data_slugify` must also be freed (with the matching allocator).
- **`mulle_utf8data_slugify` length includes the NUL:** the returned
  `length` counts the trailing `NUL` byte, so it is `strlen`-like value + 1.
- **Slugification stops at a NUL byte** inside `data` even if
  `data.length` is larger.
- **Do not hand in an uninitialized buffer:** the buffer functions require an
  initialized `struct mulle_buffer`.
- **`mulle_slugify` returns `NULL` if `dst_len == 0`** — check before
  dereferencing `dst`.
- **Case is preserved:** `"Hello World!"` → `Hello-World`, not
  `hello-world`.
- **Don't rely on specific expansions:** non-Latin text in ASCII mode is
  dropped entirely (e.g. `"汉字"` → empty string) unless the passthrough
  variant is used.

### Idiomatic Usage

Integrate with the mulle-buffer `mulle_buffer_do` macro for scoped buffers,
and with mulle-utf `mulle_utf8data_make` to build `struct mulle_utf8data`
values. See the `test/` directory for the canonical patterns
(`test/20-extensive/mulle-slugify.c`, `slugify-buffer.c`,
`slugify.c`, `slugify-empty.c`).

## 6. Integration Examples

### Example 1: Slugify a string, free the result

```c
#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   char   *slug;

   slug = mulle_utf8_slugify( "VfL Bochum 1848");
   printf( "%s\n", slug);

   mulle_free( slug);

   return( 0);
}
// Output: VfL-Bochum-1848
```

### Example 2: Write the slug into a caller-provided buffer

```c
#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   char   buf[ 256];
   char   *result;

   // explicit src_len
   result = mulle_slugify( buf, sizeof( buf), "Hello World!", 12);
   printf( "%s\n", result);

   // NUL-terminated src
   result = mulle_slugify( buf, sizeof( buf), "Take me home!", (size_t) -1);
   printf( "%s\n", result);

   // custom delimiter
   result = mulle_slugify_with_delimiter( buf, sizeof( buf), "Hello World!", 12, '_');
   printf( "%s\n", result);

   // NULL src yields an empty string in dst
   result = mulle_slugify( buf, sizeof( buf), NULL, 0);
   printf( "%s\n", result);

   // NULL src, zero-length destination
   result = mulle_slugify( buf, 0, "Hello World!", 12);
   printf( "%p\n", result);

   return( 0);
}
// Output:
// Hello-World
// Take-me-home
// Hello_World
//
// (nil)
```

### Example 3: Build a URL incrementally with a buffer

```c
#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   struct mulle_utf8data   data;

   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "/articles/");
      data = mulle_utf8data_make( "Unicode in Modern Languages", -1);
      mulle_buffer_add_slugified_utf8data( buffer, data);
      mulle_buffer_add_string( buffer, ".html");
      printf( "%s\n", mulle_buffer_get_string( buffer));
      mulle_buffer_reset( buffer);
   }

   return( 0);
}
// Output: /articles/Unicode-in-Modern-Languages.html
```

### Example 4: ASCII drop vs. UTF-8 passthrough for non-Latin text

```c
#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   char   *ascii;
   char   *utf8;

   // CJK characters cannot be transliterated:
   // ASCII mode drops them, passthrough mode keeps them as UTF-8
   ascii = mulle_utf8_slugify( "\xe6\xb1\x89\xe5\xad\x97");         /* "汉字" */
   printf( "[%s]\n", ascii);

   utf8 = mulle_utf8_slugify_utf8( "\xe6\xb1\x89\xe5\xad\x97");     /* "汉字" */
   printf( "[%s]\n", utf8);

   // Cyrillic transliterates in both modes
   utf8 = mulle_utf8_slugify_utf8( "\xd0\x9c\xd0\xbe\xd1\x81\xd0\xba\xd0\xb2\xd0\xb0");  /* "Москва" */
   printf( "[%s]\n", utf8);

   mulle_free( ascii);
   mulle_free( utf8);

   return( 0);
}
// Output:
// []
// [汉字]
// [Moskva]
```

### Example 5: Slugify into an existing buffer with a delimiter

```c
#include <mulle-slug/mulle-slug.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   mulle_buffer_do( buffer)
   {
      // both empty inputs add nothing
      mulle_buffer_add_slugified_utf8data( buffer, (struct mulle_utf8data){ 0 });
      mulle_buffer_add_slugified_utf8data( buffer, mulle_utf8data_make( NULL, 0));
      // custom delimiter '_' instead of '-'
      mulle_buffer_add_slugified_utf8data_with_delimiter( buffer,
                                                           mulle_utf8data_make( "Hello World!", 12),
                                                           '_');
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }

   return( 0);
}
// Output: Hello_World
```

## 7. Dependencies

Direct mulle-sde library dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-utf` — `mulle_utf8data`, UTF-8 decoding
  (`mulle_utf8_next_utf32character`), UTF-8 encoding
  (`mulle_utf32_bufferconvert_to_utf8`).
- `mulle-buffer` — `struct mulle_buffer` and its `mulle_buffer_do*` macros
  used for all output accumulation.
- `mulle-unicode` — character classification
  (`mulle_unicode_is_nonbase`, `mulle_unicode_is_whitespace`,
  `mulle_unicode_is_punctuation`, `mulle_unicode_is_control`).

mulle-slug is a component of the `mulle-core` amalgamation; include
`<mulle-core/mulle-core.h>` in downstream code instead of adding mulle-slug
directly.