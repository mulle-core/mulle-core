//
//  mulle_rb.h
//  MulleXLSReader
//
//  Created by Nat! on 24.10.14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//

#ifndef mulle__rbtree_h__
#define mulle__rbtree_h__

#include "include.h"

#include <string.h>


/* this red black tree implementation is based on the code from Jason Evans

   it's fairly different now though.
   
   Nodes have a payload pointer, which is used for comparison. The 
   templating with #defines is gone.


   Checkout this!!!
   https://www.cs.usfca.edu/~galles/visualization/RedBlack.html

 
 */
/*-
 * Copyright (C) 2006 Jason Evans <jasone@FreeBSD.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 *
 * Red-black trees are difficult to explain without lots of diagrams, so little 
 * attempt is made to document this code.  However, an excellent discussion can 
 * be found in the following book, which was used as the reference for writing 
 * this implementation:
 *
 *   Introduction to Algorithms
 *   Thomas H. Cormen, Charles E. Leiserson, and Ronald L. Rivest
 *   MIT Press (1990)
 *   ISBN 0-07-013143-0
 *
 * Some functions use a comparison function pointer, which is expected to have the
 * following prototype:
 *
 *   int (compare *)( void *payload_a, void *payload_b);
 *
 * Interpretation of comparison function return values:
 *
 *   <0 : a_a < a_b
 *    0 : a_a == a_b
 *   >0 : a_a > a_b
 *
 *******************************************************************************
 */
enum
{
   mulle__rbtree_black  = 0,
   mulle__rbtree_red    = 1,
   mulle__rbtree_marker = 0x40,
   mulle__rbtree_dirty  = 0x80
};


/*
 * Node structure.
 */
struct mulle_rbnode
{
   struct mulle_rbnode    *_parent;
   struct mulle_rbnode    *_left;
   struct mulle_rbnode    *_right;
   int                    _color;
   void                   *payload;
};

//
// we have two modes:
// 1)
//    Use payload pointer and no extra bytes. Comparison is called with
//    node->payload as arguments.
//
// 2) Or we use "extra" bytes at the end of the mulle_rbnode
//   (including "payload" which is then not used) In this case we call
//   the comparison with &node->payload!
//
static inline void   *_mulle_rbnode_get_payload( struct mulle_rbnode *a_node)
{
   return( a_node->payload);
}

// does not retain
static inline void   _mulle_rbnode_set_payload( struct mulle_rbnode *a_node, void *payload)
{
   a_node->payload = payload;
}


static inline void   *_mulle_rbnode_get_extra( struct mulle_rbnode *a_node)
{
   return( (void *) &a_node->payload);
}


static inline void   *_mulle_rb_get_node_from_extra( void *value)
{
   value = (void *) &((void **) value)[ 1];                    // dial past payload
   return( &((struct mulle_rbnode *) value)[ -1]);    // now get back to node
}


static inline int   _mulle_rbnode_is_dirty( struct mulle_rbnode *a_node)
{
   return( ! ! (a_node->_color & mulle__rbtree_dirty));
}


static inline void   _mulle_rbnode_set_dirty( struct mulle_rbnode *a_node)
{
   a_node->_color |= mulle__rbtree_dirty;
}

static inline void   _mulle_rbnode_clear_dirty( struct mulle_rbnode *a_node)
{
   a_node->_color &= ~mulle__rbtree_dirty;
}


static inline int   _mulle_rbnode_is_marked( struct mulle_rbnode *a_node)
{
   return( ! ! (a_node->_color & mulle__rbtree_marker));
}


static inline void   _mulle_rbnode_clear_marker( struct mulle_rbnode *a_node)
{
   a_node->_color &= ~mulle__rbtree_marker;
}


static inline void   _mulle_rbnode_set_marker( struct mulle_rbnode *a_node)
{
   a_node->_color |= mulle__rbtree_marker;
}



static inline int   _mulle_rbnode_get_color( struct mulle_rbnode *a_node)
{
   return( a_node->_color & 0x1);
}


static inline int   _mulle_rbnode_is_black( struct mulle_rbnode *a_node)
{
   return( (a_node->_color & 0x1) == mulle__rbtree_black);
}


static inline int   _mulle_rbnode_is_red( struct mulle_rbnode *a_node)
{
   return( (a_node->_color & 0x1) == mulle__rbtree_red);
}


static inline void   _mulle_rbnode_set_color( struct mulle_rbnode *a_node, int color)
{
   assert( color == mulle__rbtree_red || color == mulle__rbtree_black);

   a_node->_color = (a_node->_color & ~0x1) | color;
}


