//
//  mulle_rb.c
//  MulleXLSReader
//
//  Created by Nat! on 24.10.14.
//  Copyright (c) 2014 Mulle kybernetiK. All rights reserved.
//

#include "mulle--rbtree.h"

#include <errno.h>

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

void   _mulle__rbtree_init( struct mulle__rbtree *a_tree,
                            struct mulle_allocator *allocator)
{
   memset( a_tree, 0, sizeof( struct mulle__rbtree));

   a_tree->_root = _mulle__rbtree_get_nil_node( a_tree);

   _mulle__rbtree_init_node( a_tree, _mulle__rbtree_get_nil_node( a_tree), NULL);
   _mulle_storage_init( &a_tree->_nodes,
                        sizeof( struct mulle_rbnode),
                        alignof( struct mulle_rbnode),
                        32,
                        allocator);
}


// the tree won't be cleaned off _nodes though, you have to do this
void   _mulle__rbtree_done( struct mulle__rbtree *a_tree)
{
   assert( a_tree);
   _mulle_storage_done( &a_tree->_nodes);
#ifdef DEBUG
   mulle_memset_uint32( a_tree, 0xDEADDEAD,sizeof( struct mulle__rbtree));
#endif
}

struct mulle_rbnode  *_mulle__rbtree_new_node( struct mulle__rbtree *a_tree,
                                               void *payload)
{
   struct mulle_rbnode   *node;

   node = _mulle_storage_malloc( &a_tree->_nodes);
   _mulle__rbtree_init_node( a_tree, node, payload);
   return( node);
}



struct mulle_rbnode   *
   _mulle__rbtree_find_next_node( struct mulle__rbtree *a_tree,
                                  struct mulle_rbnode *a_node)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *t;

   if( a_node->_right != _mulle__rbtree_get_nil_node( a_tree))
      return( _mulle__rbtree_find_leftmost_node( a_tree, a_node->_right));
   
   t = a_node;
   p = a_node->_parent;
   
   while( p != _mulle__rbtree_get_nil_node( a_tree) && t == p->_right)
   {
      t = p;
      p = p->_parent;
   }
   return( p);
}


struct mulle_rbnode    *
   _mulle__rbtree_find_previous_node( struct mulle__rbtree *a_tree,
                                      struct mulle_rbnode *a_node)
{
   struct mulle_rbnode   *p;
   struct mulle_rbnode   *t;

   if( a_node->_left != _mulle__rbtree_get_nil_node( a_tree))
      return( _mulle__rbtree_find_rightmost_node( a_tree, a_node->_left));
			
   t = a_node;
   p = a_node->_parent;
   
   while( p != _mulle__rbtree_get_nil_node( a_tree) && t == p->_left)
   {
      t = p;
      p = p->_parent;
   }
   return( p);
}


/*
 * Find a match if it exists.  Otherwise, find the next greater node, if one
 * exists.
 */
