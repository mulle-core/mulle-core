# mulle-core Library Documentation for AI
<!-- Keywords: amalgamation, C, wait-free, allocator, container, buffer, thread -->

## 1. Introduction & Purpose

- mulle-core is an **amalgamated** library that bundles many small C libraries (allocator, buffer, container, rbtree, regex, slug, unicode, url, utf, time, thread, concurrent primitives, printf family, mmap, etc.) into a single library and a single envelope header `<mulle-core/mulle-core.h>`.
- It solves the problem of repeatedly pulling, building, and linking two dozen small libraries: one include, one link target (`-lmulle-core`), faster builds.
- It is *not* a second implementation: each constituent (e.g. `mulle-c/mulle-allocator`, `mulle-concurrent/mulle-concurrent`) is the canonical source; this repository gathers versioned constituent snapshots. Fixes are made upstream and brought in by the next amalgamation.
- Current envelope version: `MULLE__CORE_VERSION` is `0.9.0` (`((0UL << 20) | (9 << 8) | 0)`).
- Notable recent additions (since the previous TOC commit): a resizable **wait-free hashtable** (`mulle_concurrent_hashtable`), a bump **arena allocator** (`mulle_arena`, subclasses `struct mulle_allocator`), UTF-8-preserving **slugify** variants with custom delimiters, **flexible/inflexible** buffer constructors and `mulle_buffer_do_*` builder macros, **`reallocarray`** allocator variants, C11 **rotate** helpers, and an expanded thread portability layer (once-init, recursive mutex, extra atomics).

## 2. Key Concepts & Design Philosophy

- **Envelope/Amalgamation:** `mulle-core.h` re-exports the public headers of all constituents. User code can `#include <mulle-core/mulle-core.h>` and use any constituent API.
- **Subclass-allocation pattern:** `mulle_allocator` is not a C++ base class but struct-with-function-pointer-vtable (`MULLE_ALLOCATOR_BASE`). Constituents like `mulle_arena` and `mulle_aba` *embed* the base as their first member so they can be cast to `struct mulle_allocator *` and passed to any allocator-consuming API.
- **Wait-free / lock-free concurrency:** `mulle_concurrent_hashtable` uses cooperative, wait-free migration where the "frozen" state lives in the **hash word**, never clobbering the value — so payloads are never destroyed during resize. This replaces the older value-sentinel (tombstone) scheme of `mulle_concurrent_hashmap`.
- **NULL-safe dual API convention:** for nearly every constituent, the non-underscore function (e.g. `mulle_pointerarray_init`) is a NULL-safe wrapper around the `_`-prefixed fast variant (e.g. `_mulle_pointerarray_init`) that assumes non-NULL arguments. Use the fast variants only in tight, verified loops.
- **Naming discipline:** `_init`/`_done` for stack or embedded instances, `_create`/`_destroy` for heap instances. Underscore-prefixed *fields* are internal — do not touch directly.

## 3. Core API & Data Structures

This section summarizes each constituent. Signatures are copied verbatim from the public headers under `mulle-core/<constituent>/`.

### 3.1. `mulle-core/mulle-core.h` (envelope header)

- Purpose: single public include; defines the version and includes all constituent public headers.
- Version macros/functions:
  ```c
  #define MULLE__CORE_VERSION  ((0UL << 20) | (9 << 8) | 0)

  static inline unsigned int   mulle_core_get_version_major( void)
  static inline unsigned int   mulle_core_get_version_minor( void)
  static inline unsigned int   mulle_core_get_version_patch( void)
  uint32_t   mulle_core_get_version( void);
  ```
- Also pulls in `_mulle-core-versioncheck.h` if present (via `__has_include`).

### 3.2. `mulle-c11` — cross-platform C compiler glue

- Version `MULLE__C11_VERSION ((4UL << 20) | (9 << 8) | 0)`.
- Provides portability macros only (no runtime code): `MULLE_C_GLOBAL`, `MULLE_C_EXTERN_GLOBAL`, `MULLE_C_NONNULL_FIRST`, `MULLE_C_NO_RETURN`, `MULLE_C_DEPRECATED`, `MULLE_C_UNUSED`, `MULLE_C_LIKELY`/`MULLE_C_UNLIKELY`, `MULLE_C_CONSTRUCTOR`/`MULLE_C_DESTRUCTOR`, `alignas`/`alignof`, plus `mulle_c_popcount*`. Optional headers (`MULLE_C11_INCLUDE_ALL`): `mulle-c11-align.h`, `mulle-c11-eval.h`, `mulle-c11-swap.h`.
- **NEW** `mulle-c11-rotate.h` (added in the latest migration), all `static inline`:
  ```c
  static inline uint32_t   mulle_rotate_right_uint32( uint32_t value, unsigned shift)
  static inline uint32_t   mulle_rotate_left_uint32( uint32_t value, unsigned shift)
  static inline uint64_t   mulle_rotate_right_uint64( uint64_t value, unsigned shift)
  static inline uint64_t   mulle_rotate_left_uint64( uint64_t value, unsigned shift)
  ```
  Shift is masked to the width; shift `0`/`width` is identity (no UB). Compiles to single `ror`/`rol`.

### 3.3. `mulle-allocator` — flexible memory allocation scheme

- Version `MULLE__ALLOCATOR_VERSION ((8UL << 20) | (1 << 8) | 0)`.
- Core type (from `mulle-allocator-struct.h`):
  ```c
  #define MULLE_ALLOCATOR_BASE                                                                         \
     void   *(*calloc)( size_t n, size_t size, struct mulle_allocator *allocator);                     \
     void   *(*realloc)( void *block, size_t size, struct mulle_allocator *allocator);                 \
     void   (*free)( void *block, struct mulle_allocator *allocator);                                  \
     void   (*fail)( struct mulle_allocator *allocator, void *block, size_t size) _MULLE_C_NO_RETURN;  \
     int    (*abafree)( void *aba, void (*free)( void *, void *), void *block, void *owner);           \
     void   *aba

  struct mulle_allocator
  {
     MULLE_ALLOCATOR_BASE;
  };
  ```
- Global instances: `mulle_allocator_default`, `mulle_allocator_stdlib`, `mulle_allocator_stdlib_nofree`; legacy spellings `mulle_default_allocator` etc.
- Inline allocator API (`NULL` p == default allocator):
  ```c
  static inline void   *mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
  static inline void   *mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
  static inline void   *mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
  static inline void   *mulle_allocator_realloc_strict( struct mulle_allocator *p, void *block, size_t size)
  static inline void   *mulle_allocator_reallocarray( struct mulle_allocator *p, void *block, size_t n, size_t size)
  static inline void   *mulle_allocator_reallocarray_strict( struct mulle_allocator *p, void *block, size_t n, size_t size)
  static inline void   mulle_allocator_free( struct mulle_allocator *p, void *block)
  static inline char   *mulle_allocator_strdup( struct mulle_allocator *p, const char *s)
  ```
- Convenience API against the default allocator:
  ```c
  static inline void   *mulle_malloc( size_t size)
  static inline void   *mulle_calloc( size_t n, size_t size)
  static inline void   *mulle_realloc( void *block, size_t size)
  static inline void   *mulle_reallocarray( void *block, size_t n, size_t size)
  static inline void   *mulle_realloc_strict( void *block, size_t size)
  static inline void   *mulle_reallocarray_strict( void *block, size_t n, size_t size)
  static inline void   mulle_free( void *block)
  static inline char   *mulle_strdup( const char *s)
  ```
  The `reallocarray*` variants are **NEW**: they multiply `n * size` with overflow checking (`mulle_allocator_size_multiply`), call the vtable `realloc`, and call `fail` on failure. `_strict` frees the block when `n || size` is 0 instead of failing.
