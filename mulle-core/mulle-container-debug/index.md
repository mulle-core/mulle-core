# mulle-container-debug Library Documentation for AI
<!-- Keywords: debugging, describe, pointerarray, container, buffer, inspection -->

## 1. Introduction & Purpose

- `mulle-container-debug` provides `describe` functions for the data structures
  of `mulle-container`. A describe function builds a human-readable string that
  visualizes the contents of a container, for use during development and debugging.
- Currently only `mulle-pointerarray` (and its internal representation) has an
  implemented describe facility. All other `mulle-container` types
  (`array`, `map`, `set`, `queue`, `assoc`, `rangeset`, ...) have scaffolding
  headers present in this project but no functions implemented yet.
- It is a foundational part of `mulle-core` and a companion to `mulle-container`.
  It is intentionally kept outside `mulle-c` because it builds on `mulle-buffer`
  and `mulle-sprintf`, and `mulle-sprintf` in turn pulls in `mulle-thread`.
- Version 0.1.4.

## 2. Key Concepts & Design Philosophy

- **Describe functions**: Every implemented container gets a function named
  `mulle_<type>_describe` that returns a freshly malloc'd string describing the
  container contents.
- **Buffer-based rendering**: Rendering happens into a `mulle_buffer`
  (`mulle-buffer`). The `_describe` variant is implemented in terms of a
  `_describe_buffer` variant wrapped in the `mulle_buffer_do_string` macro.
- **Default vs. custom item printing**: Elements are rendered with a default
  printer that emits `%p` (the raw pointer). A caller can pass a custom
  `mulle_container_item_printer_t` callback (plus a `userinfo` argument) that is
  invoked per element with a `mulle_buffer` to write into.
- **Two-tier naming scheme**: Private implementation functions are prefixed with
  `mulle__` (double underscore) and operate on the private struct
  `struct mulle__pointerarray`. Public thin static-inline wrappers are prefixed
  with `mulle_` (single underscore) and accept the public
  `struct mulle_pointerarray`, casting internally.
- **Output format**: A NULL array renders as `NULL`. An empty array renders as
  `{}`. A single-element array renders as `{ %p }`. A multi-element array renders
  as a brace-enclosed, comma-separated, indented multi-line block.
- **Read-only, non-invasive**: Describe functions only traverse and print; they
  never modify the container.

## 3. Core API & Data Structures

### 3.1. `src/mulle-container-debug.h` (library entry point)

Top-level header. Defines the version and pulls in every public header via the
auto-generated `_mulle-container-debug-provide.h`.

```c
#define MULLE__CONTAINER__DEBUG_VERSION  ((0UL << 20) | (1 << 8) | 4)

static inline unsigned int   mulle_container_debug_get_version_major( void)
static inline unsigned int   mulle_container_debug_get_version_minor( void)
static inline unsigned int   mulle_container_debug_get_version_patch( void)

MULLE__CONTAINER__DEBUG_GLOBAL
uint32_t   mulle_container_debug_get_version( void);
```

- `MULLE__CONTAINER__DEBUG_VERSION` — packed `major.minor.patch` (0.1.4).
- `mulle_container_debug_get_version_major/minor/patch()` — static inline
  accessors unpacking the version fields.
- `mulle_container_debug_get_version()` — returns the packed version as `uint32_t`.

### 3.2. `src/array/pointer/mulle--pointerarray-debug.h` (implementation)

The only implemented describe facility. Declares the item printer type and the
internal functions operating on the private `struct mulle__pointerarray`
(type defined in `mulle-container`).

```c
typedef void   mulle_container_item_printer_t( struct mulle_buffer *buffer,
                                               void *item,
                                               void *userinfo);

MULLE__CONTAINER__DEBUG_GLOBAL
void   mulle__pointerarray_describe_buffer_callback( struct mulle__pointerarray *array,
                                                     struct mulle_buffer *buffer,
                                                     mulle_container_item_printer_t *callback,
                                                     void *userinfo);

// use this only for debugging
MULLE__CONTAINER__DEBUG_GLOBAL
void   mulle__pointerarray_describe_buffer( struct mulle__pointerarray *array,
                                            struct mulle_buffer *buffer);

MULLE__CONTAINER__DEBUG_GLOBAL
char   *mulle__pointerarray_describe( struct mulle__pointerarray *array);
```

