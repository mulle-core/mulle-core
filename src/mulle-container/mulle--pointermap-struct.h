//
//  mulle-container
//
//  Created by Nat! on 03/11/15.
//  Copyright (C) 2011 Nat!, Mulle kybernetiK. All rights reserved.
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
#ifndef mulle__pointermap_struct_h__
#define mulle__pointermap_struct_h__


#include "include.h"

#include "mulle-pointerpair.h"

#include <stdarg.h>


#define MULLE__POINTERMAP_FILL_SHIFT    2
#define MULLE__POINTERMAP_MIN_SIZE      (1 << MULLE__POINTERMAP_FILL_SHIFT)
#define MULLE__POINTERMAP_INITIAL_SIZE  4

// A mulle__pointermap is like a mulle__pointerset with the corresponding
// values appended. So if the set is [ 0, 1, 2 ] the map for { 0, a }
// { 1, b }, { 2, c} is stored as [ 0, 1, 2 ] [ a, b, c]. The value is
// located at offset +_size.
//
// This is a stripped down version of mulle__map without callbacks
// So you just map unique pointers to each other. You can't map
// mulle_not_a_pointer to anything.
//
#define _MULLE__POINTERMAP_BASE   \
   void     **_storage;           \
   size_t   _count;               \
   size_t   _size


#ifndef MULLE__CONTAINER_MISER_MODE

#define MULLE__POINTERMAP_BASE    \
    _MULLE__POINTERMAP_BASE;      \
    uintptr_t  _n_mutations

#else

#define MULLE__POINTERMAP_BASE    \
    _MULLE__POINTERMAP_BASE

#endif



struct mulle__pointermap
{
   MULLE__POINTERMAP_BASE;
};


#define MULLE__POINTERMAP_DATA   { 0 }


MULLE__CONTAINER_GLOBAL
struct mulle_container_keyvaluecallback  mulle__pointermap_keyvaluecallback;


// assume we have _size 4
// 0 1 2 3  (sentinel 0)
//
// we want to hash either
// to 0 or 2 (leave odd alone) for better insert perfomance
//
// (?)
static inline size_t   mulle__pointermap_hash_for_size( uintptr_t  hash,
                                                        size_t _size)
{
   assert( _size >= 2);

   //
   // so if _size is pow2, _size-1 is the mask
   // now sub one again to kill lowest bit (which will be set)
   // if _size is 0, this doesn't work though
   //
   return( hash & (_size - 2)); // kill lowest bit (it will be)
}


#pragma mark - setup and takedown