- Failure hook: `void   mulle_allocation_fail( struct mulle_allocator *allocator, void *block, size_t size)` (no-return); configurable via `mulle_allocator_set_fail`.
- `mulle-alloca.h` stack/heap hybrid macros: `mulle_alloca_do( name, type, count)`, `mulle_calloca_do_*`, `mulle_malloc_do`, `mulle_calloc_do`, `mulle_calloc_do_flexible`, ... (alloca when it fits the fixed `MULLE_ALLOCA_STACKSIZE` (128 bytes), else malloc; automatic free at scope end).

### 3.4. `mulle-data` — hash functions, ranges, qsort

- Version `MULLE__DATA_VERSION` (see `mulle-data.h`).
- Byte blob type:
  ```c
  struct mulle_data
  {
     void     *bytes;
     size_t   length;
  };
  ```
  with `mulle_data_make/make_empty/make_invalid`, `mulle_data_hash`, `mulle_data_hash_chained`, `mulle_data_subdata`, `mulle_data_search_data`, `mulle_data_equals/compare`.
- Hashing (`mulle-hash.h`, `mulle-fnv1a.h`, `xxhash.h` bundled):
  ```c
  static inline uint32_t   mulle_hash_32( const void *bytes, size_t length)
  static inline uint64_t   mulle_hash_64( const void *bytes, size_t length)
  static inline uintptr_t  mulle_hash( const void *bytes, size_t length)
  static inline uintptr_t  mulle_integer_hash( uintptr_t p)
  static inline uintptr_t  mulle_pointer_hash( const void *p)
  static inline uintptr_t  mulle_double_hash( double f)
  ```
  plus chained variants (`mulle_hash_chained_32/64/add/final`), FNV-1a (`mulle_fnv1a_32/64`, `_string_hash_32/64`), `mulle_prime_for_depth`, and the full `XXH32/64/XXH3` xxHash API.
- Ranges (`mulle-range.h`):
  ```c
  struct mulle_range
  {
     uintptr_t   location;
     uintptr_t   length;
  };

  #define mulle_not_found_e     ((uintptr_t) INTPTR_MAX)
  ```
  Functions: `mulle_range_make/make_locations/make_all/make_invalid`, `mulle_range_get_min/max`, `mulle_range_contains`, `mulle_range_intersects/_intersection/_union/_subtract/_insert`, binary search on sorted range arrays, and the `mulle_range_for( range, name)` iteration macro. Note: `mulle_not_found_e` (== `INTPTR_MAX`) is the shared "not found" sentinel.
- `mulle-qsort.h`: `void   mulle_qsort_r( void *a, size_t n, size_t es, mulle_qsort_r_cmp_t *cmp, void *thunk)` and a `mulle_qsort` wrapper.

### 3.5. `mulle-buffer` — growable char buffer and stream

- Version `MULLE__BUFFER_VERSION ((5UL << 20) | (2 << 8) | 0)`. **NOT thread-safe**; one thread per instance.
- Structure:
  ```c
  struct mulle_buffer
  {
     MULLE_BUFFER_BASE;
  };
  ```
  `MULLE_BUFFER_BASE` = `MULLE__BUFFER_BASE` (`_storage`, `_curr`, `_sentinel`, `_initial_storage`, `_size`, `_type`) + a public `struct mulle_allocator *_allocator`.
- Mode flags (`_type`): `MULLE_BUFFER_IS_FLEXIBLE` (0, grows by malloc), `MULLE_BUFFER_IS_INFLEXIBLE` (1, sets `MULLE_BUFFER_IS_OVERFLOWN` on excess), `MULLE_BUFFER_IS_FLUSHABLE` (3), `MULLE_BUFFER_IS_SPRINTF_INFLEXIBLE` (5), `MULLE_BUFFER_IS_OVERFLOWN` (8, sticky) plus `MULLE_BUFFER_IS_READONLY/WRITEONLY/TEXT/BINARY`.
- Lifecycle:
  ```c
  static inline struct mulle_buffer   *mulle_buffer_alloc( struct mulle_allocator *allocator)
  struct mulle_buffer   *mulle_buffer_create( struct mulle_allocator *allocator);
  static inline void   mulle_buffer_init( struct mulle_buffer *buffer, size_t capacity, struct mulle_allocator *allocator)
  static inline void   mulle_buffer_destroy( struct mulle_buffer *buffer)
  static inline void   mulle_buffer_done( struct mulle_buffer *buffer)
  static inline void   mulle_buffer_reset( struct mulle_buffer *buffer)
  ```
- `MULLE_BUFFER_DEFAULT_CAPACITY` (`128`) feeds `MULLE_BUFFER_DATA`, `mulle_buffer_init_default`, `mulle_buffer_create` and the `mulle_buffer_do_*` stack backing.
- **Backing constructors** (flexible vs. inflexible; caller-owned storage when noted):
  ```c
  static inline void   mulle_buffer_init_with_allocated_bytes( struct mulle_buffer *buffer,
                                                               void *storage, size_t length,
                                                               struct mulle_allocator *allocator)
  static inline void   mulle_buffer_init_with_static_bytes( struct mulle_buffer *buffer,
                                                            void *storage, size_t length,
                                                            struct mulle_allocator *allocator)
  static inline void   mulle_buffer_init_inflexible_with_static_bytes( struct mulle_buffer *buffer,
                                                                       void *storage, size_t length)
  void   mulle_buffer_init_with_const_bytes( struct mulle_buffer *buffer,
                                             const void *storage, size_t length);
  static inline void   mulle_buffer_make_inflexible( struct mulle_buffer *buffer,
                                                     void *storage, size_t length)
  static inline void   mulle_buffer_set_allocator( struct mulle_buffer *buffer,
                                                   struct mulle_allocator *allocator)
  ```
  Compound-literal initializers for static/global use: `MULLE_BUFFER_DATA( allocator)`, `MULLE_BUFFER_FLEXIBLE_DATA( data, len, allocator)`, `MULLE_BUFFER_FLEXIBLE_FILLED_DATA`, `MULLE_BUFFER_INFLEXIBLE_DATA(...)`, `MULLE_BUFFER_INFLEXIBLE_FILLED_DATA(...)` (with single-evaluation inline wrappers `_mulle_buffer_*_data`).
- **`mulle_buffer_do_*` builder macros** (declare a scoped buffer, auto-`done` at scope exit, break-safe):
  - `mulle_buffer_do( name)` — heap-backed, default capacity, default allocator
  - `mulle_buffer_do_allocator( name, allocator)`
  - `mulle_buffer_do_flexible( name, data, len)`, `mulle_buffer_do_flexible_filled( name, data, len)`
  - `mulle_buffer_do_inflexible( name, data, len)`, `mulle_buffer_do_inflexible_filled( name, data, len)`
  - `mulle_buffer_do_string( name, allocator, s)` — builds a string, assigns result to `s`
  - `mulle_buffer_return( name, value)` — `typeof`-based done+return helper
- Core writes (all `static inline` unless noted): `mulle_buffer_add_byte/char/bytes/chars/string/c_string`, `mulle_buffer_append_string`, `mulle_buffer_add_string_with_maxlength`, `mulle_buffer_strcat/strcpy`, `mulle_buffer_add_buffer/_range`, `mulle_buffer_memset`, `mulle_buffer_guarantee`, `mulle_buffer_advance`, `mulle_buffer_pop_byte`, `mulle_buffer_remove_last_byte`, `void   mulle_buffer_add_bytes_callback( void *buffer, void *bytes, size_t length)`.
- Reads/seeks: `mulle_buffer_get_bytes/get_string/get_length/get_capacity`, `mulle_buffer_extract_data/string/bytes` (transfer ownership), `mulle_buffer_get_byte/get_last_byte/next_byte/peek_byte`, `mulle_buffer_seek_byte`, `mulle_buffer_set_seek/get_seek/get_lseek/lseek`, `mulle_buffer_memcmp`, `mulle_buffer_copy_range`, `mulle_buffer_has_overflown`.
- Sizing: `mulle_buffer_grow`, `mulle_buffer_size_to_fit`, `mulle_buffer_set_length( buffer, length, options)` with `MULLE_BUFFER_NO_SHRINK/NO_ZEROFILL` flags.

