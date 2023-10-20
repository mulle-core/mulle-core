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
   mulle__rbtree_black = 0,
   mulle__rbtree_red   = 1
};


/*
 * Node structure.
 * MEMO: we could also use offsets into the nodearray
 */
struct mulle_rbnode
{
   struct mulle_rbnode    *_parent;
   struct mulle_rbnode    *_left;
   struct mulle_rbnode    *_right;
   void                   *payload;
   char                   _color;
};


static inline void   *_mulle_rbnode_get_payload( struct mulle_rbnode  *a_node)
{
   return( a_node->payload);
}


static inline void   *mulle_rbnode_get_payload( struct mulle_rbnode  *a_node)
{
   return( a_node ? a_node->payload : NULL);
}



#define MULLE__RBTREE_BASE          \
   struct mulle_rbnode    *_root;   \
   struct mulle_rbnode    _nil;     \
   struct mulle_storage   _nodes;   \
   char                   allow_duplicates


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


MULLE_C_NONNULL_FIRST_SECOND
static inline void   _mulle__rbtree_init_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *a_node,
                                               void *payload)
{
   a_node->_parent  = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_left    = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_right   = _mulle__rbtree_get_nil_node( a_tree);
   a_node->_color   = mulle__rbtree_black;
   a_node->payload  = payload;
}



MULLE_C_NONNULL_FIRST
void   _mulle__rbtree_init( struct mulle__rbtree *a_tree,
                            struct mulle_allocator *allocator);

// the tree won't release the payloads though, you have to do this
MULLE_C_NONNULL_FIRST
void   _mulle__rbtree_done( struct mulle__rbtree *a_tree);


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
   
   for( p = a_root; p->_left != _mulle__rbtree_get_nil_node( a_tree); p = p->_left);
   return( p);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline struct mulle_rbnode   *
   _mulle__rbtree_find_rightmost_node( struct mulle__rbtree *a_tree,
                                       struct mulle_rbnode  *a_root)
{
   struct mulle_rbnode   *p;
   
   for( p = a_root; p->_right != _mulle__rbtree_get_nil_node( a_tree); p = p->_right);
   return( p);
}


/* a_key is always the first argument to a_comp. */
MULLE_C_NONNULL_FIRST_THIRD
static inline struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_payload( struct mulle__rbtree *a_tree,
                                          void *a_key,
                                          int (*a_comp)( void *, void *))
{
   int             t;
   struct mulle_rbnode    *p;


   p = a_tree->_root;
   while( p != _mulle__rbtree_get_nil_node( a_tree) && (t = (a_comp)((a_key), p->payload)) != 0)
   {
      if( t == -1)
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
struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_equal_or_greater_payload( struct mulle__rbtree *a_tree,
                                                           void *a_key,
                                                           int (*a_comp)( void *, void *));


MULLE_C_NONNULL_FIRST_SECOND
struct mulle_rbnode    *_mulle__rbtree_find_next_node( struct mulle__rbtree *a_tree,
                                                       struct mulle_rbnode *a_node);

MULLE_C_NONNULL_FIRST_SECOND
struct mulle_rbnode    *_mulle__rbtree_find_previous_node( struct mulle__rbtree *a_tree,
                                                           struct mulle_rbnode  *a_node);

// returns -1 if node is already in there according to a_comp, otherwise 0
MULLE_C_NONNULL_FIRST_SECOND_THIRD
int    _mulle__rbtree_insert_node( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node,
                                   int (*a_comp)( void *, void *));

MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__rbtree_remove_node( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node);


MULLE_C_NONNULL_FIRST_SECOND
static inline
void   _mulle__rbtree_walk( struct mulle__rbtree *a_tree,
                            int (*callback)( struct mulle_rbnode *a_node, void *userinfo),
                            void *userinfo)
{
   struct mulle_rbnode  *a_node;

   a_node = _mulle__rbtree_find_leftmost_node( a_tree, _mulle__rbtree_get_root_node( a_tree));
   while( a_node != _mulle__rbtree_get_nil_node( a_tree))
   {
      if( ! (*callback)( a_node, userinfo))
         break;
      a_node = _mulle__rbtree_find_next_node( a_tree, a_node);
   }
}

#endif