static inline void   _mulle_rbnode_set_red( struct mulle_rbnode *a_node)
{
   a_node->_color = (a_node->_color & ~0x1) | mulle__rbtree_red;
}


static inline void   _mulle_rbnode_set_black( struct mulle_rbnode *a_node)
{
   a_node->_color = (a_node->_color & ~0x1) | mulle__rbtree_black;
}


static inline struct mulle_rbnode *   _mulle_rbnode_get_parent( struct mulle_rbnode *a_node)
{
   return( a_node->_parent);
}


static inline struct mulle_rbnode *   _mulle_rbnode_get_grandparent( struct mulle_rbnode *a_node)
{
   return( a_node->_parent->_parent);
}



struct mulle_rbtree;

enum
{
   mulle_rbtree_option_allow_duplicates = 0x1,
   mulle_rbtree_option_use_extra        = 0x2,
   mulle_rbtree_option_use_dirty        = 0x4,
   mulle_rbtree_option_use_marker       = 0x8
};

#define MULLE__RBTREE_BASE                \
   struct mulle_rbnode          *_root;   \
   struct mulle_rbnode          _nil;     \
   struct mulle_storage         _nodes;   \
   unsigned int                 _options


/* Root structure. */
struct mulle__rbtree
{
   MULLE__RBTREE_BASE;
};



MULLE_C_NONNULL_FIRST
static inline struct mulle_rbnode  *
   _mulle__rbtree_get_nil_node( struct mulle__rbtree *a_tree)
{
   return( &a_tree->_nil);
}


