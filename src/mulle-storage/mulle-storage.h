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


/**
 * Initialize the storage with the given parameters.
 *
 * @param alloc The storage allocator.
 * @param sizeof_struct The size of the structure to be stored.
 * @param alignof_struct The alignment of the structure to be stored.
 * @param capacity The initial capacity of the storage.
 * @param allocator The allocator to be used for memory management.
 */
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

/**
 * Finalize the storage, freeing any allocated resources.
 *
 * @param alloc The storage allocator.
 */
MULLE_C_NONNULL_FIRST
static inline void
   _mulle_storage_done( struct mulle_storage *alloc)
{
   struct mulle_allocator   *allocator;

   allocator = mulle_structqueue_get_allocator( &alloc->_structs);
   _mulle__pointerarray_done( &alloc->_freed, allocator);
   mulle_structqueue_done( &alloc->_structs);
}

/**
 * Allocate memory for a new element from the storage.
 * If there are freed elements, reuse one of them.
 *
 * @param alloc The storage allocator.
 * @return A pointer to the allocated memory.
 */
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

/**
 * Allocate and zero-initialize memory for a new element from the storage.
 * If there are freed elements, reuse one of them.
 *
 * @param alloc The storage allocator.
 * @return A pointer to the allocated and zero-initialized memory.
 */
MULLE_C_NONNULL_FIRST
static inline void  *
   _mulle_storage_calloc( struct mulle_storage *alloc)
{
   void   *p;

   p = _mulle_storage_malloc( alloc);
   memset( p, 0, _mulle_structqueue_get_element_size( &alloc->_structs));
   return( p);
}

/**
 * Free an element and add it to the list of freed elements for reuse.
 *
 * @param alloc The storage allocator.
 * @param p The pointer to the element to be freed.
 */
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

/**
 * Allocate memory for a new element and copy the contents of an existing element into it.
 *
 * @param alloc The storage allocator.
 * @param q The pointer to the existing element to be copied.
 * @return A pointer to the newly allocated and copied element.
 */
MULLE_C_NONNULL_FIRST
static inline void *
   _mulle_storage_copy( struct mulle_storage *alloc, void *q)
{
   void   *p;

   p = _mulle_storage_malloc( alloc);
   memcpy( p, q, _mulle_structqueue_get_element_size( &alloc->_structs));
   return( p);
}

/**
 * Get the allocator associated with the storage.
 *
 * @param alloc The storage allocator.
 * @return The allocator associated with the storage.
 */
MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator *
   _mulle_storage_get_allocator( struct mulle_storage *alloc)
{
   return( _mulle_structqueue_get_allocator( &alloc->_structs));
}

/**
 * Get the allocator associated with the storage, or NULL if the storage is NULL.
 *
 * @param alloc The storage allocator.
 * @return The allocator associated with the storage, or NULL if the storage is NULL.
 */
static inline struct mulle_allocator *
   mulle_storage_get_allocator( struct mulle_storage *alloc)
{
   return( alloc ? _mulle_structqueue_get_allocator( &alloc->_structs) : NULL);
}

/**
 * Get the count of active elements in the storage.
 *
 * @param alloc The storage allocator.
 * @return The count of active elements in the storage.
 */
MULLE_C_NONNULL_FIRST
static inline unsigned int
   _mulle_storage_get_count( struct mulle_storage *alloc)
{
   return( _mulle_structqueue_get_count( &alloc->_structs) -
           _mulle__pointerarray_get_count( &alloc->_freed));
}

/**
 * Get the count of active elements in the storage, or 0 if the storage is NULL.
 *
 * @param alloc The storage allocator.
 * @return The count of active elements in the storage, or 0 if the storage is NULL.
 */
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