### 3.6. `mulle-container` — arrays, hashtables, queues

- Version `MULLE__CONTAINER_VERSION ((10UL << 20) | (1 << 8) | 3)`.
- Engineered types (names to use): `struct mulle_pointerarray` (ordered `void *` array), `struct mulle_map` (key/value hashtable), `struct mulle_set` (membership), `struct mulle_pointerqueue`/`struct mulle_structqueue` (FIFO), plus generic `mulle__pointerarray`, `mulle__pointermap`, `mulle__pointerset`, `mulle_structarray` etc.
- **Callbacks** (`mulle-container-callback.h`) drive hashing/equality/retain/release. Key ones:
  ```c
  typedef uintptr_t
     mulle_container_keycallback_hash_t( const struct mulle_container_keycallback *callback,
                                         const void *p);
  typedef int
     mulle_container_keycallback_is_equal_t( const struct mulle_container_keycallback *callback,
                                             const void *p, const void *q);
  typedef void *
     mulle_container_keycallback_retain_t( const struct mulle_container_keycallback *callback,
                                           void *p, struct mulle_allocator *allocator);
  typedef void
     mulle_container_keycallback_release_t( const struct mulle_container_keycallback *callback,
                                            void *p, struct mulle_allocator *allocator);
  ```
  `struct mulle_container_keycallback` has fields `retain, release, describe, userinfo, notakey, hash, is_equal`; `struct mulle_container_valuecallback` has `retain, release, describe, userinfo`; `struct mulle_container_keyvaluecallback` embeds both.
- `mulle_pointerarray` lifecycle/ops:
  ```c
  static inline void   mulle_pointerarray_init( struct mulle_pointerarray *array,
                                                size_t capacity,
                                                struct mulle_allocator *allocator)
  static inline struct mulle_pointerarray *   mulle_pointerarray_create( struct mulle_allocator *allocator)
  static inline void   mulle_pointerarray_done( struct mulle_pointerarray *array)
  static inline void   mulle_pointerarray_destroy( struct mulle_pointerarray *array)
  static inline void   mulle_pointerarray_add( struct mulle_pointerarray *array, void *p)
  static inline void   *mulle_pointerarray_get( struct mulle_pointerarray *array, size_t i)
  static inline void   *mulle_pointerarray_pop( struct mulle_pointerarray *array)
  static inline void   mulle_pointerarray_remove( struct mulle_pointerarray *array, void *p)
  static inline void   mulle_pointerarray_insert( struct mulle_pointerarray *array,
                                                  uintptr_t location, void *p)
  ```
- `mulle_map` lifecycle/ops:
  ```c
  void   mulle_map_init( struct mulle_map *map,
                         size_t capacity,
                         struct mulle_container_keyvaluecallback *callback,
                         struct mulle_allocator *allocator);
  struct mulle_map   *mulle_map_create( size_t capacity,
                                        struct mulle_container_keyvaluecallback *callback,
                                        struct mulle_allocator *allocator);
  static inline int   mulle_map_insert( struct mulle_map *map, void *key, void *value)
  static inline void   *mulle_map_register( struct mulle_map *map, void *key, void *value)
  static inline void   *mulle_map_update( struct mulle_map *map, void *key, void *value)
  static inline void   *mulle_map_get( struct mulle_map *map, const void *key)
  static inline int   mulle_map_remove( struct mulle_map *map, const void *key)
  static inline size_t   mulle_map_get_count( struct mulle_map *map)
  void   mulle_map_add_map( struct mulle_map *map, struct mulle_map *other);
  ```
- `mulle_pointerqueue` (bucket-linked FIFO):
  ```c
  static inline void   mulle_pointerqueue_init( struct mulle_pointerqueue *queue,
                                                unsigned short bucket_size,
                                                unsigned short spare_allowance,
                                                struct mulle_allocator *allocator)
  void   mulle_pointerqueue_push( struct mulle_pointerqueue *queue, void *p)
  void   *mulle_pointerqueue_pop( struct mulle_pointerqueue *queue)
  static inline size_t   mulle_pointerqueue_get_count( struct mulle_pointerqueue *queue)
  ```

### 3.7. `mulle-container-debug` — container describe helpers

- Version `MULLE__CONTAINER__DEBUG_VERSION ((0UL << 20) | (1 << 8) | 4)`.
- Provides `mulle_*_describe_buffer`, `mulle_*_describe`, `mulle_*_describe_buffer_callback` for each container type, e.g.:
  ```c
  typedef void   mulle_container_item_printer_t( struct mulle_buffer *buffer,
                                                 void *item, void *userinfo);
  char   *mulle_pointerarray_describe( struct mulle_pointerarray *array);
  void   mulle_pointerarray_describe_buffer( struct mulle_pointerarray *array,
                                             struct mulle_buffer *buffer);
  ```

### 3.8. `mulle-rbtree` and `mulle-rbtree-debug`

- Red/black tree storing `void *` values with an ordering comparison function.
- Types:
  ```c
  struct mulle_rbnode
  {
     struct mulle_rbnode    *_parent;
     struct mulle_rbnode    *_left;
     struct mulle_rbnode    *_right;
     int                    _color;
     void                   *payload;
  };

  struct mulle_rbtree
  {
     MULLE__RBTREE_BASE;
     int    (*comparison)( void *a, void *b);
     void   (*dirty)( void *a, void *b, void *c);
     struct mulle_container_valuecallback    callback;
  };
  ```
- Ops:
  ```c
  void   mulle_rbtree_init( struct mulle_rbtree *a_tree,
                            int (*a_comp)( void *, void *),
                            struct mulle_container_valuecallback *callback,
                            struct mulle_allocator *allocator);
  int   _mulle_rbtree_add( struct mulle_rbtree *a_tree, void *value);
  int   _mulle_rbtree_remove( struct mulle_rbtree *a_tree, void *value);
  void  *mulle_rbtree_find( struct mulle_rbtree *a_tree, void *a_key);
  void  *mulle_rbtree_find_equal_or_greater( struct mulle_rbtree *a_tree, void *a_key);
  void   mulle_rbtree_walk( struct mulle_rbtree *a_tree,
                            int (*callback)( void *value, void *userinfo), void *userinfo);
  ```
  Iteration macros `mulle_rbtree_for( a_tree, item)` and `mulle_rbtree_reversefor( a_tree, item)`.
- `mulle-rbtree-debug` adds only:
  ```c
  char  *mulle__rbtree_validate( struct mulle__rbtree *a_tree);   // NULL == valid
  void   mulle__rbtree_node_dot_fprintf( FILE *fp, struct mulle__rbtree *tree, ...);
  void   mulle__rbtree_node_ascii_fprintf( FILE *fp, struct mulle__rbtree *tree, ...);
  ```

### 3.9. `mulle-storage` — tree-node memory management (+ arena)

- Purpose: fast fixed-size object reuse for tree/container nodes.
- Types and ops:
  ```c
  struct mulle_storage   { struct mulle_structqueue _structs; struct mulle__pointerarray _freed; };
  void   _mulle_storage_init( struct mulle_storage *alloc, size_t sizeof_struct,
                              unsigned int alignof_struct, unsigned int capacity,
                              struct mulle_allocator *allocator);
  void  *_mulle_storage_malloc( struct mulle_storage *alloc);
  void  *_mulle_storage_calloc( struct mulle_storage *alloc);
  void   _mulle_storage_free( struct mulle_storage *alloc, void *p);

  struct mulle_indexedstorage { struct mulle_structarray _structs; struct mulle__pointerarray _freed; };
  unsigned int   _mulle_indexedstorage_alloc( struct mulle_indexedstorage *alloc);
  void          *_mulle_indexedstorage_get( struct mulle_indexedstorage *alloc, unsigned int index);
  void           _mulle_indexedstorage_free( struct mulle_indexedstorage *alloc, unsigned int index);
  ```