struct mulle_rbnode    *
   _mulle__rbtree_find_node_with_equal_or_greater_payload( struct mulle__rbtree *a_tree,
                                                         void *a_key,
                                                         int (*a_comp)( void *, void *))
{
   int       t;
   struct mulle_rbnode    *p;
   struct mulle_rbnode    *n;
   
   p = a_tree->_root;
   while( p != _mulle__rbtree_get_nil_node( a_tree) && (t = (a_comp)((a_key), p->payload)) != 0)
   {
      if( t < 0)
      {
         if( p->_left == _mulle__rbtree_get_nil_node( a_tree))
            break;
         
         p = p->_left;
      }
      else
      {
         if( p->_right == _mulle__rbtree_get_nil_node( a_tree))
         {
            n = p;
            p = p->_parent;
            while (p != _mulle__rbtree_get_nil_node( a_tree) && n == p->_right)
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


MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_left_rotate_node( struct mulle__rbtree *a_tree,
                                              struct mulle_rbnode *p)
{
   struct mulle_rbnode    *t;
   
   t         = p->_right;
   p->_right = t->_left;
   if( t->_left != _mulle__rbtree_get_nil_node( a_tree))
      t->_left->_parent = p;
   
   t->_parent = p->_parent;
   if( p->_parent == _mulle__rbtree_get_nil_node( a_tree))
      a_tree->_root = t;
   else
      if( p == p->_parent->_left)
         p->_parent->_left = t;
      else
         p->_parent->_right = t;
   
   t->_left   = p;
   p->_parent = t;
}


MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_right_rotate_node( struct mulle__rbtree *a_tree,
                                                struct mulle_rbnode *p)
{
   struct mulle_rbnode    *t;
   
   t       = p->_left;
   p->_left = t->_right;
   
   if( t->_right != _mulle__rbtree_get_nil_node( a_tree))
      t->_right->_parent = p;
   
   t->_parent = p->_parent;
   if( p->_parent == _mulle__rbtree_get_nil_node( a_tree))
      a_tree->_root = t;
   else
      if( p == p->_parent->_right)
         p->_parent->_right = t;
   else
      p->_parent->_left = t;
   
   t->_right  = p;
   p->_parent = t;
}


/* a_node is always the first argument to a_comp. */
int   _mulle__rbtree_insert_node( struct mulle__rbtree *a_tree,
                                  struct mulle_rbnode *a_node,
                                  int (*a_comp)( void *, void *))
{
   struct mulle_rbnode    *x;
   struct mulle_rbnode    *y;
   int       c;
   
   x = _mulle__rbtree_get_nil_node( a_tree);
   y = a_tree->_root;
   c = 0;
   
   while( y != _mulle__rbtree_get_nil_node( a_tree))
   {
      x = y;
      c = (*a_comp)(a_node->payload, y->payload);
      if( ! c && ! a_tree->allow_duplicates)
         return( EEXIST);
      if( c < 0)
         y = y->_left;
      else
         y = y->_right;
   }
   
   a_node->_parent = x;
   
   if( x == _mulle__rbtree_get_nil_node( a_tree))
      a_tree->_root = a_node;
   else
      if( c < 0)
         x->_left = a_node;
      else
         x->_right = a_node;
   
   /* Fix up. */
   x        = a_node;
   x->_color = mulle__rbtree_red;
   
   while( x != a_tree->_root && x->_parent->_color)
   {
      y = x->_parent;
      if( y == y->_parent->_left)
      {
         y = y->_parent->_right;
         if( y->_color)
         {
            x->_parent->_color          = mulle__rbtree_black;
            y->_color                   = mulle__rbtree_black;
            x->_parent->_parent->_color = mulle__rbtree_red;
            x = x->_parent->_parent;
         }
         else
         {
            if( x == x->_parent->_right)
            {
               x = x->_parent;
               _mulle__rbtree_left_rotate_node( a_tree, x);
            }
            x->_parent->_color          = mulle__rbtree_black;
            x->_parent->_parent->_color = mulle__rbtree_red;
            x                           = x->_parent->_parent;

            _mulle__rbtree_right_rotate_node( a_tree, x);
         }
      }
      else
      {
         y = y->_parent->_left;
         if( y->_color)
         {
            x->_parent->_color          = mulle__rbtree_black;
            y->_color                   = mulle__rbtree_black;
            x->_parent->_parent->_color = mulle__rbtree_red;
            x                           = x->_parent->_parent;
         }
         else
         {
            if( x == x->_parent->_left)
            {
               x = x->_parent;
               _mulle__rbtree_right_rotate_node(a_tree, x);
            }
            x->_parent->_color          = mulle__rbtree_black;
            x->_parent->_parent->_color = mulle__rbtree_red;
            x                           = x->_parent->_parent;

            _mulle__rbtree_left_rotate_node( a_tree, x);
         }
      }
   }
   a_tree->_root->_color = mulle__rbtree_black;
   return( 0);
}


void   _mulle__rbtree_remove_node( struct mulle__rbtree *a_tree,
                                   struct mulle_rbnode *a_node)
{
   int                    fixup;
   struct mulle_rbnode    *x, *y;

   if( a_node->_left == _mulle__rbtree_get_nil_node( a_tree) ||
       a_node->_right == _mulle__rbtree_get_nil_node( a_tree))
   {
      y = a_node;
   }
   else
      y = _mulle__rbtree_find_next_node( a_tree, a_node);
   
   if( y->_left != _mulle__rbtree_get_nil_node( a_tree))
      x = y->_left;
   else
      x = y->_right;
   
   x->_parent = y->_parent;
   if( y->_parent == _mulle__rbtree_get_nil_node( a_tree))
      a_tree->_root = x;
   else
      if( y == y->_parent->_left)
         y->_parent->_left = x;
      else
         y->_parent->_right = x;
   
   if( y->_color == mulle__rbtree_black)
      fixup = mulle__rbtree_red;
   else
      fixup = mulle__rbtree_black;
   
   if( y != a_node)
   {
      /* Splice y into a_node's location. */
      y->_parent = a_node->_parent;
      y->_left   = a_node->_left;
      y->_right  = a_node->_right;
      y->_color  = a_node->_color;
      
      if( y->_parent != _mulle__rbtree_get_nil_node( a_tree))
      {
         if (y->_parent->_left == a_node)
            y->_parent->_left = y;
         else
            y->_parent->_right = y;
      }
      else
         a_tree->_root = y;
      
      y->_right->_parent = y;
      y->_left->_parent  = y;
   }
   
   // more like cleanup
   //   _mulle__rbtree_init_node( a_tree, a_node);

   // push on reuse list
   _mulle_storage_free( &a_tree->_nodes, a_node);
   
   if( fixup)
   {
      /* Fix up. */
      struct mulle_rbnode  *v, *w;
      
      while( x != a_tree->_root && x->_color == mulle__rbtree_black)
      {
         if( x == x->_parent->_left)
         {
            w = x->_parent->_right;
            if( w->_color)
            {
               w->_color = mulle__rbtree_black;
               v         = x->_parent;
               v->_color = mulle__rbtree_red;
               
               _mulle__rbtree_left_rotate_node(a_tree, v);
               
               w         = x->_parent->_right;
            }
            
            if( w->_left->_color == mulle__rbtree_black &&
                w->_right ->_color == mulle__rbtree_black)
            {
               w->_color = mulle__rbtree_red;
               x         = x->_parent;
            }
            else
            {
               if( w->_right->_color == mulle__rbtree_black)
               {
                  w->_left->_color = mulle__rbtree_black;
                  w->_color        = mulle__rbtree_red;
                  
                  _mulle__rbtree_right_rotate_node( a_tree, w);
                  
                  w                = x->_parent->_right;
               }
               
               w->_color          = x->_parent->_color;
               x->_parent->_color = mulle__rbtree_black;
               w->_right->_color  = mulle__rbtree_black;
               v                  = x->_parent;
               
               _mulle__rbtree_left_rotate_node(a_tree, v);
               break;
            }
         }
         else
         {
            w = x->_parent->_left;
            if( w->_color)
            {
               w->_color = mulle__rbtree_black;
               v        = x->_parent;
               v->_color = mulle__rbtree_red;
               
               _mulle__rbtree_right_rotate_node(a_tree, v);
               
               w = x->_parent->_left;
            }
            
            if( w->_right->_color == mulle__rbtree_black &&
                w->_left->_color == mulle__rbtree_black)
            {
               w->_color = mulle__rbtree_red;
               x = x->_parent;
            }
            else
            {
               if( w->_left->_color == mulle__rbtree_black)
               {
                  w->_right->_color = mulle__rbtree_black;
                  w->_color         = mulle__rbtree_red;
                  
                  _mulle__rbtree_left_rotate_node(a_tree, w);
                  
                  w = x->_parent	->_left;
               }
               
               w->_color           = x->_parent->_color;
               x->_parent->_color  = mulle__rbtree_black;
               w->_left->_color    = mulle__rbtree_black;
               
               v                   = x->_parent;
               _mulle__rbtree_right_rotate_node(a_tree, v);
               break;
            }
         }
      }							
      x->_color = mulle__rbtree_black;
   }								
}