MULLE_C_NONNULL_FIRST
static inline int
   _mulle__rbtree_is_nil_node( struct mulle__rbtree *a_tree,
                               struct mulle_rbnode  *a_node)
{
   return( &a_tree->_nil == a_node);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void   _mulle__rbtree_set_node_color( struct mulle__rbtree *a_tree,
                                                    struct mulle_rbnode *a_node,
                                                    int color)
{
   assert( color == mulle__rbtree_red || color == mulle__rbtree_black);
   assert( &a_tree->_nil != a_node);

   MULLE_C_UNUSED( a_tree);

   _mulle_rbnode_set_color( a_node, color);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void   _mulle__rbtree_set_node_red( struct mulle__rbtree *a_tree,
                                                  struct mulle_rbnode *a_node)
{
   // can't set nil to red!
   assert( &a_tree->_nil != a_node);

   MULLE_C_UNUSED( a_tree);

   _mulle_rbnode_set_color( a_node, mulle__rbtree_red);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void   _mulle__rbtree_set_node_black( struct mulle__rbtree *a_tree,
                                                    struct mulle_rbnode *a_node)
{
   MULLE_C_UNUSED( a_tree);

   _mulle_rbnode_set_color( a_node, mulle__rbtree_black);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle__rbtree_set_node_marked( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node)
{
   if( a_tree->_options & mulle_rbtree_option_use_marker)
      if( a_node != _mulle__rbtree_get_nil_node( a_tree))
         _mulle_rbnode_set_marker( a_node);
}


MULLE_C_NONNULL_FIRST
static inline void
   _mulle__rbtree_enable_marking( struct mulle__rbtree *a_tree)
{
   a_tree->_options |= mulle_rbtree_option_use_marker;
}


MULLE_C_NONNULL_FIRST
static inline size_t
   _mulle__rbtree_get_extra_size( struct mulle__rbtree *a_tree)
{
   return( _mulle_storage_get_element_size( &a_tree->_nodes) - sizeof( struct mulle_rbnode));
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void   *
   _mulle__rbtree_get_node_value( struct mulle__rbtree *a_tree,
                                  struct mulle_rbnode  *a_node)
{
   if( _mulle__rbtree_is_nil_node( a_tree, a_node))
      return( NULL);

   return( (a_tree->_options & mulle_rbtree_option_use_extra)
           ? _mulle_rbnode_get_extra( a_node)
           : _mulle_rbnode_get_payload( a_node));
}


MULLE_C_NONNULL_FIRST_SECOND
static inline struct mulle_rbnode   *
   _mulle__rbtree_get_node_from_extra( struct mulle__rbtree *a_tree,
                                       void  *extra)
{
   assert( a_tree->_options & mulle_rbtree_option_use_extra);

   MULLE_C_UNUSED( a_tree);

   // dial to end of payload then back up to node
   return( &((struct mulle_rbnode *) ((void **) extra + 1))[ -1]);
}



MULLE_C_NONNULL_FIRST_SECOND
static inline void   _mulle__rbtree_init_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *a_node,
                                               void *value)
{
   size_t  extra_size;

   a_node->_parent  = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_left    = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_right   = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_color   = mulle__rbtree_red;
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      a_node->_color |= mulle__rbtree_dirty;
   if( a_tree->_options & mulle_rbtree_option_use_extra)
   {
      extra_size = _mulle__rbtree_get_extra_size( a_tree);
      memcpy( &a_node->payload, value, extra_size);
   }
   else
      a_node->payload  = value;
}



//
// new API, you can provide a callback that will be called on a node when
// its position in the tree changes (due to balancing) and on insert
// node_extra is extra memory added to a node. This memory will be double
// aligned after sizeof( mulle_rbnode) and pointed to by payload by default
//
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__rbtree_init_with_options( struct mulle__rbtree *a_tree,
                                         size_t node_extra,
                                         unsigned int options,
                                         struct mulle_allocator *allocator);


MULLE_C_NONNULL_FIRST
static inline
void   _mulle__rbtree_init( struct mulle__rbtree *a_tree,
                            struct mulle_allocator *allocator)
{
   _mulle__rbtree_init_with_options( a_tree, 0, 0, allocator);
}

// the tree won't release the payloads though, you have to do this
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__rbtree_done( struct mulle__rbtree *a_tree);


MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST
struct mulle_rbnode  *_mulle__rbtree_new_node( struct mulle__rbtree *a_tree,
                                               void *payload);


MULLE_C_NONNULL_FIRST
static inline
void   _mulle__rbtree_free_node( struct mulle__rbtree *a_tree, void *node)
{
   _mulle_storage_free( &a_tree->_nodes, node);
}



/* Operations. */
MULLE_C_NONNULL_FIRST
static inline struct mulle_rbnode    *
   _mulle__rbtree_get_root_node( struct mulle__rbtree *a_tree)
{
   return( a_tree->_root);
}

MULLE_C_NONNULL_FIRST
static inline int
   _mulle__rbtree_is_root_node( struct mulle__rbtree *a_tree,
                                struct mulle_rbnode *node)
{
   return( a_tree->_root == node);
}


//
// we propagate the dirty flag up
//
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST
void   _mulle__rbtree_mark_node_as_dirty( struct mulle__rbtree *a_tree,
                                          struct mulle_rbnode *node);


MULLE_C_NONNULL_FIRST
static inline int  _mulle__rbtree_is_dirty( struct mulle__rbtree *a_tree)
{
   return( _mulle_rbnode_is_dirty( a_tree->_root));
}

/* Operations. */
MULLE_C_NONNULL_FIRST
static inline struct mulle_allocator    *
   _mulle__rbtree_get_allocator( struct mulle__rbtree *a_tree)
{
   return( mulle_storage_get_allocator( &a_tree->_nodes));
}


MULLE_C_NONNULL_FIRST_SECOND
static inline struct mulle_rbnode   *
   _mulle__rbtree_find_leftmost_node( struct mulle__rbtree *a_tree,
                                      struct mulle_rbnode *a_root)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   
   for( p = a_root; p->_left != nil_node; p = p->_left);
   return( p);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline struct mulle_rbnode   *
   _mulle__rbtree_find_rightmost_node( struct mulle__rbtree *a_tree,
                                       struct mulle_rbnode  *a_root)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   for( p = a_root; p->_right != nil_node; p = p->_right);
   return( p);
}



/*
 * Find a match if it exists.  Otherwise, find the next greater node, if one
 * exists.
 */
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node( struct mulle__rbtree *a_tree,
                             void *a_key,
                             int (*a_comp)( void *, void *));

MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node_equal_or_greater( struct mulle__rbtree *a_tree,
                                              void *a_key,
                                              int (*a_comp)( void *, void *));

#if 0
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
struct mulle_rbnode    *_mulle__rbtree_next_node( struct mulle__rbtree *a_tree,
                                                       struct mulle_rbnode *a_node);

MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
struct mulle_rbnode    *_mulle__rbtree_previous_node( struct mulle__rbtree *a_tree,
                                                           struct mulle_rbnode  *a_node);
#endif


// will not return NULL, will return "nil" node
MULLE_C_NONNULL_FIRST_SECOND
MULLE_C_NONNULL_RETURN
static inline struct mulle_rbnode   *
   _mulle__rbtree_next_node( struct mulle__rbtree *a_tree,
                             struct mulle_rbnode  *node)
{
   struct mulle_rbnode   *nil_node;
   struct mulle_rbnode   *parent;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   if( node->_right != nil_node)
   {
      // If there's a right child, go to the leftmost node in right subtree
      return( _mulle__rbtree_find_leftmost_node( a_tree, node->_right));
   }

   // Otherwise, bubble up until we come from a left child
   while( (parent = node->_parent) != nil_node && node == parent->_right)
   {
      node = parent;
   }

   return( parent);
}


// will not return NULL, will return "nil" node
MULLE_C_NONNULL_FIRST_SECOND
MULLE_C_NONNULL_RETURN
static inline struct mulle_rbnode   *
   _mulle__rbtree_previous_node( struct mulle__rbtree *a_tree,
                                 struct mulle_rbnode  *node)
{
   struct mulle_rbnode   *nil_node;
   struct mulle_rbnode   *parent;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   if( node->_left != nil_node)
   {
      // If there's a left child, go to the rightmost node in left subtree
      return( _mulle__rbtree_find_rightmost_node( a_tree, node->_left));
   }

   // Otherwise, bubble up until we come from a left child
   while( (parent = node->_parent) != nil_node && node == parent->_left)
   {
      node = parent;
   }

   return( parent);
}



/* a_key is always the first argument to a_comp. */
MULLE_C_NONNULL_FIRST_THIRD
static inline struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_payload( struct mulle__rbtree *a_tree,
                                          void *a_key,
                                          int (*a_comp)( void *, void *))
{
   int                   r;
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header
   void                  *b_key;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   // this seems like it would be an easy mistake to make
   assert( ! (a_tree->_options & mulle_rbtree_option_use_extra));

   p = a_tree->_root;
   while( p != nil_node)
   {
      b_key = _mulle_rbnode_get_payload( p);
      r     = (*a_comp)( a_key, b_key);
      if( ! r)
          break;
      if( r < 0)
         p = p->_left;
      else
         p = p->_right;
   }
   return( p);
}


MULLE_C_NONNULL_FIRST_THIRD
static inline struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_extra( struct mulle__rbtree *a_tree,
                                        void *a_key,
                                        int (*a_comp)( void *, void *))
{
   int                   r;
   struct mulle_rbnode   *p;
   void                  *b_key;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   assert( a_tree->_options & mulle_rbtree_option_use_extra);

   p = a_tree->_root;
   while( p != nil_node)
   {
      b_key = _mulle_rbnode_get_extra( p);
      r     = (*a_comp)( a_key, b_key);
      if( ! r)
          break;
      if( r < 0)
         p = p->_left;
      else
         p = p->_right;
   }
   return( p);
}


/*
 * Find a match if it exists.  Otherwise, find the next greater node, if one
 * exists.
 */
MULLE_C_NONNULL_FIRST_THIRD
static inline
struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_equal_or_greater_payload( struct mulle__rbtree *a_tree,
                                                           void *a_key,
                                                           int (*a_comp)( void *, void *))
{
   int                   r;
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *n;
   void                  *b_key;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   p   = a_tree->_root;
   while( p != nil_node)
   {
      b_key = _mulle_rbnode_get_payload( p);
      r     = (*a_comp)( a_key, b_key);
      if( ! r)
         break;

      if( r < 0)
      {
         if( p->_left == nil_node)
            break;

         p = p->_left;
      }
      else
      {
         if( p->_right == nil_node)
         {
            n = p;
            p = p->_parent;
            while( p != nil_node && n == p->_right)
            {
               n = p;
               p = p->_parent;
            }
            break;
         }
         p = p->_right;
      }
   }
   return( p);
}


/*
 * Find a match if it exists.  Otherwise, find the next greater node, if one
 * exists.
 */
MULLE_C_NONNULL_FIRST_THIRD
static inline
struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_equal_or_greater_extra( struct mulle__rbtree *a_tree,
                                                         void *a_key,
                                                         int (*a_comp)( void *, void *))
{
   int                   r;
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *n;
   void                  *b_key;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   p = a_tree->_root;
   while( p != nil_node)
   {
      b_key = _mulle_rbnode_get_extra( p);
      r     = (*a_comp)( a_key, b_key);
      if( ! r)
         break;

      if( r < 0)
      {
         if( p->_left == nil_node)
            break;

         p = p->_left;
      }
      else
      {
         if( p->_right == nil_node)
         {
            n = p;
            p = p->_parent;
            while (p != nil_node && n == p->_right)
            {
               n = p;
               p = p->_parent;
            }
            break;
         }
         p = p->_right;
      }
   }
   return( p);
}


//
// returns -1 if node is already in there according to a_comp, otherwise 0.
// After inserting nodes, you can call __mulle__rbtree_walk_dirty to get
// callbacks for all affected nodes (once)
//
// MEMO: do not fall into the trap of directly manipulating the red black
//       tree. only use insert and remove (its doable :))
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND_THIRD
int    _mulle__rbtree_insert_node( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node,
                                   int (*a_comp)( void *, void *));

MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND_THIRD
void   _mulle__rbtree_insert_node_before_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *a_node,
                                               struct mulle_rbnode *successor);

MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND_THIRD
void  _mulle__rbtree_insert_node_after_node( struct mulle__rbtree *a_tree,
                                             struct mulle_rbnode *a_node,
                                             struct mulle_rbnode *predecessor);


//
// the node will be freed by _mulle__rbtree_remove_node do not touch it
// afterwards.
// After inserting nodes, you can call __mulle__rbtree_walk_dirty to get
// callbacks for all affected nodes (once)
//
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__rbtree_remove_node( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node);


MULLE_C_NONNULL_FIRST_SECOND
static inline
void   _mulle__rbtree_walk( struct mulle__rbtree *a_tree,
                            int (*callback)( struct mulle_rbnode *a_node, void *userinfo),
                            void *userinfo)
{
   struct mulle_rbnode   *a_node;
   struct mulle_rbnode   *nil_node; // nil not good for objc in header

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   a_node = _mulle__rbtree_find_leftmost_node( a_tree, _mulle__rbtree_get_root_node( a_tree));
   while( a_node != nil_node)
   {
      if( ! (*callback)( a_node, userinfo))
         break;
      a_node = _mulle__rbtree_next_node( a_tree, a_node);
   }
}


MULLE_C_NONNULL_FIRST_SECOND
static inline
void   _mulle__rbtree_walk_reverse( struct mulle__rbtree *a_tree,
                                    int (*callback)( struct mulle_rbnode *a_node, void *userinfo),
                                    void *userinfo)
{
   struct mulle_rbnode   *a_node;
   struct mulle_rbnode   *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   a_node = _mulle__rbtree_find_rightmost_node( a_tree, _mulle__rbtree_get_root_node( a_tree));
   while( a_node != nil_node)
   {
      if( ! (*callback)( a_node, userinfo))
         break;
      a_node = _mulle__rbtree_previous_node( a_tree, a_node);
   }
}


MULLE_C_NONNULL_FIRST_SECOND_THIRD
static inline
void   __mulle__rbtree_walk_dirty( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node,
                                   void (*callback)( void *node,
                                                     void *left,
                                                     void *right))
{
   struct mulle_rbnode  *nil_node;
   void                 *value;
   void                 *left_value;
   void                 *right_value;

   if( ! _mulle_rbnode_is_dirty( a_node))
      return;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   assert( a_node != nil_node);

   if( a_node->_left != nil_node)
      __mulle__rbtree_walk_dirty( a_tree, a_node->_left, callback);
   if( a_node->_right != nil_node)
      __mulle__rbtree_walk_dirty( a_tree, a_node->_right, callback);

   value       = _mulle__rbtree_get_node_value( a_tree, a_node),
   left_value  = _mulle__rbtree_get_node_value( a_tree, a_node->_left),
   right_value = _mulle__rbtree_get_node_value( a_tree, a_node->_right);

   (*callback)( value, left_value, right_value);

   _mulle_rbnode_clear_dirty( a_node);
}



MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__rbtree_walk_dirty( struct mulle__rbtree *a_tree,
                                  void (*callback)( void *node,
                                                    void *left,
                                                    void *right));



MULLE_C_NONNULL_FIRST_SECOND
static inline
size_t   __mulle__rbtree_walk_count( struct mulle__rbtree *a_tree,
                                     struct mulle_rbnode *a_node)
{
   struct mulle_rbnode  *nil_node;
   size_t               count;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   if( a_node == nil_node)
      return( 0);

   count = __mulle__rbtree_walk_count( a_tree, a_node->_left)
           +
           __mulle__rbtree_walk_count( a_tree, a_node->_right);

   return( count + 1);
}


MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST
size_t   _mulle__rbtree_get_count( struct mulle__rbtree *a_tree);

#endif