- **NEW `mulle_arena`** (`mulle-storage/mulle-arena.h`, header-inline, no `.c`): a bump allocator that **subclasses `struct mulle_allocator`** by embedding `MULLE_ALLOCATOR_BASE` first, so `mulle_arena_get_allocator( &arena)` yields a `struct mulle_allocator *` that can be handed to any allocator-consuming API. Variable-size blocks; the freed memory is reclaimed only on reset/done (allocation-free is a no-op except rewinding the last allocation in debug).
  ```c
  struct mulle_arena
  {
     MULLE_ALLOCATOR_BASE;
     struct mulle__pointerarray   _pages;
     char                         *_current;
     char                         *_sentinel;
     char                         *_last;       // start of last allocation (for realloc)
     size_t                       _page_size;
     size_t                       _page_index;  // index of current page in _pages
  };

  #define MULLE_ARENA_DEFAULT_ALIGNMENT   alignof( max_align_t)

  static inline void   mulle_arena_init( struct mulle_arena *arena,
                                         size_t page_size,
                                         struct mulle_allocator *allocator)
  static inline void   mulle_arena_done( struct mulle_arena *arena)
  static inline void   mulle_arena_reset( struct mulle_arena *arena)
  static inline void   mulle_arena_reset_keep_pages( struct mulle_arena *arena, size_t keep_count)
  static inline void   mulle_arena_reset_keep_percentage( struct mulle_arena *arena, unsigned int percent)
  static inline void  *mulle_arena_alloc( struct mulle_arena *arena, size_t size, unsigned int alignment)
  static inline void  *mulle_arena_calloc( struct mulle_arena *arena, size_t size, unsigned int alignment)
  static inline void  *mulle_arena_memdup( struct mulle_arena *arena, void *src, size_t size)
  static inline char  *mulle_arena_strdup( struct mulle_arena *arena, char *s)
  static inline struct mulle_allocator  *mulle_arena_get_allocator( struct mulle_arena *arena)
  static inline size_t   mulle_arena_get_page_count( struct mulle_arena *arena)
  ```
  `page_size == 0` selects a 4096-byte default. The allocator argument is unused internally (pages come from `mulle_allocator_default`). `_mulle_arena_realloc` extends in place or copies. Oversized requests get a dedicated page.

### 3.10. `mulle-http` and `mulle-url`

- `mulle-http` (version `MULLE__HTTP_VERSION ((0UL << 20) | (1 << 8) | 14)`) bundles the **node.js http-parser** (v2.7.0). The main struct is `struct http_parser` (opaque-ish, has public `data` field). Callbacks are set in `struct http_parser_settings` (`on_message_begin`, `on_url`, `on_header_field`, `on_header_value`, `on_headers_complete`, `on_body`, `on_message_complete`, `on_chunk_header`, `on_chunk_complete`).
  ```c
  void   http_parser_init( http_parser *parser, enum http_parser_type type);
  size_t http_parser_execute( http_parser *parser,
                              const http_parser_settings *settings,
                              const char *data, size_t len);
  int    http_should_keep_alive( const http_parser *parser);
  int    http_parser_parse_url( const char *buf, size_t buflen,
                                int is_connect, struct http_parser_url *u);
  ```
  `enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH }`; `struct http_parser_url` holds `field_set`, `port`, and `field_data[7]` for the `UF_*` fields (schema, host, port, path, query, fragment, userinfo).
- `mulle-url` is *not* a parser in this amalgamation — it provides **URL character validation** predicates (valid scheme/host/user/password/path/query/fragment, non-percent-escape), each as the standard unicode triple pattern:
  ```c
  int   mulle_unicode16_is_validurlscheme( uint16_t c);
  int   mulle_unicode_is_validurlscheme( int32_t c);
  int   mulle_unicode_is_validurlschemeplane( unsigned int plane);
  ```

### 3.11. `mulle-regex` — Unicode regex

- Opaque `struct mulle_utf32regex *`, compiled from UTF-32 patterns.
  ```c
  struct mulle_utf32regex   *mulle_utf32regex_compile( const mulle_utf32_t *pattern);
  static inline void   mulle_utf32regex_free( struct mulle_utf32regex *regex);
  int   mulle_utf32regex_execute( struct mulle_utf32regex *regex, const mulle_utf32_t *src);
  int   mulle_utf32regex_substitute( struct mulle_utf32regex *regex,
                                     const mulle_utf32_t *replacement,
                                     mulle_utf32_t *dst, size_t dst_len, int zero);
  mulle_utf32_t   *mulle_utf32_match( const mulle_utf32_t *pattern, const mulle_utf32_t *src);
  struct mulle_range   mulle_utf32regex_range_for_index( struct mulle_utf32regex *regex, unsigned int i);
  ```
  `execute` returns `<0` error / `1` match / `0` no match. `range_for_index` index `0` = whole match, `1-9` = capture groups.

### 3.12. `mulle-slug` — URL slugs

- Version `MULLE__SLUG_VERSION ((0UL << 20) | (2 << 8) | 0)`. Converts text into URL- or filename-safe slugs. Transliterates where possible; non-transliterable letters are either dropped (ASCII-only) or **kept as UTF-8** (new `_utf8` variants).
  ```c
  // allocated string, mulle_free it; non-transliterable chars dropped
  char   *mulle_utf8_slugify( const char *s);

  // NEW: non-transliterable letters passed through as UTF-8
  char   *mulle_utf8_slugify_utf8( const char *s);

  struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                  struct mulle_allocator *allocator);

  // direct-to-buffer variants (NEW): empty buffer, no extra allocation for ASCII
  void   mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer, struct mulle_utf8data data);
  void   mulle_buffer_add_utf8_slugified_utf8data( struct mulle_buffer *buffer, struct mulle_utf8data data);
  void   mulle_buffer_add_slugified_utf8data_with_delimiter( struct mulle_buffer *buffer,
                                                             struct mulle_utf8data data, char delimiter);
  void   mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer, struct mulle_utf8data data);

  // NEW convenience: write into caller-provided buffer, returns dst (NULL if dst_len == 0)
  char  *mulle_slugify( char *dst, size_t dst_len, const char *src, size_t src_len);
  char  *mulle_slugify_with_delimiter( char *dst, size_t dst_len,
                                       const char *src, size_t src_len, char delimiter);
  ```
  Default delimiter is `-`. `src_len` may be `(size_t) -1` for NUL-terminated input.

### 3.13. `mulle-unicode` and `mulle-utf`

- `mulle-unicode`: Unicode "ctype"-like classification/case predicates. Each predicate comes as the standard triple:
  ```c
  int   mulle_unicode16_is_letter( uint16_t c);
  int   mulle_unicode_is_letter( int32_t c);
  int   mulle_unicode_is_letterplane( unsigned int plane);
  ```
  Predicates: `is_alphanumeric`, `is_capitalized`, `is_control`, `is_decimaldigit`, `is_decomposable`, `is_identifierstart`, `is_identifiercontinuation`, `is_legalcharacter`, `is_lowercase`, `is_newline`, `is_nonbase`, `is_noncharacter`, `is_punctuation`, `is_symbol`, `is_uppercase`, `is_whitespace`, `is_whitespaceornewline`, `is_zerodigit`. Case conversion: `mulle_unicode_tolower/16_tolower`, `mulle_unicode_toupper/16_toupper`, `mulle_unicode_totitlecase/16_totitlecase`. Also supports the `mulle_unicode_is_*` variants taking `mulle_unicode_planeloader` for BMP/supplementary.
