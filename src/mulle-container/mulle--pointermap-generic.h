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
#ifndef mulle__pointermap_generic_h__
#define mulle__pointermap_generic_h__

#include "mulle--pointermap-struct.h"


# pragma mark - callback access with mutation check


#if MULLE__CONTAINER_HAVE_MUTATION_COUNT

static inline uintptr_t   _mulle__pointermap_keycallback_hash( struct mulle__pointermap *map,
                                                               struct mulle_container_keycallback *callback,
                                                               void *a)
{
   uintptr_t   memo_map;
   uintptr_t   hash;

   memo_map = map->_n_mutations;
   hash     = (*callback->hash)( callback, a);
   assert( map->_n_mutations == memo_map && "map was modified during hash callback");

   return( hash);
}

static inline int   _mulle__pointermap_keycallback_equal( struct mulle__pointermap *map,
                                                          struct mulle_container_keycallback *callback,
                                                          void *a,
                                                          void *b)
{
   uintptr_t   memo_map;
   int         is_equal;

   memo_map = map->_n_mutations;
   is_equal = (*callback->is_equal)( callback, a, b);
   assert( map->_n_mutations == memo_map && "map was modified during is_equal callback");

   return( is_equal);
}


static inline void   *_mulle__pointermap_keycallback_retain( struct mulle__pointermap *map,
                                                             struct mulle_container_keycallback *callback,
                                                             void *p,
                                                             struct mulle_allocator *allocator)
{
   uintptr_t   memo_map;

   memo_map = map->_n_mutations;
   p        = (*callback->retain)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during retain callback");
   return( p);
}


static inline void   _mulle__pointermap_keycallback_release( struct mulle__pointermap *map,
                                                             struct mulle_container_keycallback *callback,
                                                             void *p,
                                                             struct mulle_allocator *allocator)
{
   uintptr_t   memo_map;

   memo_map = map->_n_mutations;
   (*callback->release)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during release callback");

}


static inline char   *_mulle__pointermap_keycallback_describe( struct mulle__pointermap *map,
                                                               struct mulle_container_keycallback *callback,
                                                               void *p,
                                                               struct mulle_allocator **allocator)
{
   uintptr_t   memo_map;
   char        *s;

   memo_map = map->_n_mutations;
   s        = (*callback->describe)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during release callback");
   return( s);
}


static inline void   *_mulle__pointermap_valuecallback_retain( struct mulle__pointermap *map,
                                                               struct mulle_container_valuecallback *callback,
                                                               void *p,
                                                               struct mulle_allocator *allocator)
{
   uintptr_t   memo_map;

   memo_map = map->_n_mutations;
   p        = (*callback->retain)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during retain callback");
   return( p);
}


static inline void   _mulle__pointermap_valuecallback_release( struct mulle__pointermap *map,
                                                               struct mulle_container_valuecallback *callback,
                                                               void *p,
                                                               struct mulle_allocator *allocator)
{
   uintptr_t   memo_map;

   memo_map = map->_n_mutations;
   (*callback->release)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during release callback");

}


static inline char   *_mulle__pointermap_valuecallback_describe( struct mulle__pointermap *map,
                                                                 struct mulle_container_valuecallback *callback,
                                                                 void *p,
                                                                 struct mulle_allocator **allocator)
{
   uintptr_t   memo_map;
   char        *s;

   memo_map = map->_n_mutations;
   s        = (*callback->describe)( callback, p, allocator);
   assert( map->_n_mutations == memo_map && "map was modified during release callback");
   return( s);
}


#else

static inline uintptr_t   _mulle__pointermap_keycallback_hash( struct mulle__pointermap *map,
                                                               struct mulle_container_keycallback *callback,
                                                               void *a)
{
   uintptr_t   hash;

   MULLE_C_UNUSED( map);

   hash = (*callback->hash)( callback, a);
   return( hash);
}


static inline int   _mulle__pointermap_keycallback_equal( struct mulle__pointermap *map,
                                                          struct mulle_container_keycallback *callback,
                                                          void *a,
                                                          void *b)
{
   int   is_equal;

   MULLE_C_UNUSED( map);

   is_equal = (*callback->is_equal)( callback, a, b);
   return( is_equal);
}


static inline void   *_mulle__pointermap_keycallback_retain( struct mulle__pointermap *map,
                                                             struct mulle_container_keycallback *callback,
                                                             void *p,
                                                             struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( map);

   p = (*callback->retain)( callback, p, allocator);
   return( p);
}


static inline void   _mulle__pointermap_keycallback_release( struct mulle__pointermap *map,
                                                             struct mulle_container_keycallback *callback,
                                                             void *p,
                                                             struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( map);

   (*callback->release)( callback, p, allocator);
}


static inline char   *_mulle__pointermap_keycallback_describe( struct mulle__pointermap *map,
                                                               struct mulle_container_keycallback *callback,
                                                               void *p,
                                                               struct mulle_allocator **allocator)
{
   char   *s;

   MULLE_C_UNUSED( map);