MULLE__CONTAINER_GLOBAL
struct mulle__pointermap   *mulle__pointermap_create( size_t capacity,
                                                      size_t extra,
                                                      struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__pointermap_destroy( struct mulle__pointermap *map,
                                   struct mulle_allocator *allocator);

static inline void   
   mulle__pointermap_destroy( struct mulle__pointermap *map,
                              struct mulle_allocator *allocator)
{
   if( map)
      _mulle__pointermap_destroy( map, allocator);      
}


MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__pointermap_init( struct mulle__pointermap *map,
                                size_t capacity,
                                struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__pointermap_done( struct mulle__pointermap *map,
                                struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__pointermap_reset( struct mulle__pointermap *map,
                                 struct mulle_allocator *allocator);


static inline void   
   mulle__pointermap_reset( struct mulle__pointermap *map,
                            struct mulle_allocator *allocator)
{
   if( map)
      _mulle__pointermap_reset( map, allocator);      
}


#pragma mark - petty accessors

MULLE_C_NONNULL_FIRST
static inline int   _mulle__pointermap_is_full( struct mulle__pointermap *map)
{
   size_t    _size;

   _size = map->_size;
   _size = (_size - (_size >> MULLE__POINTERMAP_FILL_SHIFT));  // full when 75% occupied
   return( map->_count >= _size);
}

static inline int   mulle__pointermap_is_full( struct mulle__pointermap *map)
{
   return( map ? _mulle__pointermap_is_full( map) : 1);
}


MULLE_C_NONNULL_FIRST
static inline int  _mulle__pointermap_is_sparse_size( struct mulle__pointermap *map, size_t size)
{
   size = size / 2;
   size = (size - (size >> MULLE__POINTERMAP_FILL_SHIFT));
   return( map->_count < size);
}


MULLE_C_NONNULL_FIRST
static inline int   _mulle__pointermap_is_sparse( struct mulle__pointermap *map)
{
   return( _mulle__pointermap_is_sparse_size( map, map->_size));
}


static inline int   mulle__pointermap_is_sparse( struct mulle__pointermap *map)
{
   return( map ? _mulle__pointermap_is_sparse( map) : 0);
}


MULLE_C_NONNULL_FIRST
static inline size_t   _mulle__pointermap_get_count( struct mulle__pointermap *map)
{
   return( map->_count);
}

static inline size_t   mulle__pointermap_get_count( struct mulle__pointermap *map)
{
   return( map ? _mulle__pointermap_get_count( map) : 0);
}


// _size for key really
MULLE_C_NONNULL_FIRST
static inline size_t   _mulle__pointermap_get_size( struct mulle__pointermap *map)
{
   return( map->_size);
}

static inline size_t   mulle__pointermap_get_size( struct mulle__pointermap *map)
{
   return( map ? _mulle__pointermap_get_size( map) : 0);
}


#pragma mark - operations

//
// call this after remove operations, to make enumerations quicker and
// conserve memory
//
MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void  _mulle__pointermap_shrink_if_needed( struct mulle__pointermap *map,
                                           struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   *_mulle__pointermap_get( struct mulle__pointermap *map,
                                void *key);


// returns NULL because that's the notfound *value*!
static inline void   *mulle__pointermap_get( struct mulle__pointermap *map,
                                             void *key)
{
   if( ! map)
      return( NULL);
   return( _mulle__pointermap_get( map, key));
}


// check for mulle_pointerpair_is_invalid if found or not
MULLE__CONTAINER_GLOBAL
struct mulle_pointerpair
   mulle__pointermap_find_by_value( struct mulle__pointermap *map, void *value);


// Experimental!
// You need to provide zeroed space on the initial call for the return value.
// Afterwards you keep it empty.
// The intended use is for iterating over and deleting contents. This could be
// fairly efficient, but its untested.
//
MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
struct mulle_pointerpair   *_mulle__pointermap_get_any_pair( struct mulle__pointermap *map,
                                                             struct mulle_pointerpair *space);


# pragma mark - copy

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__pointermap_copy_items( struct mulle__pointermap *dst,
                                      struct mulle__pointermap *src,
                                      struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
struct mulle__pointermap   *_mulle__pointermap_copy( struct mulle__pointermap *set,
                                                     struct mulle_allocator *allocator);


# pragma mark - enumeration

#define _MULLE__POINTERMAPENUMERATOR_BASE  \
   void     **_curr;                       \
   size_t   _left;                         \
   size_t   _offset

//
// could also use MULLE__CONTAINER_MISER_MODE, but enumerators are not to
// be stored over the length of a function call
//
#if MULLE__CONTAINER_HAVE_MUTATION_COUNT

#define MULLE__POINTERMAPENUMERATOR_BASE   \
    _MULLE__POINTERMAPENUMERATOR_BASE;     \
    struct mulle__pointermap *_map;        \
    uintptr_t  _n_mutations

#else

#define MULLE__POINTERMAPENUMERATOR_BASE   \
    _MULLE__POINTERMAPENUMERATOR_BASE

#endif


struct mulle__pointermapenumerator
{
   MULLE__POINTERMAPENUMERATOR_BASE;
};


static inline struct mulle__pointermapenumerator
   _mulle__pointermap_enumerate( struct mulle__pointermap *map)
{
   struct mulle__pointermapenumerator   rover;

   assert( map);
   rover._left   = map->_count;
   rover._curr   = map->_storage;
   rover._offset = _mulle__pointermap_get_size( map);

#if MULLE__CONTAINER_HAVE_MUTATION_COUNT
   rover._n_mutations = map->_n_mutations;
   rover._map         = map;
#endif

   return( rover);
}


static inline struct mulle__pointermapenumerator
   mulle__pointermap_enumerate( struct mulle__pointermap *map)
{
   if( map)
      return( _mulle__pointermap_enumerate( map));

   return( (struct mulle__pointermapenumerator) { 0 }); // less sanitizer warnings
}



MULLE_C_NONNULL_FIRST
static inline int
   _mulle__pointermapenumerator_next_pair_notakey( struct mulle__pointermapenumerator *rover,
                                                   void  *notakey,
                                                   struct mulle_pointerpair *pair)
{
   void   **p;

   if( ! rover->_left)
   {
      if( pair)
         *pair = mulle_pointerpair_make_invalid();
      return( 0);
   }

#if MULLE__CONTAINER_HAVE_MUTATION_COUNT
   assert( rover->_map->_n_mutations == rover->_n_mutations && "map was modified during enumeration");
#endif

   rover->_left--;
   for(;;)
   {
      p = rover->_curr++;
      if( *p != notakey)
      {
         if( pair)
         {
            pair->key   = *p;
            pair->value = p[ rover->_offset];
         }
         return( 1);
      }
   }
}

//
// the map mustn't have too much free space for this to be reasonably
// efficient, therefore it's important to shrink after lots of removes
//
MULLE_C_NONNULL_FIRST
static inline int
   _mulle__pointermapenumerator_next_pair( struct mulle__pointermapenumerator *rover,
                                           struct mulle_pointerpair *pair)
{
   return( _mulle__pointermapenumerator_next_pair_notakey( rover, mulle_not_a_pointer, pair));
}



MULLE_C_NONNULL_FIRST
static inline int
   _mulle__pointermapenumerator_next_notakey( struct mulle__pointermapenumerator *rover,
                                              void  *notakey,
                                              void **key,
                                              void **value)
{
   void   **p;

   if( ! rover->_left)
   {
      if( key)
         *key = NULL;
      if( value)
         *value = 0;
      return( 0);
   }

#if MULLE__CONTAINER_HAVE_MUTATION_COUNT
   assert( rover->_map->_n_mutations == rover->_n_mutations && "map was modified during enumeration");
#endif

   rover->_left--;
   for(;;)
   {
      p = rover->_curr++;
      if( *p != notakey)
      {
         if( key)
            *key = *p;
         if( value)
            *value = p[ rover->_offset];
         return( 1);
      }
   }
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle__pointermapenumerator_next( struct mulle__pointermapenumerator *rover,
                                      void **key,
                                      void **value)
{
   return( _mulle__pointermapenumerator_next_notakey( rover, mulle_not_a_pointer, key, value));
}


static inline void
   _mulle__pointermapenumerator_done( struct mulle__pointermapenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


static inline void
   mulle__pointermapenumerator_done( struct mulle__pointermapenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


#endif