- `mulle-utf`: UTF-8/16/32 analysis and conversion.
  ```c
  typedef uint16_t   mulle_utf16_t;
  typedef int32_t    mulle_utf32_t;   // 0 - 0x10FFFF

  struct mulle_utf8data  { char    *characters; size_t length; };
  struct mulle_utf16data { mulle_utf16_t *characters; size_t length; };
  struct mulle_utf32data { mulle_utf32_t *characters; size_t length; };
  ```
  Length prediction: `mulle_utf8_utf16length`, `mulle_utf8_utf32length`, `mulle_utf16_utf8length`, `mulle_utf16_utf32length`, `mulle_utf32_utf8length`, `mulle_utf32_utf16length`. Validation: `mulle_utf8_validate`, `mulle_utf16_validate`, `mulle_utf32_validate`. Low-level converters `_mulle_utf8_convert_to_utf16/32`, `_mulle_utf16_convert_to_utf8/32`, `_mulle_utf32_convert_to_utf16/8` (into caller buffer). High-level "string" converters `mulle_utf8_convert_to_utf16_string(src, len, allocator)` etc. always NUL-terminate and allocate. There is also the small-string compression scheme `mulle-char5.h`/`mulle-char7.h` (`mulle_char5_encode32/64`, `mulle_char7_*`) and substring helpers `mulle_utf8_strnlen/strnstr/strnchr/strspn/strcspn`.

### 3.14. `mulle-vararg` — structured vararg access

- Lets you read `va_list` arguments in struct layout fashion (register preservation), avoiding the platform `va_arg` limitations.
  ```c
  typedef struct { void *p; }   mulle_vararg_list;

  #define mulle_vararg_start( args, ap)
  #define mulle_vararg_next_integer( args, type)
  #define mulle_vararg_next_pointer( args, type)
  #define mulle_vararg_next_fp( args, type)
  #define mulle_vararg_next_struct( args, type)
  #define mulle_vararg_copy( dst, src)
  ```
  Convenience: `mulle_vararg_next_int/char/short/long/longlong/int32/int64/uint*/float/double/longdouble`, `mulle_vararg_count_pointers( args, first)`. Builder (`mulle-vararg-builder.h`): `mulle_vararg_list_make( buf)`, `MULLE_VARARG_BUILDERBUFFER_N( n)`, `mulle_vararg_push_integer/fp/pointool/struct`, and the `mulle_vararg_builder_do( name, size)` scoped macro. Used by `mulle_sprintf`'s `mulle_buffer_mvsprintf`.

### 3.15. `mulle-thread` (+ `mintomic`) — threads, synch, atomics

- Version `MULLE__THREAD_VERSION ((4UL << 20) | (10 << 8) | 0)`. Backend auto-selected: C11 threads (`HAVE_C11_THREADS`), Windows, or pthreads. API identical across backends.
- Thread types (pthreads backend): `typedef pthread_t mulle_thread_t;`, `typedef pthread_mutex_t mulle_thread_mutex_t;`, `typedef pthread_cond_t mulle_thread_cond_t;`, `typedef pthread_key_t mulle_thread_tss_t;`, `typedef void *mulle_thread_rval_t;`, with `typedef mulle_thread_rval_t (MULLE_THREAD_CALL mulle_thread_function_t)( void *);`. Functions return `0` on success.
  ```c
  static inline int   mulle_thread_create( mulle_thread_function_t *f, void *arg, mulle_thread_t *p_thread)
  static inline mulle_thread_rval_t   mulle_thread_join( mulle_thread_t thread)
  static inline int   mulle_thread_detach( mulle_thread_t thread)
  static inline int   mulle_thread_mutex_init( mulle_thread_mutex_t *lock)
  static inline int   mulle_thread_mutex_lock( mulle_thread_mutex_t *lock)
  static inline int   mulle_thread_mutex_unlock( mulle_thread_mutex_t *lock)
  static inline int   mulle_thread_cond_wait( mulle_thread_cond_t *cond, mulle_thread_mutex_t *mutex)
  static inline int   mulle_thread_cond_signal( mulle_thread_cond_t *cond)
  int   mulle_thread_tss_create( mulle_thread_callback_t *f, mulle_thread_tss_t *key)
  void  *mulle_thread_tss_get( mulle_thread_tss_t key)
  ```
- **NEW once-init** (`mulle_thread_once_t`): values `MULLE_THREAD_ONCE_DATA 0`, `MULLE_THREAD_ONCE_BUSY 1848`, `MULLE_THREAD_ONCE_DONE 1`.
  ```c
  void   mulle_thread_once( mulle_thread_once_t *once, void (*init)( void));
  void   mulle_thread_once_call( mulle_thread_once_t *once, void (*init)( void *), void *userinfo);
  void   mulle_thread_once_call_recursive( mulle_thread_once_recursive_t *once, void (*init)( void *), void *userinfo);
  ```
  plus inline `mulle_thread_once_recursive`, `mulle_thread_once_noblock`, `mulle_thread_once_call_noblock` and the `mulle_thread_once_do( name)` macros.
- **NEW recursive mutex**:
  ```c
  typedef struct
  {
     mulle_thread_mutex_t    _mutex;
     mulle_atomic_pointer_t  _thread_id;  // owning thread id, or NULL
     mulle_atomic_pointer_t  _depth;      // recursion depth (1-based when locked)
  } mulle_thread_recursive_mutex_t;

  int   mulle_thread_recursive_mutex_init( mulle_thread_recursive_mutex_t *p);
  int   mulle_thread_recursive_mutex_done( mulle_thread_recursive_mutex_t *p);
  void  mulle_thread_recursive_mutex_lock( mulle_thread_recursive_mutex_t *p);
  void  mulle_thread_recursive_mutex_unlock( mulle_thread_recursive_mutex_t *p);
  int   mulle_thread_recursive_mutex_trylock( mulle_thread_recursive_mutex_t *p); // 0=acquired
  ```
  Scoped locks: `mulle_thread_mutex_do( mutex)`, `mulle_thread_recursive_mutex_do( mutex)`.
- **Atomics** (`mulle-atomic.h` over C11 atomics or the bundled **mintomic** backend — mintomic also ships standalone under `mintomic/`):
  ```c
  typedef _Atomic( void *)               mulle_atomic_pointer_t;  // C11 backend
  void  *_mulle_atomic_pointer_read( mulle_atomic_pointer_t *address);
  void   _mulle_atomic_pointer_write( mulle_atomic_pointer_t *address, void *value);
  int    _mulle_atomic_pointer_cas( mulle_atomic_pointer_t *address, void *value, void *expect);
  void  *__mulle_atomic_pointer_cas( mulle_atomic_pointer_t *address, void *value, void *expect);
  void  *_mulle_atomic_pointer_set( mulle_atomic_pointer_t *address, void *value);
  void  *_mulle_atomic_pointer_increment( mulle_atomic_pointer_t *address);
  void  *_mulle_atomic_pointer_decrement( mulle_atomic_pointer_t *address);
  void  *_mulle_atomic_pointer_add( mulle_atomic_pointer_t *address, intptr_t diff);
  ```
  Library also provides acquire/relaxed load/store variants (`_read_acquire`, `_read_relaxed`, `_write_relaxed`) and a `_functionpointer_*` family. mintomic itself exposes `mint_atomic32_t/64_t/Ptr_t` and `mint_*_{32,64,ptr}_relaxed` load/store/CAS/exchange/fetch-and/or/add operations plus fences. `mint_atomicPtr_t` is `mulle_atomic_pointer_t`.

### 3.16. `mulle-concurrent` — lock-/wait-free hashtables, sets, arrays

