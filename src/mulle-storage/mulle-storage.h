#ifndef mulle__storage_h__
#define mulle__storage_h__

#include "include.h"

#include <stdint.h>
#include <string.h>

/*
 *  (c) 2023 nat <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE__STORAGE_VERSION  ((0 << 20) | (0 << 8) | 1)


static inline unsigned int   mulle_storage_get_version_major( void)
{
   return( MULLE__STORAGE_VERSION >> 20);
}


static inline unsigned int   mulle_storage_get_version_minor( void)
{
   return( (MULLE__STORAGE_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_storage_get_version_patch( void)
{
   return( MULLE__STORAGE_VERSION & 0xFF);
}


MULLE__STORAGE_GLOBAL
uint32_t   mulle_storage_get_version( void);



struct mulle_storage
{
   struct mulle_structqueue     _structs;
   struct mulle__pointerarray   _freed;
};


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_storage_init( struct mulle_storage *alloc,
                        size_t sizeof_struct,
                        unsigned int alignof_struct,
                        unsigned int capacity,
                        struct mulle_allocator *allocator)
{
   mulle_structqueue_init( &alloc->_structs,
                           sizeof_struct,
                           alignof_struct,
                           capacity,
                           capacity / 16,
                           allocator);
   _mulle__pointerarray_init( &alloc->_freed, capacity / 32, allocator);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_storage_done( struct mulle_storage *alloc)
{
   struct mulle_allocator   *allocator;

   allocator = mulle_structqueue_get_allocator( &alloc->_structs);
   _mulle__pointerarray_done( &alloc->_freed, allocator);
   mulle_structqueue_done( &alloc->_structs);
}


MULLE_C_NONNULL_FIRST
static inline void  *
   _mulle_storage_malloc( struct mulle_storage *alloc)
{
   void   *p;

   p = mulle__pointerarray_pop( &alloc->_freed);
   if( ! p)
      p = mulle_structqueue_reserve( &alloc->_structs);
   return( p);
}


MULLE_C_NONNULL_FIRST
static inline void  *
   _mulle_storage_calloc( struct mulle_storage *alloc)
{
   void   *p;

   p = _mulle_storage_malloc( alloc);
   memset( p, 0, _mulle_structqueue_get_element_size( &alloc->_structs));
   return( p);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_storage_free( struct mulle_storage *alloc, void *p)
{
   struct mulle_allocator   *allocator;

   assert( _mulle__pointerarray_find( &alloc->_freed, p) == mulle_not_found_e);
   _mulle_structqueue_assert_pointer( &alloc->_structs, p);

#if DEBUG
   memset( p, 0xFD, _mulle_structqueue_get_element_size( &alloc->_structs));
#endif
   allocator = mulle_structqueue_get_allocator( &alloc->_structs);
   mulle__pointerarray_add( &alloc->_freed, p, allocator);
}


MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_storage_copy( struct mulle_storage *alloc, void *q)
{
   void   *p;

   p = _mulle_storage_malloc( alloc);
   memcpy( p, q, _mulle_structqueue_get_element_size( &alloc->_structs));
   return( p);
}


MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator *
   _mulle_storage_get_allocator( struct mulle_storage *alloc)
{
   return( _mulle_structqueue_get_allocator( &alloc->_structs));
}


static inline struct mulle_allocator *
   mulle_storage_get_allocator( struct mulle_storage *alloc)
{
   return( alloc ? _mulle_structqueue_get_allocator( &alloc->_structs) : NULL);
}


MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_storage_get_count( struct mulle_storage *alloc)
{
   return( _mulle_structqueue_get_count( &alloc->_structs) -
           _mulle__pointerarray_get_count( &alloc->_freed));
}


static inline unsigned int
   mulle_storage_get_count( struct mulle_storage *alloc)
{
   return( alloc ? _mulle_storage_get_count( alloc) : 0);
}


#include "mulle-indexedstorage.h"

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
