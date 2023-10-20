#ifndef mulle__indexedstorage_h__
#define mulle__indexedstorage_h__

#include "include.h"

#include <stdint.h>

//
// The indexed storage stores "nodes" in a struct array, you deal with
// nodes by "indexes". You get the address of a node with a certain "index"
// with _mulle_indexedstorage_get, which is valid as long as you don't add or
// remove items from the storage (->realloc!)
//
struct mulle_indexedstorage
{
   struct mulle_structarray     _structs;
   struct mulle__pointerarray   _freed;
};


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_indexedstorage_init( struct mulle_indexedstorage *alloc,
                               size_t sizeof_struct,
                               unsigned int alignof_struct,
                               unsigned int capacity,
                               struct mulle_allocator *allocator)
{
   mulle_structarray_init( &alloc->_structs,
                           sizeof_struct,
                           alignof_struct,
                           capacity,
                           allocator);
   _mulle__pointerarray_init( &alloc->_freed, capacity / 32, allocator);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_indexedstorage_done( struct mulle_indexedstorage *alloc)
{
   struct mulle_allocator   *allocator;

   allocator = mulle_structarray_get_allocator( &alloc->_structs);
   _mulle__pointerarray_done( &alloc->_freed, allocator);
   mulle_structarray_done( &alloc->_structs);
}


MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_indexedstorage_alloc( struct mulle_indexedstorage *alloc)
{
   void        *index_1;
   uintptr_t   index;

   index_1 = mulle__pointerarray_pop( &alloc->_freed);
   if( index_1)
   {
      index = (uintptr_t) index_1 - 1;
      return( (unsigned int) index);
   }

   index = mulle_structarray_get_count( &alloc->_structs);
   mulle_structarray_reserve( &alloc->_structs);
   return( index);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_indexedstorage_free( struct mulle_indexedstorage *alloc, unsigned int index)
{
   struct mulle_allocator   *allocator;
   void                     *index_1;

   index_1 = (void *) (uintptr_t) index + 1;
   assert( index < mulle_structarray_get_count( &alloc->_structs));
   assert( mulle__pointerarray_find( &alloc->_freed, index_1) == mulle_not_found_e);

#if DEBUG
   memset( _mulle_structarray_get( &alloc->_structs, (unsigned int) (uintptr_t) index),
           0xFD,
           _mulle_structarray_get_element_size( &alloc->_structs));
#endif

   allocator = mulle_structarray_get_allocator( &alloc->_structs);
   mulle__pointerarray_add( &alloc->_freed, index_1, allocator);
}


MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_indexedstorage_get( struct mulle_indexedstorage *alloc, unsigned int index)
{
   assert( index < mulle_structarray_get_count( &alloc->_structs));
   assert( mulle__pointerarray_find( &alloc->_freed, (void *) (uintptr_t) index + 1) == mulle_not_found_e);

   return( _mulle_structarray_get( &alloc->_structs, (unsigned int) index));
}



MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator *
   _mulle_indexedstorage_get_allocator( struct mulle_indexedstorage *alloc)
{
   return( _mulle_structarray_get_allocator( &alloc->_structs));
}


static inline struct mulle_allocator *
   mulle_indexedstorage_get_allocator( struct mulle_indexedstorage *alloc)
{
   return( alloc ? _mulle_structarray_get_allocator( &alloc->_structs) : NULL);
}


MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_indexedstorage_get_count( struct mulle_indexedstorage *alloc)
{
   return( _mulle_structarray_get_count( &alloc->_structs) -
           _mulle__pointerarray_get_count( &alloc->_freed));
}


static inline unsigned int
   mulle_indexedstorage_get_count( struct mulle_indexedstorage *alloc)
{
   return( alloc ? _mulle_indexedstorage_get_count( alloc) : 0);
}



/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-storage-versioncheck.h")
#  include "_mulle-storage-versioncheck.h"
# endif
#endif

#endif