- Version `MULLE__CONCURRENT_VERSION ((4UL << 20) | (0 << 8) | 0)`. Coordination relies on `mulle-aba` for safe reclamation (see 3.17).
- **NEW resizable wait-free `mulle_concurrent_hashtable`** (`mulle-concurrent-hashtable.h`). The *hash word* carries the migration "FROZEN" state, so values are never overwritten during resize (no tombstones; a removed slot keeps its hash claim and is immediately reusable). `NULL` is the only reserved payload. Process must call `mulle_aba_init(NULL)` once; each worker thread must `mulle_aba_register()`/`mulle_aba_unregister()`.
  ```c
  struct _mulle_concurrent_hashtablepair
  {
     mulle_atomic_pointer_t   hash;    // intptr_t, 0 == unclaimed, FROZEN bit
     mulle_atomic_pointer_t   value;   // payload or NULL
  };

  struct mulle_concurrent_hashtable
  {
     union mulle_concurrent_atomichashtablestorage_t   storage;
     union mulle_concurrent_atomichashtablestorage_t   next_storage;
     mulle_atomic_pointer_t   allocator;
     uintptr_t                frozen_bit;
     uintptr_t                hash_mask;
  };

  void   mulle_concurrent_hashtable_init( struct mulle_concurrent_hashtable *map,
                                          size_t size, struct mulle_allocator *allocator);
  void   mulle_concurrent_hashtable_init_positive( struct mulle_concurrent_hashtable *map,
                                                   size_t size, struct mulle_allocator *allocator);
  void   mulle_concurrent_hashtable_init_even( struct mulle_concurrent_hashtable *map,
                                               size_t size, struct mulle_allocator *allocator);
  void  mulle_concurrent_hashtable_done( struct mulle_concurrent_hashtable *map);
  int   mulle_concurrent_hashtable_insert( struct mulle_concurrent_hashtable *map, intptr_t hash, void *value);
  int   mulle_concurrent_hashtable_register( struct mulle_concurrent_hashtable *map,
                                             intptr_t hash, void *value, void **p_old);
  int   mulle_concurrent_hashtable_remove( struct mulle_concurrent_hashtable *map, intptr_t hash, void *value);
  void  *mulle_concurrent_hashtable_lookup( struct mulle_concurrent_hashtable *map, intptr_t hash);
  size_t   mulle_concurrent_hashtable_get_size( struct mulle_concurrent_hashtable *map);
  size_t   mulle_concurrent_hashtable_count( struct mulle_concurrent_hashtable *map);
  ```
  Hash keys must be `intptr_t` in `[1, INTPTR_MAX]` (positive mode, FROZEN is the top bit; `0 == MULLE_CONCURRENT_NO_HASH`). Sentinel macros: `MULLE_CONCURRENT_NO_HASH 0`, `MULLE_CONCURRENT_NO_POINTER ((void *) 0)`, `MULLE_CONCURRENT_INVALID_POINTER`, `MULLE_CONCURRENT_TOMBSTONE_POINTER`. Return codes: `insert` → `0`/`EEXIST`/`EINVAL`; `remove` → `0`/`ENOENT`/`EINVAL`; enumerators → `1`=next, `0`=end, `ECANCELED`=storage migrated (restart enumeration).
  Enumeration with the `mulle_concurrent_hashtable_for( name, hash, value)` / `..._for_rval` macros (plus explicit `mulle_concurrent_hashtable_enumerate`/`enumerator_next`/`enumerator_done`).
- `mulle_concurrent_hashmap` (legacy, when you must store `NULL` payloads or want a backing value-sentinel scheme):
  ```c
  void   mulle_concurrent_hashmap_init( struct mulle_concurrent_hashmap *map,
                                        unsigned int size, struct mulle_allocator *allocator);
  void   mulle_concurrent_hashmap_done( struct mulle_concurrent_hashmap *map);
  int   mulle_concurrent_hashmap_insert( struct mulle_concurrent_hashmap *map, intptr_t hash, void *value);
  void  *mulle_concurrent_hashmap_register( struct mulle_concurrent_hashmap *map, intptr_t hash, void *value);
  void  *mulle_concurrent_hashmap_lookup( struct mulle_concurrent_hashmap *map, intptr_t hash);
  int   mulle_concurrent_hashmap_count( struct mulle_concurrent_hashmap *map);
  ```
  Removal is single-threaded only (`_mulle_concurrent_hashmap_patch`/`_remove`). Enumerators similar to the hashtable's.
- `mulle_concurrent_pointerarray` (resizable array):
  ```c
  void  _mulle_concurrent_pointerarray_init( struct mulle_concurrent_pointerarray *array,
                                             unsigned int size, struct mulle_allocator *allocator);
  void  *_mulle_concurrent_pointerarray_get( struct mulle_concurrent_pointerarray *array, unsigned int index);
  void   _mulle_concurrent_pointerarray_add( struct mulle_concurrent_pointerarray *array, void *value);
  int    _mulle_concurrent_pointerarray_find( struct mulle_concurrent_pointerarray *array, void *search);
  unsigned int  _mulle_concurrent_pointerarray_get_count( struct mulle_concurrent_pointerarray *array);
  ```
- `mulle_concurrent_pointerset` (resizable set of `void *`; NULL not allowed as a member):
  ```c
  int   mulle_concurrent_pointerset_insert( struct mulle_concurrent_pointerset *set, void *ptr);
  int   mulle_concurrent_pointerset_member( struct mulle_concurrent_pointerset *set, void *ptr);
  int   mulle_concurrent_pointerset_remove( struct mulle_concurrent_pointerset *set, void *ptr);
  void  *mulle_concurrent_pointerset_register( struct mulle_concurrent_pointerset *set, void *ptr);
  void  *mulle_concurrent_pointerset_lookup_any( struct mulle_concurrent_pointerset *set);
  size_t   mulle_concurrent_pointerset_count( struct mulle_concurrent_pointerset *set);
  ```

### 3.17. `mulle-aba` — ABA-problem solution

- Timestamp-based lock-free reclamation service for concurrent structures (used by every mulle-concurrent structure). Global singleton + per-instance test API.
  ```c
  void   mulle_aba_init( struct mulle_allocator *allocator);
  void   mulle_aba_done( void);
  struct mulle_aba   *mulle_aba_get_global( void);
  void   mulle_aba_set_global( struct mulle_aba *p);
  void   mulle_aba_register( void);          // per worker thread, before touching concurrent structures
  void   mulle_aba_checkin( void);
  void   mulle_aba_unregister( void);        // per worker thread, before exit
  int    mulle_aba_free( void (*p_free)( void *), void *pointer);
  int    mulle_aba_free_owned_pointer( void (*p_free)( void *pointer, void *owner),
                                       void *pointer, void *owner);
  ```

### 3.18. `mulle-fifo`, `mulle-multififo`, `mulle-linkedlist`

- `mulle-fifo` — fixed-size SPSC producer/consumer FIFOs of `void *`. The *fixed* types are generated (`struct mulle__pointerfifoN`, N ∈ {4,8,...,8192}); the *dynamic* type is `struct mulle_pointerfifo`.
  ```c
  struct mulle_pointerfifo
  {
     mulle_atomic_pointer_t   n;
     unsigned int             write;    // producer only
     unsigned int             read;     // consumer only
     unsigned int             size;     // read only after init
     struct mulle_allocator   *allocator;
     mulle_atomic_pointer_t   *storage;
  };
  void   _mulle_pointerfifo_init( struct mulle_pointerfifo *p, unsigned int size,
                                  struct mulle_allocator *allocator);
  int    _mulle_pointerfifo_write( struct mulle_pointerfifo *p, void *pointer);
  void   *_mulle_pointerfifo_read( struct mulle_pointerfifo *p);
  unsigned int   _mulle_pointerfifo_get_count( struct mulle_pointerfifo *p);
  ```
  Fixed variants: `_mulle__pointerfifoN_init/_done/_write/_read/_get_count` (write: `0` ok / `-1` full / `-2` NULL pointer; read: NULL when empty).
- `mulle-multififo` — MPMC FIFOs: `struct mulle_pointermultififo` (lock-free) and `struct mulle_lockingpointermultififo` (mutex-guarded).
  ```c
  void   _mulle_pointermultififo_init( struct mulle_pointermultififo *p,
                                       unsigned int size, struct mulle_allocator *allocator);
  int    _mulle_pointermultififo_write( struct mulle_pointermultififo *p, void *pointer);
  void   *_mulle_pointermultififo_read_barrier( struct mulle_pointermultififo *p);
  ```
  Non-blocking: write returns `-1` with `errno == EBUSY` when full, `-1`/`EINVAL` for invalid pointer; `NULL` and `(void *) ~0` are never stored.