   s = (*callback->describe)( callback, p, allocator);
   return( s);
}


static inline void   *_mulle__pointermap_valuecallback_retain( struct mulle__pointermap *map,
                                                               struct mulle_container_valuecallback *callback,
                                                               void *p,
                                                               struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( map);

   p = (*callback->retain)( callback, p, allocator);
   return( p);
}


static inline void   _mulle__pointermap_valuecallback_release( struct mulle__pointermap *map,
                                                               struct mulle_container_valuecallback *callback,
                                                               void *p,
                                                               struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( map);

   (*callback->release)( callback, p, allocator);
}


static inline char   *_mulle__pointermap_valuecallback_describe( struct mulle__pointermap *map,
                                                                 struct mulle_container_valuecallback *callback,
                                                                 void *p,
                                                                 struct mulle_allocator **allocator)
{
   char   *s;

   MULLE_C_UNUSED( map);

   s = (*callback->describe)( callback, p, allocator);
   return( s);
}


#endif


//
// mulle_pointermap_generic is the base for mulle-map
// with the passing of the callbacks, this is now more generic to use than
// pure mulle__pointermap
//

MULLE__CONTAINER_GLOBAL
void   **mulle__pointermap_allocate_storage_generic( size_t n,
                                                     void *notakey,
                                                     struct mulle_allocator *allocator);


MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__pointermap_reset_generic( struct mulle__pointermap *map,
                                         void *notakey);


MULLE_C_NONNULL_FIRST_SECOND_THIRD
static inline
void   _mulle__pointermap_set_pair_generic( struct mulle__pointermap *map,
                                            struct mulle_pointerpair *pair,
                                            struct mulle_container_keyvaluecallback *callback,
                                            struct mulle_allocator *allocator)
{
   MULLE__CONTAINER_GLOBAL
   void   *_mulle__pointermap_write_pair_generic( struct mulle__pointermap *map,
                                                  struct mulle_pointerpair *pair,
                                                  enum mulle_container_write_mode mode,
                                                  struct mulle_container_keyvaluecallback *callback,
                                                  struct mulle_allocator *allocator);

   _mulle__pointermap_write_pair_generic( map,
                                          pair,
                                          mulle_container_overwrite_e,
                                          callback,
                                          allocator);
}


MULLE_C_NONNULL_FIRST_SECOND_THIRD
static inline
   void    *_mulle__pointermap_register_pair_generic( struct mulle__pointermap *map,
                                                      struct mulle_pointerpair *pair,
                                                      struct mulle_container_keyvaluecallback *callback,
                                                      struct mulle_allocator *allocator)
{
   void   *value;

   MULLE__CONTAINER_GLOBAL
   void   *_mulle__pointermap_write_pair_generic( struct mulle__pointermap *map,
                                                  struct mulle_pointerpair *pair,
                                                  enum mulle_container_write_mode mode,
                                                  struct mulle_container_keyvaluecallback *callback,
                                                  struct mulle_allocator *allocator);

   value = _mulle__pointermap_write_pair_generic( map, pair, mulle_container_insert_e, callback, allocator);
   return( value ? value : pair->value);
}


MULLE_C_NONNULL_FIRST_SECOND_THIRD
static inline
int    _mulle__pointermap_insert_pair_generic( struct mulle__pointermap *map,
                                               struct mulle_pointerpair *pair,
                                               struct mulle_container_keyvaluecallback *callback,
                                               struct mulle_allocator *allocator)
{
   MULLE__CONTAINER_GLOBAL
   void   *_mulle__pointermap_write_pair_generic( struct mulle__pointermap *map,
                                                  struct mulle_pointerpair *pair,
                                                  enum mulle_container_write_mode mode,
                                                  struct mulle_container_keyvaluecallback *callback,
                                                  struct mulle_allocator *allocator);

   return( _mulle__pointermap_write_pair_generic( map, pair, mulle_container_insert_e, callback, allocator) == NULL);
}



MULLE_C_NONNULL_FIRST_SECOND_THIRD
static inline void *
   _mulle__pointermap_update_pair_generic( struct mulle__pointermap *map,
                                           struct mulle_pointerpair *pair,
                                           struct mulle_container_keyvaluecallback *callback,
                                           struct mulle_allocator *allocator)
{
   MULLE__CONTAINER_GLOBAL
   void   *_mulle__pointermap_write_pair_generic( struct mulle__pointermap *map,
                                                  struct mulle_pointerpair *pair,
                                                  enum mulle_container_write_mode mode,
                                                  struct mulle_container_keyvaluecallback *callback,
                                                  struct mulle_allocator *allocator);

   // can do this only if the release is a nop (could also do this technically
   // with autoreleases, but we don't know it here)
   assert( ! _mulle_container_valuecallback_releases( &callback->valuecallback));
   return( _mulle__pointermap_write_pair_generic( map, pair, mulle_container_update_e, callback, allocator));
}




MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_FOURTH
void   *
   _mulle__pointermap__get_generic_knownhash( struct mulle__pointermap *map,
                                              void *key,
                                              uintptr_t hash,
                                              struct mulle_container_keyvaluecallback *callback);

