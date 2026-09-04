//
//  mulle-arena.h
//  mulle-storage
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_arena_h__
#define mulle_arena_h__

#include "include.h"

#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <string.h>


//
// A bump/arena allocator for variable-sized allocations.
// Individual allocations cannot be freed. All memory is released at once
// via `_mulle_arena_done` or rewound with `_mulle_arena_reset`.
//
// mulle_arena "subclasses" mulle_allocator via MULLE_ALLOCATOR_BASE, so you
// can cast `(struct mulle_allocator *) &arena` and pass it to anything that
// uses the mulle_allocator protocol.
//
// Semantics when used as a mulle_allocator:
//   calloc  - bump allocate + zero
//   realloc - if block is NULL, this is malloc (bump allocate)
//             if block is the last allocation, extend in place if possible
//             otherwise allocate new + memcpy (old space is abandoned)
//   free    - no-op (memory reclaimed only on done/reset)
//
// Oversized allocations (larger than page_size) get their own dedicated page.
//
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


// forward declarations of vtable functions
static void   *_mulle_arena_calloc_v( size_t n, size_t size, struct mulle_allocator *allocator);
static void   *_mulle_arena_realloc_v( void *block, size_t size, struct mulle_allocator *allocator);
static void    _mulle_arena_free_v( void *block, struct mulle_allocator *allocator);


static inline char *
   _mulle_arena_align_pointer( char *p, unsigned int alignment)
{
   uintptr_t   mask;

   mask = (uintptr_t) alignment - 1;
   return( (char *) (((uintptr_t) p + mask) & ~mask));
}


//
// Allocate a new page. If `min_size` exceeds page_size, allocate an
// oversized page that fits exactly.
//
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_arena_new_page( struct mulle_arena *arena, size_t min_size)
{
   size_t                actual_size;
   size_t                n;
   char                  *page;
   struct mulle_allocator *a;

   actual_size = arena->_page_size;
   a           = &mulle_allocator_default;

   // if we have pre-existing pages from a previous cycle (kept on reset),
   // advance into the next one instead of allocating
   n = _mulle__pointerarray_get_count( &arena->_pages);
   if( min_size <= actual_size && arena->_page_index + 1 < n)
   {
      arena->_page_index++;
      page             = _mulle__pointerarray_get( &arena->_pages, arena->_page_index);
      arena->_current  = page;
      arena->_sentinel = page + actual_size;
      return( page);
   }

   // allocate a fresh page (oversized if needed)
   if( min_size > actual_size)
      actual_size = min_size;

   page = _mulle_allocator_calloc( a, 1, actual_size);
   _mulle__pointerarray_add( &arena->_pages, page, a);
   arena->_page_index = _mulle__pointerarray_get_count( &arena->_pages) - 1;

   arena->_current  = page;
   arena->_sentinel = page + actual_size;
   return( page);
}