- `mulle-linkedlist` — intrusive single-linked lists. `struct _mulle_linkedlist` (`_head`) for single-threaded, `struct _mulle_concurrent_linkedlist` (`union _mulle_concurrent_atomiclinkedlistentry _head`) for lock-less concurrent use (LIFO semantics, no allocation).
  ```c
  void   _mulle_concurrent_linkedlist_init( struct _mulle_concurrent_linkedlist *p);
  void   _mulle_concurrent_linkedlist_add( struct _mulle_concurrent_linkedlist *list,
                                           struct _mulle_concurrent_linkedlistentry *entry);
  struct _mulle_concurrent_linkedlistentry  *
     _mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list);
  int    _mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list,
                                            int (*callback)( struct _mulle_concurrent_linkedlistentry *,
                                                             struct _mulle_concurrent_linkedlistentry *,
                                                             void *), void *userinfo);
  ```

### 3.19. `mulle-dlfcn` and `mulle-mmap`

- `mulle-dlfcn`: portable `dlopen`-style symbol lookup in the current executable.
  ```c
  static inline void   *mulle_dlsym_exe( const char *name );
  uint32_t   mulle_dlfcn_get_version( void);
  ```
- `mulle-mmap`: memory-mapped file access.
  ```c
  struct mulle_mmap
  {
     void                         *data_;
     size_t                       length_;
     size_t                       mapped_length_;
     int                          is_handle_internal_;
     enum mulle_mmap_accessmode   accessmode_;
     mulle_mmap_file_t            file_handle_;
  };

  void   _mulle_mmap_init( struct mulle_mmap *p, enum mulle_mmap_accessmode mode);
  int    _mulle_mmap_map_file_range( struct mulle_mmap *p, const char *path,
                                     size_t offset, size_t length);
  int    _mulle_mmap_unmap( struct mulle_mmap *p);
  int    _mulle_mmap_sync( struct mulle_mmap *p);
  void   *mulle_mmap_alloc_pages( size_t size);
  struct mulle_mmap_shared_memory   mulle_mmap_alloc_shared_memory( size_t size);
  int   mulle_mmap_free_shared_memory( struct mulle_mmap_shared_memory *mem);
  size_t   mulle_mmap_get_system_pagesize( void);
  ```
  `enum mulle_mmap_accessmode { mulle_mmap_read, mulle_mmap_write, mulle_mmap_no_unmap = 0x80 };`. Accessors: `mulle_mmap_get_bytes/length/mapped_length/is_open/is_mapped/is_empty/is_writable`.

### 3.20. `mulle-dtostr`, `mulle-sprintf`, `mulle-fprintf`

- `mulle-dtostr`: exact double ↔ string conversion (Schubfach-based). Round-trip guaranteed.
  ```c
  struct mulle_dtostr_decimal
  {
     uint64_t   significand;
     int16_t    exponent;
     uint8_t    sign;
     uint8_t    special;
     uint8_t    digits;
     uint8_t    truncated;
     uint16_t   _padding;
  };
  struct mulle_dtostr_decimal   mulle_dtostr_decompose( double value);
  size_t   mulle_dtostr( double value, char *buffer);   // needs MULLE__DTOSTR_BUFFER_SIZE == 25 bytes
  struct mulle_dtostr_decimal   mulle_strtod_parse( const char *s, size_t len, ...);
  double   mulle_strtod_compose( struct mulle_dtostr_decimal decimal);
  int   mulle_strtod_scan( const char *s, size_t len, const struct mulle__strtod_syntax *syntax,
                           double *p_value, char **endptr);
  static inline double   mulle_strtod( const char *s, char **endptr);
  ```
- `mulle-sprintf`: extensible, `va_list`- and `mulle_vararg_list`-capable sprintf family operating on a `struct mulle_buffer`. Returns `-1` on overflow, always NUL-terminates.
  ```c
  int   mulle_buffer_sprintf( struct mulle_buffer *buffer, const char *format, ...);
  int   mulle_buffer_vsprintf( struct mulle_buffer *buffer, const char *format, va_list va);
  int   mulle_buffer_mvsprintf( struct mulle_buffer *buffer, const char *format, mulle_vararg_list va);
  int   mulle_snprintf( char *buf, size_t size, const char *format, ...);
  int   mulle_mvsnprintf( char *buf, size_t size, const char *format, mulle_vararg_list arguments);
  int   mulle_asprintf( char **strp, const char *format, ...);          // allocated result
  int   mulle_allocator_asprintf( struct mulle_allocator *allocator, char **strp,
                                  const char *format, ...);
  ```
  Extensible conversion tables via `struct mulle_sprintf_conversion`/`struct mulle_sprintf_function` and `mulle_sprintf_register_functions/register_default_functions/register_modifier/register_standardmodifiers`. Scoped convenience: `mulle_sprintf_do( string, format, ...)`.
- `mulle-fprintf`: marries `mulle-sprintf` to `stdio.h`. All `printf`/`fprintf` calls are routed through the extensible mulle-sprintf engine.
  ```c
  int   mulle_printf( const char *format, ...);
  int   mulle_fprintf( FILE *fp, const char *format, ...);
  int   mulle_vfprintf( FILE *fp, const char *format, va_list args);
  int   mulle_mvfprintf( FILE *fp, const char *format, mulle_vararg_list arguments);
  static inline int   mulle_puts( const char *s);
  static inline int   mulle_fputs( const char *s, FILE *fp);
  static inline int   mulle_putchar( int c);
  ```
- `mulle-buffer-stdio.h` adds `mulle_buffer_fprint( FILE *fp, struct mulle_buffer *buffer)`.

### 3.21. `mulle-time` — simple time arithmetic

- The three time types are `typedef double` aliases of `mulle_timeinterval_t` (seconds).
  ```c
  typedef double   mulle_timeinterval_t;
  typedef mulle_timeinterval_t   mulle_absolutetime_t;   // monotonic, since boot
  typedef mulle_timeinterval_t   mulle_relativetime_t;   // durations
  typedef mulle_timeinterval_t   mulle_calendartime_t;   // wall clock

  typedef enum
  {
     MulleTimeAscending  = -1,
     MulleTimeSame       = 0,
     MulleTimeDescending = 1
  } mulle_time_comparison_t;

  struct mulle_timeintervalrange   { mulle_timeinterval_t start; mulle_timeinterval_t end; };
  ```
- Functions: `mulle_timeinterval_now`, `mulle_timeinterval_now_monotonic`, `mulle_relativetime_now`, `mulle_absolutetime_now`, `mulle_calendartime_now`, `mulle_relativetime_sleep`, `mulle_relativetime_make_with_s_ns`, `mulle_timeinterval_quantize`, `mulle_absolutetime_init_with_timespec`, plus `timespec_add/sub/compare/make_with_relativetime` and range makers `mulle_absolutetimerange_make/mulle_relativetimerange_make/mulle_calendartimerange_make/mulle_timeintervalrange_make`.

## 4. Performance Characteristics

