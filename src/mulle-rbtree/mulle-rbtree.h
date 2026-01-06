//
//  mulle-rbtree.c
//  mulle-rbtree
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
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
#ifndef mulle_rbtree_h__
#define mulle_rbtree_h__

#include "mulle--rbtree.h"

#include <errno.h>


#define MULLE__RBTREE_VERSION   ((0UL << 20) | (1 << 8) | 0)


//
// The mulle-rbtree is more convenient than mulle--rbtree. You don't interact
// with the nodes of the red, black tree. You just operate on the payload.
// The payload can be a string, a pointer or an object. The only requirement
// of the payload is, that it can be sorted (stably) with the comparison
// function. The payload will be retain/release with the callback. By default
// the values are assumed to be non-owned c-strings (no strdup, no free!)
//
struct mulle_rbtree
{
   MULLE__RBTREE_BASE;
   int                                     (*comparison)( void *a, void *b);
   void                                    (*dirty)( void *a, void *b, void *c);
   struct mulle_container_valuecallback    callback;
};

MULLE_C_NONNULL_FIRST
MULLE__RBTREE_GLOBAL
void   _mulle_rbtree_init( struct mulle_rbtree *a_tree,
                           int (*a_comp)( void *, void *),
                           struct mulle_container_valuecallback *callback,
                           struct mulle_allocator *allocator);

static inline void
   mulle_rbtree_init( struct mulle_rbtree *a_tree,
                      int (*a_comp)( void *, void *),
                      struct mulle_container_valuecallback *callback,
                      struct mulle_allocator *allocator)
{
   if( ! a_tree)
      return;
   _mulle_rbtree_init( a_tree, a_comp, callback, allocator);
}


struct mulle_rbtree_config
{
   int                                     (*comparison)( void *a, void *b);
   void                                    (*dirty)( void *a, void *b, void *c);
   struct mulle_container_valuecallback    *callback;
   size_t                                  node_extra;
   unsigned int                            options;
};


MULLE_C_NONNULL_FIRST_SECOND
MULLE__RBTREE_GLOBAL
void   _mulle_rbtree_init_with_config( struct mulle_rbtree *a_tree,
                                       struct mulle_rbtree_config *config,
                                       struct mulle_allocator *allocator);

MULLE_C_NONNULL_SECOND
static inline void
   mulle_rbtree_init_with_config( struct mulle_rbtree *a_tree,
                                  struct mulle_rbtree_config *config,
                                  struct mulle_allocator *allocator)
{
   if( ! a_tree)
      return;
   _mulle_rbtree_init_with_config( a_tree, config, allocator);
}



MULLE_C_NONNULL_FIRST
void   _mulle_rbtree_done( struct mulle_rbtree *a_tree);

// this cleans the pay
static inline void   mulle_rbtree_done( struct mulle_rbtree *a_tree)
{
   if( a_tree)
      _mulle_rbtree_done( a_tree);
}


MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator    *
   _mulle_rbtree_get_allocator( struct mulle_rbtree *a_tree)
{
   return( _mulle__rbtree_get_allocator( (struct mulle__rbtree *) a_tree));
}


static inline struct mulle_allocator    *
   mulle_rbtree_get_allocator( struct mulle_rbtree *a_tree)
{
   return( a_tree ? _mulle__rbtree_get_allocator( (struct mulle__rbtree *) a_tree) : NULL);
}



static inline void   _mulle_rbtree_walk_dirty( struct mulle_rbtree *a_tree)
{
   _mulle__rbtree_walk_dirty( (struct mulle__rbtree *) a_tree,
                              a_tree->dirty);

}



// 0: success
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
int   _mulle_rbtree_add( struct mulle_rbtree *a_tree, void *value);


// returns 0 on success
MULLE_C_NONNULL_SECOND
static inline int
   mulle_rbtree_add( struct mulle_rbtree *a_tree, void *value)
{
   if( ! a_tree)
      return( EINVAL);
   return( _mulle_rbtree_add( a_tree, value));
}



MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle_rbtree_remove_node( struct mulle_rbtree *a_tree,
                                  struct mulle_rbnode *node);


MULLE_C_NONNULL_SECOND
static inline void
   mulle_rbtree_remove_node( struct mulle_rbtree *a_tree,
                             struct mulle_rbnode *node)
{
   if( ! a_tree)
      return;
   _mulle_rbtree_remove_node( a_tree, node);
}



// return 0 on succesful removal
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
int   _mulle_rbtree_remove( struct mulle_rbtree *a_tree, void *value);


MULLE_C_NONNULL_SECOND
static inline int
   mulle_rbtree_remove( struct mulle_rbtree *a_tree, void *value)
{
   if( ! a_tree)
      return( EINVAL);
   return( _mulle_rbtree_remove( a_tree, value));
}


/* a_key is always the first argument to a_comp. */
static inline void    *
   mulle_rbtree_find( struct mulle_rbtree *a_tree, void *a_key)
{
   struct mulle_rbnode   *node;

   if( ! a_tree)
      return( NULL);

   node = _mulle__rbtree_find_node( (struct mulle__rbtree *) a_tree,
                                    a_key,
                                    a_tree->comparison);
   return( _mulle__rbtree_get_node_value( (struct mulle__rbtree *) a_tree, node));
}