- **`mulle_container_item_printer_t`** — function type for rendering a single
  item into a `mulle_buffer`. `callback` may be NULL, which selects the default
  printer (renders `%p`, i.e. the pointer hex value). `userinfo` is passed
  through untouched.
- **`mulle__pointerarray_describe_buffer_callback(...)`** — core routine. Writes
  the description of `array` into `buffer`:
  - `array == NULL` -> writes `"NULL"`
  - count == 0 -> writes `"{}"`
  - count == 1 -> writes `"{ %p }"` (default printer)
  - count > 1 -> writes `"{\n   <item>,\n   <item>,\n   <item>\n}"` using the
    (default or custom) item printer for each element.
- **`mulle__pointerarray_describe_buffer(...)`** — convenience wrapper calling
  the `_callback` variant with `callback == NULL` and `userinfo == NULL`.
- **`mulle__pointerarray_describe(...)`** — returns a freshly allocated string
  with the same rendering; caller owns and must free it.
- Internally uses `mulle__pointerarray_get_count()`, `mulle__pointerarray_get()`
  and `mulle__pointerarray_for()` from `mulle-container`.

### 3.3. `src/array/pointer/mulle-pointerarray-debug.h` (public facade)

Thin static-inline wrappers. Prefer these over the `mulle__` variants; they take
the public `struct mulle_pointerarray` from `mulle-container` and cast to the
private struct.

```c
static inline void
   mulle_pointerarray_describe_buffer_callback( struct mulle_pointerarray *array,
                                                struct mulle_buffer *buffer,
                                                mulle_container_item_printer_t *callback,
                                                void *userinfo);

// use this only for debugging
static inline void
   mulle_pointerarray_describe_buffer( struct mulle_pointerarray *array,
                                       struct mulle_buffer *buffer);

static inline
char   *mulle_pointerarray_describe( struct mulle_pointerarray *array);
```