/**
 * Initialize the arena with the given page size.
 *
 * @param arena      The arena to initialize.
 * @param page_size  Size of each allocation page. A larger value means fewer
 *                   system allocations but higher memory overhead.  Pass 0
 *                   for a sensible default (4096).
 * @param allocator  Unused, present for API consistency. The arena uses the
 *                   default allocator internally for page management.
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_arena_init( struct mulle_arena *arena,
                      size_t page_size,
                      struct mulle_allocator *allocator)
{
   if( ! page_size)
      page_size = 4096;

   memset( arena, 0, sizeof( *arena));

   arena->calloc  = _mulle_arena_calloc_v;
   arena->realloc = _mulle_arena_realloc_v;
   arena->free    = _mulle_arena_free_v;
   arena->fail    = mulle_allocation_fail;
   arena->abafree = NULL;
   arena->aba     = NULL;

   arena->_page_size = page_size;
   _mulle__pointerarray_init( &arena->_pages, 8, &mulle_allocator_default);
}


/**
 * Finalize the arena, freeing all pages.
 *
 * @param arena  The arena to finalize.
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_arena_done( struct mulle_arena *arena)
{
   size_t   i;
   size_t   n;

   n = _mulle__pointerarray_get_count( &arena->_pages);
   for( i = 0; i < n; i++)
      _mulle_allocator_free( &mulle_allocator_default,
                             _mulle__pointerarray_get( &arena->_pages, i));
   _mulle__pointerarray_done( &arena->_pages, &mulle_allocator_default);
}


/**
 * Reset the arena, keeping `keep_count` pages. Freed pages are returned
 * to the system; kept pages are zeroed (poisoned in DEBUG) for reuse.
 *
 * @param arena       The arena to reset.
 * @param keep_count  Number of pages to retain (0 = free all).
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_arena_reset_keep_pages( struct mulle_arena *arena, size_t keep_count)
{
   size_t   i;
   size_t   n;
   char     *page;

   n = _mulle__pointerarray_get_count( &arena->_pages);
   if( ! n)
      return;

   if( keep_count > n)
      keep_count = n;

   // free pages beyond keep_count
   for( i = keep_count; i < n; i++)
      _mulle_allocator_free( &mulle_allocator_default,
                             _mulle__pointerarray_get( &arena->_pages, i));

   // rebuild the pages array with just the kept pages
   {
      struct mulle__pointerarray   old_pages;

      old_pages = arena->_pages;
      _mulle__pointerarray_init( &arena->_pages, keep_count ? keep_count : 1,
                                 &mulle_allocator_default);
      for( i = 0; i < keep_count; i++)
      {
         page = _mulle__pointerarray_get( &old_pages, i);
#if DEBUG
         mulle_memset_uint32( page, 0xDEADDEAD, arena->_page_size);
#else
         memset( page, 0, arena->_page_size);
#endif
         _mulle__pointerarray_add( &arena->_pages, page, &mulle_allocator_default);
      }
      _mulle__pointerarray_done( &old_pages, &mulle_allocator_default);
   }

   if( keep_count)
   {
      page             = _mulle__pointerarray_get( &arena->_pages, 0);
      arena->_current  = page;
      arena->_sentinel = page + arena->_page_size;
   }
   else
   {
      arena->_current  = NULL;
      arena->_sentinel = NULL;
   }
   arena->_last       = NULL;
   arena->_page_index = 0;
}


/**
 * Reset the arena, keeping a percentage of pages.
 *
 * @param arena      The arena to reset.
 * @param percent    Percentage of pages to keep (0-100).
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_arena_reset_keep_percentage( struct mulle_arena *arena,
                                       unsigned int percent)
{
   size_t   n;
   size_t   keep;

   n = _mulle__pointerarray_get_count( &arena->_pages);
   if( percent > 100)
      percent = 100;
   keep = (n * percent + 99) / 100;
   _mulle_arena_reset_keep_pages( arena, keep);
}


/**
 * Reset the arena: free all pages, rewind. Equivalent to
 * `_mulle_arena_reset_keep_pages( arena, 0)`.
 *
 * @param arena  The arena to reset.
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_arena_reset( struct mulle_arena *arena)
{
   _mulle_arena_reset_keep_pages( arena, 0);
}


/**
 * Allocate memory from the arena with the given size and alignment.
 *
 * @param arena      The arena allocator.
 * @param size       Number of bytes to allocate.
 * @param alignment  Required alignment (must be a power of 2, minimum 1).
 * @return Pointer to the allocated memory.
 */
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_arena_alloc( struct mulle_arena *arena,
                       size_t size,
                       unsigned int alignment)
{
   char   *aligned;
   char   *end;

   assert( size > 0);
   assert( alignment > 0 && (alignment & (alignment - 1)) == 0);

   aligned = _mulle_arena_align_pointer( arena->_current, alignment);
   end     = aligned + size;

   if( end > arena->_sentinel)
   {
      _mulle_arena_new_page( arena, size + alignment - 1);
      aligned = _mulle_arena_align_pointer( arena->_current, alignment);
      end     = aligned + size;
   }

   arena->_current = end;
   arena->_last    = aligned;
   return( aligned);
}


/**
 * Allocate zero-initialized memory from the arena.
 *
 * @param arena      The arena allocator.
 * @param size       Number of bytes to allocate.
 * @param alignment  Required alignment (must be a power of 2, minimum 1).
 * @return Pointer to the allocated and zero-initialized memory.
 */
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_arena_calloc( struct mulle_arena *arena,
                        size_t size,
                        unsigned int alignment)
{
   void   *p;

   p = _mulle_arena_alloc( arena, size, alignment);
   memset( p, 0, size);
   return( p);
}