/*
 * Find a match if it exists.  Otherwise, find the next greater node, if one
 * exists.
 */
static inline void *
   mulle_rbtree_find_equal_or_greater( struct mulle_rbtree *a_tree, void *a_key)
{
   struct mulle_rbnode   *node;

   if( ! a_tree)
      return( NULL);

   node = _mulle__rbtree_find_node_equal_or_greater( (struct mulle__rbtree *) a_tree,
                                                     a_key,
                                                     a_tree->comparison);
   return( _mulle__rbtree_get_node_value( (struct mulle__rbtree *) a_tree, node));
}


MULLE_C_NONNULL_SECOND
static inline
void   mulle_rbtree_walk( struct mulle_rbtree *a_tree,
                          int (*callback)( void *value, void *userinfo),
                          void *userinfo)
{
   struct mulle_rbnode    *a_node;
   struct mulle__rbtree   *a__tree;
   void                   *value;

   if( ! a_tree)
      return;

   a__tree = (struct mulle__rbtree *) a_tree;
   a_node  = _mulle__rbtree_find_leftmost_node( a__tree,
                                              _mulle__rbtree_get_root_node( a__tree));
   while( a_node != _mulle__rbtree_get_nil_node( a__tree))
   {
      value = _mulle__rbtree_get_node_value( (struct mulle__rbtree *) a_tree, a_node);
      if( ! (*callback)( value, userinfo))
         break;
      a_node = _mulle__rbtree_next_node( a__tree, a_node);
   }
}


#pragma mark - value enumerator

// these enumerators return node *values*
struct mulle_rbtreeenumerator
{
   struct mulle__rbtree   *_tree;
   struct mulle_rbnode    *_node;
};


static inline struct mulle_rbtreeenumerator
   mulle_rbtree_enumerate( struct mulle_rbtree *a_tree)
{
   struct mulle_rbtreeenumerator   rover = { 0 };

   if( ! a_tree)
      return( rover);

   rover._tree = (struct mulle__rbtree *) a_tree;
   rover._node = _mulle__rbtree_find_leftmost_node( rover._tree,
                                                    _mulle__rbtree_get_root_node( rover._tree));
   return( rover);
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle_rbtreeenumerator_next( struct mulle_rbtreeenumerator *rover, void **item)
{
   if( ! rover->_node)
      return( 0);
   if( rover->_node == _mulle__rbtree_get_nil_node( rover->_tree))
      return( 0);
   if( item)
      *item = _mulle__rbtree_get_node_value( (struct mulle__rbtree *) rover->_tree, rover->_node);
   rover->_node = _mulle__rbtree_next_node( rover->_tree, rover->_node);
   return( 1);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_rbtreeenumerator_done( struct mulle_rbtreeenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


static inline void
   mulle_rbtreeenumerator_done( struct mulle_rbtreeenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


// these enumerators return node *values*
#define mulle_rbtree_for( a_tree, item)                                                      \
      for( struct mulle_rbtreeenumerator rover__ ## item = mulle_rbtree_enumerate( a_tree);  \
           _mulle_rbtreeenumerator_next( &rover__ ## item, (void **) &item);)

/**
 **/

// these enumerators return node *values*

struct mulle_rbtreereverseenumerator
{
   struct mulle__rbtree   *_tree;
   struct mulle_rbnode    *_node;
};


static inline struct mulle_rbtreereverseenumerator
   mulle_rbtree_reverseenumerate( struct mulle_rbtree *a_tree)
{
   struct mulle_rbtreereverseenumerator   rover = { 0 };

   if( ! a_tree)
      return( rover);

   rover._tree = (struct mulle__rbtree *) a_tree;
   rover._node = _mulle__rbtree_find_rightmost_node( rover._tree,
                                                    _mulle__rbtree_get_root_node( rover._tree));
   return( rover);
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle_rbtreereverseenumerator_next( struct mulle_rbtreereverseenumerator *rover, void **item)
{
   if( ! rover->_node)
      return( 0);
   if( rover->_node == _mulle__rbtree_get_nil_node( rover->_tree))
      return( 0);
   if( item)
      *item = _mulle__rbtree_get_node_value( (struct mulle__rbtree *) rover->_tree, rover->_node);
   rover->_node = _mulle__rbtree_previous_node( rover->_tree, rover->_node);
   return( 1);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle_rbtreereverseenumerator_done( struct mulle_rbtreereverseenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}


static inline void
   mulle_rbtreereverseenumerator_done( struct mulle_rbtreereverseenumerator *rover)
{
   MULLE_C_UNUSED( rover);
}

// these enumerators return node *values*
#define mulle_rbtree_reversefor( a_tree, item)                                                      \
      for( struct mulle_rbtreereverseenumerator rover__ ## item = mulle_rbtree_reverseenumerate( a_tree);  \
           _mulle_rbtreereverseenumerator_next( &rover__ ## item, (void **) &item);)


#endif