- **`mulle_pointerarray_describe(array)`** — primary entry point; returns the
  type-named description string (malloc'd).
- **`mulle_pointerarray_describe_buffer(array, buffer)`** — renders into a
  caller-provided `mulle_buffer` with default item printing.
- **`mulle_pointerarray_describe_buffer_callback(array, buffer, callback, userinfo)`**
  — renders into a `mulle_buffer` using a custom item printer.

### 3.4. Scaffolding headers (not yet implemented)

These public/private header pairs exist and are included via
`_mulle-container-debug-provide.h`, but contain no functions:

- `array/`: `mulle-array-debug`, `mulle-flexarray-debug`, `mulle-structarray-debug`
- `assoc/`: `mulle-assoc-debug`, `pointerpair/`: `mulle-pointerpair-debug`,
  `mulle-pointerpairarray-debug`
- `map/`: `mulle-map-debug`, `pointer/`: `mulle--pointermap-debug` (includes
  `mulle--pointermap-generic-debug` and `mulle--pointermap-struct-debug`)
- `queue/`: `mulle-pointerqueue-debug`, `struct/`: `mulle-structqueue-debug`
- `set/`: `mulle-set-debug`, `pointer/`: `mulle-pointerset-debug` (includes
  `mulle--pointerset-generic-debug`, `mulle--pointerset-struct-debug`)
- `rangeset/`: `mulle--rangeset-debug`

Do not assume describe functions exist for these types yet; only
`mulle-pointerarray` has an implementation.

## 4. Performance Characteristics

- **Describe cost**: O(n) in the number of elements; every element is visited and
  rendered into the buffer. There is no per-element random access overhead beyond
  `mulle__pointerarray_get`.
- **Memory**: `mulle__pointerarray_describe` allocates a result string via
  `mulle_buffer_do_string` (default allocator, i.e. malloc-backed) and the caller
  must free it.
- **Suitability**: Intended for debugging, logging and documentation; not for
  production hot paths. Huge containers produce huge strings.
- **Thread safety**: Not thread-safe. Describe only reads the container, but it
  must not run concurrently with mutation of the same container unless the caller
  provides external locking.

## 5. AI Usage Recommendations & Patterns

### Best Practices

- Use the public `mulle_pointerarray_describe` (single underscore) API, not the
  `mulle__pointerarray_*` implementation functions.
- Free the string returned by `_describe` when done.
- Pass a custom `mulle_container_item_printer_t` callback for user-friendly
  output of non-pointer data, instead of post-processing the raw `%p` output.
- Call describe only in debug paths / diagnostics; gate with build-time debug
  flags when the production build must not pay the cost.
- Call `mulle_pointerarray_describe_buffer` directly when you already have a
  `mulle_buffer` to append into (e.g. a log buffer), avoiding the extra
  allocation.

### Common Pitfalls

- Do not call `mulle_container_debug_get_version_major/minor/patch` — they are
  correct static inline helpers; but there is no `describe_with_callback`
  one-liner returning a string. To get a string with a custom callback, render
  into a buffer (via `mulle_buffer_do_string`) using
  `mulle_pointerarray_describe_buffer_callback` and free the produced string.
- The custom printer (and the default) may be invoked with a NULL `item` when the
  array contains NULL elements; handle that in your callback.
- Do not leak the `char *` returned by `_describe`.
- `mulle_container_item_printer_t` is defined by this library (in
  `mulle--pointerarray-debug.h`), not by `mulle-container`; include
  `mulle-container-debug.h` before referencing it.
- Do not free the `mulle_buffer` you hand to `_describe_buffer` if you created it
  with the `mulle_buffer_do*` macros; those manage cleanup via the block scope.

### Idiomatic Usage

- Include only `mulle-container-debug.h`; it transitively pulls in
  `mulle-container`, `mulle-buffer`, `mulle-sprintf` and `mulle-c11`.
- Use the `mulle_buffer_do_string( buffer, NULL, s) { ... }` macro pattern (as in
  the implementation) to collect a describe rendering into a string.

## 6. Integration Examples

Code style: 3-space indent, Allman braces, C89 variable rules, `return( expr);`.

### Example 1: Describing a Pointerarray

```c
#include <stdio.h>
#include <stdlib.h>

#include <mulle-container-debug/mulle-container-debug.h>
#include <mulle-container/mulle-pointerarray.h>

int  main( void)
{
   struct mulle_pointerarray   array;
   char                        *s;

   mulle_pointerarray_init_default( &array);

   mulle_pointerarray_add( &array, "hamburgesa");
   mulle_pointerarray_add( &array, "mangu");

   s = mulle_pointerarray_describe( &array);
   printf( "%s\n", s);       /* prints something like: { %p, %p } */
   free( s);

   mulle_pointerarray_done( &array);

   return( 0);
}
```

### Example 2: Custom Item Printer via Buffer

```c
#include <stdio.h>
#include <stdlib.h>

#include <mulle-container-debug/mulle-container-debug.h>
#include <mulle-container/mulle-pointerarray.h>
#include <mulle-buffer/mulle-buffer.h>

void   print_string_item( struct mulle_buffer *buffer,
                          void *item,
                          void *userinfo)
{
   MULLE_C_UNUSED( userinfo);

   if( ! item)
      mulle_buffer_add_string( buffer, "(nil)");
   else
      mulle_buffer_add_string( buffer, (char *) item);
}


int  main( void)
{
   struct mulle_pointerarray   array;
   char                        *s;

   mulle_pointerarray_init_default( &array);
   mulle_pointerarray_add( &array, NULL);
   mulle_pointerarray_add( &array, "hamburgesa");

   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle_pointerarray_describe_buffer_callback( &array,
                                                   buffer,
                                                   print_string_item,
                                                   NULL);
   }
   printf( "%s\n", s);       /* prints item text, not raw pointers */
   free( s);

   mulle_pointerarray_done( &array);

   return( 0);
}
```

### Example 3: Version Inspection

```c
#include <stdio.h>

#include <mulle-container-debug/mulle-container-debug.h>

int  main( void)
{
   printf( "version %u.%u.%u\n",
           mulle_container_debug_get_version_major(),
           mulle_container_debug_get_version_minor(),
           mulle_container_debug_get_version_patch());

   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` library dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-c11` — cross-platform C compiler glue and conventions
- `mulle-buffer` — growable C char array / stream used for describe rendering
- `mulle-container` — the container types being inspected (e.g. `mulle-pointerarray`)
- `mulle-sprintf` — extensible sprintf used for formatting (via `mulle-buffer`)