- Generic guidance by constituent:
  - `mulle_arena`: O(1) bump allocation (no per-allocation free). Reclaims memory only on `reset`/`done`. Ideal for many small short-lived allocations or translator/scratch contexts. Trade-off: no individual frees → memory volume is bounded by peak live usage.
  - `mulle_concurrent_hashtable`: average O(1) `lookup`/`insert`/`remove`; wait-free for readers, cooperative migration triggered at ~50% load; a migration is *helpable* by any thread (a stalled helper can extend it, but total work is proportional to entries). No tombstones, so no probe-chain degradation after removals. Slightly more expensive CAP per write than the legacy hashmap.
  - `mulle_concurrent_hashmap/pointerset/pointerarray`: lock-free; hashmap uses the value word for the frozen/redirect state and keeps tombstones.
  - Containers (`mulle_pointerarray`): O(1) append/access, O(n) remove/insert at arbitrary index, amortized growth. `mulle_map`: average O(1) insert/get/remove. `mulle_pointerqueue`: O(1) push/pop.
  - `mulle_rbtree`: O(log n) insert/remove/find.
  - `mulle_fifo` (SPSC) and `mulle_multififo` (MPMC): O(1) enqueue/dequeue, non-blocking.
  - `mulle-aba`: amortized O(1); deferred reclamation keeps freed-after-read memory alive until all readers of a generation retire. Memory reclamation latency can be delayed compared to direct free.
  - `mulle_sprintf`: general-purpose but extensible; fast paths for common conversions (dtostr path is exact/round-trip, Jeaiii-shifted integer conversion).
- Trade-offs: the amalgamation trades build/link simplicity for a larger library binary. Thread-safety is per-constituent: never assume a plain container is thread-safe; only the `mulle_*concurrent*`, `mulle_fifo`, `mulle_multififo` types advertise lock-/wait-free operation.

## 5. AI Usage Recommendations & Patterns

- **Best practices:**
  - `#include <mulle-core/mulle-core.h>` for everything; link `-lmulle-core`.
  - Always pair `_create`/`_init` with `_destroy`/`_done`. Prefer the `mulle_buffer_do_*` / `mulle_sprintf_do` / `mulle_thread_mutex_do` scoped macros for automatic, break-safe cleanup without manual pairing.
  - When you need `malloc`-like flexibility without freeing, use `mulle_arena`: init once, allocate, `mulle_arena_reset` or `_done` at the end. Pass `mulle_arena_get_allocator( &arena)` into any API that accepts a `struct mulle_allocator *`.
  - For concurrent keyed access use `mulle_concurrent_hashtable`; init with `mulle_concurrent_hashtable_init( &map, expected_size, NULL)`. Never forget `mulle_aba_register()` in worker threads before first use and `mulle_aba_unregister()` before exit.
  - Use the `_init_even`/`_init_positive` hashtable mode to match your key domain: even-mode accepts only even `intptr_t` keys (great for aligned pointers with the low bit unused); positive mode accepts `[1, INTPTR_MAX]`.
  - For slug generation prefer the no-allocation `mulle_slugify( dst, dst_len, src, src_len)` or the `mulle_buffer_add_*_utf8data` variants; only use `mulle_utf8_slugify[_utf8]` when you accept an allocated result (free with `mulle_free`).
- **Common pitfalls:**
  - Do not touch underscore-prefixed fields (`_storage`, `_curr`, `_pages`, ...) — use the accessors.
  - `mulle_buffer` is not thread-safe. `mulle_arena`'s final memory is not freed until `reset`/`done` — don't use it for elements you intend to free individually.
  - `mulle_concurrent_hashtable` keys: hash `0` is `MULLE_CONCURRENT_NO_HASH`; setting the FROZEN bit (top bit in positive mode) is illegal as a key. `NULL` may not be a stored value; use `mulle_concurrent_hashmap` if you must store `NULL`.
  - `mulle_snprintf`-family returns `-1` on overflow (not a negative byte count).
  - Functions returning a "static string" or a borrowed pointer (e.g. buffer `get_*` accessors) must not be freed; only `extract_*` and the documented "you get an allocated string back" functions transfer ownership.
- **Idiomatic usage:**
  - Depends on neither `mulle-atinit` nor `mulle-atexit`/`mulle-testallocator`. If you add them, add `mulle-core` *before* them (`mulle-sde add github:mulle-core/mulle-core`).
  - Use it as a CMake subdirectory and link with `target_link_libraries( ... PRIVATE mulle-core)`.

## 6. Integration Examples

### Example 1: Arena bump allocator (no per-allocation free)

```c
#include <mulle-core/mulle-core.h>
#include <string.h>

int
main( void)
{
   struct mulle_arena        arena;
   struct mulle_allocator    *allocator;
   char                      *a;
   char                      *b;

   mulle_arena_init( &arena, 4096, NULL);          // bump allocator, 4k pages

   allocator = mulle_arena_get_allocator( &arena); // usable as plain allocator
   a         = mulle_allocator_strdup( allocator, "hello");
   b         = mulle_arena_strdup( &arena, "world");
   printf( "%s %s\n", a, b);

   mulle_arena_reset( &arena);                     // reclaim everything at once
   mulle_arena_done( &arena);
   return( 0);
}
```

### Example 2: Resizable wait-free hashtable

```c
#include <mulle-core/mulle-core.h>

typedef struct
{
   int   value;
} payload;

int
main( void)
{
   struct mulle_concurrent_hashtable   map;
   intptr_t                            hash;
   payload                             *p;

   mulle_aba_init( NULL);             // reclamation service
   mulle_aba_register();              // this thread participates

   mulle_concurrent_hashtable_init( &map, 16, NULL);

   hash = (intptr_t) 0x1111;
   if( mulle_concurrent_hashtable_insert( &map, hash, (void *) 0x2222))
      printf( "insert failed\n");

   // iterate; macro re-fetches on ECANCELED (migration)
   mulle_concurrent_hashtable_for( &map, hash, p)
   {
      printf( "%ld -> %p\n", (long) hash, (void *) p);
   }

   mulle_concurrent_hashtable_done( &map);
   mulle_aba_unregister();
   mulle_aba_done();
   return( 0);
}
```

### Example 3: Scoped buffer with `mulle_buffer_do`

```c
#include <mulle-core/mulle-core.h>

int
main( void)
{
   char   *s;

   mulle_buffer_do( buffer)
   {
      mulle_buffer_add_string( buffer, "The answer is ");
      mulle_buffer_sprintf( buffer, "%d", 42);

      // get_string is borrowed, NUL-terminated
      printf( "%s\n", mulle_buffer_get_string( buffer));
   }
   // buffer is done automatically here
   return( 0);
}
```

### Example 4: Slugify into a caller-provided buffer (no allocation)

```c
#include <mulle-core/mulle-core.h>

int
main( void)
{
   char   buf[ 256];

   if( mulle_slugify( buf, sizeof( buf), "Grüße, Welt!", (size_t) -1))
      printf( "%s\n", buf);
   return( 0);
}
```

## 7. Dependencies

The amalgamation itself has no build-time library dependencies beyond the C runtime and OS (pthreads/Windows threads); but it *contains* the following constituent libraries (each documented separately in `mulle-core/<name>/index.md`):

- mulle-c11
- mintomic (vendored, no-build, included via mulle-thread atomics)
- mulle-allocator
- mulle-data
- mulle-buffer
- mulle-container
- mulle-container-debug
- mulle-rbtree
- mulle-rbtree-debug
- mulle-storage
- mulle-http
- mulle-url
- mulle-regex
- mulle-slug
- mulle-unicode
- mulle-utf
- mulle-vararg
- mulle-thread
- mulle-concurrent
- mulle-aba
- mulle-fifo
- mulle-multififo
- mulle-linkedlist
- mulle-dlfcn
- mulle-mmap
- mulle-dtostr
- mulle-sprintf
- mulle-fprintf
- mulle-time

Internal dependency edges among constituents (relevant when reading code): `mulle-concurrent`/`mulle-multififo`/`mulle-aba` consume `mulle-thread` + `mulle-allocator`; `mulle-storage` uses `mulle-allocator` + `mulle-container` pointer array; `mulle-rbtree` uses `mulle-storage` and `mulle-container` value callback; `mulle-sprintf` uses `mulle-buffer`, `mulle-vararg`, `mulle-dtostr`, `mulle-utf`; `mulle-fprintf` uses `mulle-sprintf`.

---

References: README.md (project overview), mulle-core/mulle-core.h (envelope header), per-constituent headers and index.md under mulle-core/<constituent>/, .mulle/etc/sourcetree/config (constituent list).