/**
 * Attempt to resize the last allocation in place, or allocate new + copy.
 *
 * @param arena     The arena allocator.
 * @param block     Pointer to the previous allocation (or NULL for malloc).
 * @param old_size  Size of the previous allocation (needed for memcpy).
 * @param new_size  Desired new size.
 * @param alignment Required alignment.
 * @return Pointer to the (possibly moved) allocation.
 */
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_arena_realloc( struct mulle_arena *arena,
                         void *block,
                         size_t old_size,
                         size_t new_size,
                         unsigned int alignment)
{
   char   *end;
   void   *p;

   if( ! block)
      return( _mulle_arena_alloc( arena, new_size, alignment));

   // if block is the last allocation, try to extend in place
   if( (char *) block == arena->_last)
   {
      end = arena->_last + new_size;
      if( end <= arena->_sentinel)
      {
         arena->_current = end;
         return( block);
      }
   }

   // can't extend: allocate new, copy, abandon old
   p = _mulle_arena_alloc( arena, new_size, alignment);
   memcpy( p, block, old_size < new_size ? old_size : new_size);
   return( p);
}


/**
 * Duplicate a block of memory into the arena.
 *
 * @param arena  The arena allocator.
 * @param src    Source memory to copy.
 * @param size   Number of bytes to copy.
 * @return Pointer to the arena-allocated copy.
 */
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_arena_memdup( struct mulle_arena *arena,
                        void *src,
                        size_t size)
{
   void   *p;

   p = _mulle_arena_alloc( arena, size, 1);
   memcpy( p, src, size);
   return( p);
}


/**
 * Duplicate a C string into the arena.
 *
 * @param arena  The arena allocator.
 * @param s      The null-terminated string to duplicate.
 * @return Pointer to the arena-allocated copy.
 */
MULLE_C_NONNULL_FIRST
static inline char *
   _mulle_arena_strdup( struct mulle_arena *arena, char *s)
{
   size_t   len;

   len = strlen( s) + 1;
   return( (char *) _mulle_arena_memdup( arena, s, len));
}


/**
 * Get the arena as a mulle_allocator pointer.
 * This is the primary way to pass the arena to code expecting
 * a mulle_allocator.
 *
 * @param arena  The arena.
 * @return The arena cast to mulle_allocator pointer.
 */
MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator *
   _mulle_arena_get_allocator( struct mulle_arena *arena)
{
   return( (struct mulle_allocator *) arena);
}


/**
 * Get the number of pages allocated by the arena.
 *
 * @param arena  The arena allocator.
 * @return The page count.
 */
MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle_arena_get_page_count( struct mulle_arena *arena)
{
   return( _mulle__pointerarray_get_count( &arena->_pages));
}


// --- mulle_allocator vtable implementations ---

//
// Default alignment for allocations made through the mulle_allocator vtable.
// This matches what malloc() guarantees: suitable for any fundamental type.
//
#define MULLE_ARENA_DEFAULT_ALIGNMENT   alignof( max_align_t)


//
// Vtable alloc: stores the allocation size at ((size_t *) user)[-1].
//
// Layout:  ... [size_t: size][user data (size bytes)]
//                            ^ returned pointer, aligned to max_align_t
//
// The arena's _current pointer is kept at offset (alignment - sizeof(size_t))
// relative to alignment boundaries. This way writing sizeof(size_t) bytes
// advances _current to an aligned position — no wasted padding per alloc.
//
static inline void *
   _mulle_arena_alloc_with_size_header( struct mulle_arena *arena, size_t size)
{
   char     *user;
   char     *end;
   size_t   total;

   total = sizeof( size_t) + size;
   user  = arena->_current + sizeof( size_t);
   end   = user + size;

   if( end > arena->_sentinel || ! arena->_sentinel)
   {
      _mulle_arena_new_page( arena, total + (MULLE_ARENA_DEFAULT_ALIGNMENT - sizeof( size_t)));
      // offset _current so that + sizeof(size_t) is aligned
      arena->_current = arena->_current + (MULLE_ARENA_DEFAULT_ALIGNMENT - sizeof( size_t));
      user = arena->_current + sizeof( size_t);
      end  = user + size;
   }

   ((size_t *) user)[ -1] = size;
   arena->_current = _mulle_arena_align_pointer( end, MULLE_ARENA_DEFAULT_ALIGNMENT);
   arena->_current = arena->_current - sizeof( size_t);  // prep for next header
   arena->_last    = user - sizeof( size_t);  // raw start for last-alloc detection
   return( user);
}