//
// the __get function does not do a quick check for pointer equality.
// If you know that a pointer equality check will fail anyway, you can
// save some time using __get.
//
MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
void   *_mulle__pointermap__get_generic( struct mulle__pointermap *map,
                                         void *key,
                                         struct mulle_container_keyvaluecallback *callback);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD_FOURTH
struct mulle_pointerpair   *_mulle__pointermap__get_pair_generic( struct mulle__pointermap *map,
                                                                  void *key,
                                                                  struct mulle_container_keyvaluecallback *callback,
                                                                  struct mulle_pointerpair *space);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND_THIRD
struct mulle_pointerpair   *_mulle__pointermap_get_any_pair_generic( struct mulle__pointermap *map,
                                                                     struct mulle_container_keyvaluecallback *callback,
                                                                     struct mulle_pointerpair *space);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_FOURTH
void   *_mulle__pointermap_get_generic_knownhash( struct mulle__pointermap *map,
                                                  void *key,
                                                  uintptr_t hash,
                                                  struct mulle_container_keyvaluecallback *callback);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
void   *_mulle__pointermap_get_generic( struct mulle__pointermap *map,
                                        void *key,
                                        struct mulle_container_keyvaluecallback *callback);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_pointerpair   *_mulle__pointermap_get_pair_generic( struct mulle__pointermap *map,
                                                                 void *key,
                                                                 struct mulle_container_keyvaluecallback *callback,
                                                                 struct mulle_pointerpair *space);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
int   _mulle__pointermap_remove_generic( struct mulle__pointermap *map,
                                         void *key,
                                         struct mulle_container_keyvaluecallback *callback,
                                         struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__pointermap_shrink_generic( struct mulle__pointermap *map,
                                          struct mulle_container_keyvaluecallback *callback,
                                          struct mulle_allocator *allocator);


MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND_THIRD
void   _mulle__pointermap_copy_items_generic( struct mulle__pointermap *dst,
                                              struct mulle__pointermap *src,
                                              struct mulle_container_keyvaluecallback *callback,
                                              struct mulle_allocator *allocator);

MULLE__CONTAINER_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
size_t
   _mulle__pointermap_count_collisions_generic( struct mulle__pointermap *set,
                                                struct mulle_container_keyvaluecallback *callback,
                                                size_t *perfects);


# pragma mark - enumeration


#define MULLE__GENERICPOINTERMAPENUMERATOR_BASE      \
   struct mulle__pointermapenumerator  _base;        \
   struct mulle_pointerpair            _space;       \
   void                                *_notakey


struct mulle__genericpointermapenumerator
{
   MULLE__GENERICPOINTERMAPENUMERATOR_BASE;
};



static inline struct mulle__genericpointermapenumerator
   mulle__pointermap_enumerate_generic( struct mulle__pointermap *map,
                                        struct mulle_container_keyvaluecallback *callback)
{
   struct mulle__genericpointermapenumerator   rover = { 0 }; // awkward, but sanitizes better

   if( map)
   {
      rover._base    = _mulle__pointermap_enumerate( map);
      rover._notakey = callback ? callback->keycallback.notakey : 0;
   }

   return( rover);
}


//
// the map mustn't have too much free space for this to be reasonably
// efficient, therefore it's important to shrink after lots of removes
//
MULLE_C_NONNULL_FIRST
static inline struct mulle_pointerpair *
   _mulle__genericpointermapenumerator_next_pair( struct mulle__genericpointermapenumerator *rover)
{
   if( _mulle__pointermapenumerator_next_pair_notakey( &rover->_base, 
                                                       rover->_notakey, 
                                                       &rover->_space))
      return( &rover->_space);
   return( NULL);
}


static inline struct mulle_pointerpair *
   mulle__genericpointermapenumerator_next_pair( struct mulle__genericpointermapenumerator *rover)
{
   if( ! rover)
      return( NULL);
   return( _mulle__genericpointermapenumerator_next_pair( rover));
}



MULLE_C_NONNULL_FIRST
static inline int
   _mulle__genericpointermapenumerator_next( struct mulle__genericpointermapenumerator *rover,
                                             void **key,
                                             void **value)
{
   return( _mulle__pointermapenumerator_next_notakey( &rover->_base, 
                                                      rover->_notakey, 
                                                      key, 
                                                      value));
}


static inline int
   mulle__genericpointermapenumerator_next( struct mulle__genericpointermapenumerator *rover,
                                            void **key,
                                            void **value)
{
   if( ! rover)
   {
      if( key)
         *key = NULL;
      if( value)
         *value = NULL;
      return( 0);
   }

   return( _mulle__genericpointermapenumerator_next( rover, key, value));
}



MULLE_C_NONNULL_FIRST
static inline void
   _mulle__genericpointermapenumerator_done( struct mulle__genericpointermapenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


static inline void
   mulle__genericpointermapenumerator_done( struct mulle__genericpointermapenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}

#endif