//
// The vtable realloc: handles malloc (block==NULL) and realloc.
// All blocks allocated through this vtable have a size header at
// ((size_t *) block)[-1], so we always know the old size.
//
static inline void *
   _mulle_arena_realloc_v( void *block, size_t size, struct mulle_allocator *allocator)
{
   struct mulle_arena   *arena = (struct mulle_arena *) allocator;
   char                 *raw;
   char                 *end;
   size_t               old_size;
   void                 *p;

   if( ! block)
      return( _mulle_arena_alloc_with_size_header( arena, size));

   old_size = ((size_t *) block)[ -1];
   raw      = (char *) block - sizeof( size_t);

   // if this is the last allocation, try to extend in place
   if( raw == arena->_last)
   {
      end = (char *) block + size;
      if( end <= arena->_sentinel)
      {
         arena->_current = _mulle_arena_align_pointer( end, MULLE_ARENA_DEFAULT_ALIGNMENT);
         arena->_current = arena->_current - sizeof( size_t);
         ((size_t *) block)[ -1] = size;
         return( block);
      }
   }

   // allocate new, copy old data
   p = _mulle_arena_alloc_with_size_header( arena, size);
   memcpy( p, block, old_size < size ? old_size : size);
   return( p);
}


static inline void *
   _mulle_arena_calloc_v( size_t n, size_t size, struct mulle_allocator *allocator)
{
   struct mulle_arena   *arena = (struct mulle_arena *) allocator;
   size_t               total;
   void                 *p;

   total = n * size;
   p     = _mulle_arena_alloc_with_size_header( arena, total);
   memset( p, 0, total);
   return( p);
}


static inline void
   _mulle_arena_free_v( void *block, struct mulle_allocator *allocator)
{
   struct mulle_arena   *arena = (struct mulle_arena *) allocator;
   char                 *raw;

   if( ! block)
      return;

   raw = (char *) block - sizeof( size_t);

   if( raw == arena->_last)
   {
#if DEBUG
      mulle_memset_uint32( raw, 0xDEADDEAD,
                           (size_t) (arena->_current - raw));
#endif
      arena->_current = raw;
      arena->_last    = NULL;
   }
   // else: no-op, reclaimed on done/reset
}


// --- NULL-safe wrappers ---


static inline void
   mulle_arena_init( struct mulle_arena *arena,
                     size_t page_size,
                     struct mulle_allocator *allocator)
{
   if( ! arena)
      return;
   _mulle_arena_init( arena, page_size, allocator);
}


static inline void
   mulle_arena_done( struct mulle_arena *arena)
{
   if( ! arena)
      return;
   _mulle_arena_done( arena);
}


static inline void
   mulle_arena_reset( struct mulle_arena *arena)
{
   if( ! arena)
      return;
   _mulle_arena_reset( arena);
}


static inline void
   mulle_arena_reset_keep_pages( struct mulle_arena *arena, size_t keep_count)
{
   if( ! arena)
      return;
   _mulle_arena_reset_keep_pages( arena, keep_count);
}


static inline void
   mulle_arena_reset_keep_percentage( struct mulle_arena *arena,
                                      unsigned int percent)
{
   if( ! arena)
      return;
   _mulle_arena_reset_keep_percentage( arena, percent);
}


static inline void *
   mulle_arena_alloc( struct mulle_arena *arena,
                      size_t size,
                      unsigned int alignment)
{
   if( ! arena || ! size)
      return( NULL);
   return( _mulle_arena_alloc( arena, size, alignment));
}


static inline void *
   mulle_arena_calloc( struct mulle_arena *arena,
                       size_t size,
                       unsigned int alignment)
{
   if( ! arena || ! size)
      return( NULL);
   return( _mulle_arena_calloc( arena, size, alignment));
}


static inline void *
   mulle_arena_memdup( struct mulle_arena *arena,
                       void *src,
                       size_t size)
{
   if( ! arena || ! src || ! size)
      return( NULL);
   return( _mulle_arena_memdup( arena, src, size));
}


static inline char *
   mulle_arena_strdup( struct mulle_arena *arena, char *s)
{
   if( ! arena || ! s)
      return( NULL);
   return( _mulle_arena_strdup( arena, s));
}


static inline struct mulle_allocator *
   mulle_arena_get_allocator( struct mulle_arena *arena)
{
   return( arena ? (struct mulle_allocator *) arena : NULL);
}


static inline size_t
   mulle_arena_get_page_count( struct mulle_arena *arena)
{
   return( arena ? _mulle_arena_get_page_count( arena) : 0);
}


#endif
