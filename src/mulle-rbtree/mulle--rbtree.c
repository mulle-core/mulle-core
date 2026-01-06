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

// if in read only memory, cant be changed, convenient catcher of bugs
// ... or not our algorithm actually clobbers ._parent but its intended...
// const struct mulle_rbnode    mulle_rbnode_nil =
// {
//    ._color = mulle__rbtree_black
// };
//

void   _mulle__rbtree_init_with_options( struct mulle__rbtree *a_tree,
                                         size_t node_extra_size,
                                         unsigned int options,
                                         struct mulle_allocator *allocator)
{
   struct mulle_rbnode   dummy;
   size_t                extra;

   memset( a_tree, 0, sizeof( struct mulle__rbtree));

   a_tree->_nil._left   = &a_tree->_nil;
   a_tree->_nil._right  = &a_tree->_nil;
   a_tree->_nil._parent = &a_tree->_nil;

   a_tree->_root        = &a_tree->_nil;
   a_tree->_options     = options;

   // so possibly extend node_extra for alignment
   // assert( extra && (options & mulle_rbtree_option_use_extra))
   // we'll let it slide...
   extra = &((char *) _mulle_rbnode_get_extra( &dummy))[ node_extra_size] - (char *) &dummy;

   _mulle_storage_init( &a_tree->_nodes,
                        sizeof( struct mulle_rbnode) + extra,
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
   mulle_memset_uint32( a_tree, 0xDEADDEAD, sizeof( struct mulle__rbtree));
#endif
}


struct mulle_rbnode  *_mulle__rbtree_new_node( struct mulle__rbtree *a_tree,
                                               void *value)
{
   struct mulle_rbnode   *node;

   node = _mulle_storage_malloc( &a_tree->_nodes);
   _mulle__rbtree_init_node( a_tree, node, value);
   return( node);
}


MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node( struct mulle__rbtree *a_tree,
                             void *a_key,
                             int (*a_comp)( void *, void *))
{
   if( a_tree->_options & mulle_rbtree_option_use_extra)
      return( _mulle__rbtree_find_node_with_extra( a_tree, a_key, a_comp));
   return( _mulle__rbtree_find_node_with_payload( a_tree, a_key, a_comp));
}


MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_THIRD
struct mulle_rbnode    *
   _mulle__rbtree_find_node_equal_or_greater( struct mulle__rbtree *a_tree,
                                              void *a_key,
                                              int (*a_comp)( void *, void *))
{
   if( a_tree->_options & mulle_rbtree_option_use_extra)
      return( _mulle__rbtree_find_node_with_equal_or_greater_extra( a_tree, a_key, a_comp));
   return( _mulle__rbtree_find_node_with_equal_or_greater_payload( a_tree, a_key, a_comp));
}


static inline void  mulle_rb_node_set_left( struct mulle_rbnode *p,
                                            struct mulle_rbnode *o,
                                            struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   if( o != nil_node)
      o->_parent = p;
   p->_left = o;
}


static inline void  mulle_rb_node_set_right( struct mulle_rbnode *p,
                                             struct mulle_rbnode *o,
                                             struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   if( o != nil_node)
      o->_parent = p;
   p->_right = o;
}


#ifndef NDEBUG
static inline void   _assert_dirty_flag_propagation( struct mulle_rbnode *node,
                                                     struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *parent;

   assert( _mulle__rbtree_get_nil_node( a_tree) != node);
   if( _mulle__rbtree_get_root_node( a_tree) == node)
      return;

   parent = node->_parent;
   assert( _mulle_rbnode_is_dirty( parent));
   _assert_dirty_flag_propagation( parent, a_tree);
}


static inline void   assert_dirty_flag_propagation( struct mulle_rbnode *node,
                                                    struct mulle__rbtree *a_tree)
{
   if( _mulle_rbnode_is_dirty( node))
      _assert_dirty_flag_propagation( node, a_tree);
}
#else
#define assert_dirty_flag_propagation( node, a_tree)  do{}while(0)
#endif


static inline void   mulle_rb_node_reparent( struct mulle_rbnode *p,
                                             struct mulle_rbnode *t,
                                             struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   // move 't' above 'p'
   t->_parent = p->_parent;
   _mulle__rbtree_set_node_marked( a_tree,  t);

   // If p was the root, then t becomes the new root of the tree.
   // Otherwise, reconnect t into p’s parent — replacing p in the tree.
   // Handles whether p was a left or right child.
   if( p->_parent == nil_node)
      a_tree->_root = t;
   else
   {
      if( p == p->_parent->_right)
         p->_parent->_right = t;
      else
         p->_parent->_left = t;
      _mulle__rbtree_set_node_marked( a_tree,  p->_parent);
   }
}


//
// we propagate the dirty flag up
//
void   _mulle__rbtree_mark_node_as_dirty( struct mulle__rbtree *a_tree,
                                          struct mulle_rbnode *node)
{
   struct mulle_rbnode   *nil_node;

   assert( a_tree->_options & mulle_rbtree_option_use_dirty);

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   while( node != nil_node)
   {
      // brute force to the top
      _mulle_rbnode_set_dirty( node);
      node = _mulle_rbnode_get_parent( node);
   }
}


// T as right child of P moves to the top. P becomes left child.
// Former left child of T becomes right child of P. The number of nodes on
// the left side is now
//
//          P                                 T
//         / \                               / \     gcc is fing stupid
//        A  [T]              ->           <P>  C
//           / \                           / \       gcc is fing stupid
//         <B>   C                        A  [B]
//
// P is the node being rotated.    T has taken P's place.
// T = P->_right                   P is now the left child of T.
// A is P->_left                   B, which was T->_left, is now P->_right.
// B = T->_left                    All parent pointers are updated accordingly.
// C = T->_right
//
MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_left_rotate_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *p)
{
   struct mulle_rbnode   *t;

   _mulle__rbtree_set_node_marked( a_tree,  p);

   // t is the node to the right
   t   = p->_right;
   _mulle__rbtree_set_node_marked( a_tree,  t);

   assert( p != _mulle__rbtree_get_nil_node( a_tree));
   assert( t != _mulle__rbtree_get_nil_node( a_tree));

   // Move t's left subtree into p's right subtree (we still have t..)
   mulle_rb_node_set_right( p, t->_left, a_tree);
   _mulle__rbtree_set_node_marked( a_tree,  t->_left);

   // move 't' above 'p'
   mulle_rb_node_reparent( p, t, a_tree);

   // p becomes the left child of t
   // t becomes the new parent of p
   t->_left   = p;
   p->_parent = t;
   _mulle__rbtree_set_node_marked( a_tree,  p->_parent);

   // MEMO: need to redo 'p' and then 't' (and propagate up)
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
   {
      _mulle__rbtree_mark_node_as_dirty( a_tree, p);

      assert_dirty_flag_propagation( p, a_tree);
      assert_dirty_flag_propagation( t, a_tree);
   }
}


//         P                                   T
//        / \                                 / \         gcc is fing stupid
//       T   C                 ->            A   P
//      / \                                     / \       gcc is fing stupid
//     A   B                                   B   C
//
// P is the node being rotated.     T has taken P's place.
// T = P->_left                     P is now the right child of T.
// C is P->_right                   B, which was T->_right, is now P->_left.
// A = T->_left                     All parent pointers are updated accordingly.
// B = T->_right
//
// just the inverse of left rotate
MULLE_C_NONNULL_FIRST_SECOND
static void   _mulle__rbtree_right_rotate_node( struct mulle__rbtree *a_tree,
                                                struct mulle_rbnode *p)
{
   struct mulle_rbnode   *t;
   
   _mulle__rbtree_set_node_marked( a_tree,  p);

   t   = p->_left;
   _mulle__rbtree_set_node_marked( a_tree,  t);

   assert( p != _mulle__rbtree_get_nil_node( a_tree));
   assert( t != _mulle__rbtree_get_nil_node( a_tree));

   mulle_rb_node_set_left( p, t->_right, a_tree);
   _mulle__rbtree_set_node_marked( a_tree,  t->_right);
   
   mulle_rb_node_reparent( p, t, a_tree);
   
   t->_right  = p;
   p->_parent = t;
   _mulle__rbtree_set_node_marked( a_tree,  p->_parent);

   // MEMO: need to redo 'p' and then 't' (and propagate up)
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
   {
      _mulle__rbtree_mark_node_as_dirty( a_tree, p);

      assert_dirty_flag_propagation( p, a_tree);
      assert_dirty_flag_propagation( t, a_tree);
   }
}





//
// Fixup differences:
//
// | Aspect              | Insert Fixup                   | Delete Fixup                               |
// | ------------------- | ------------------------------ | ------------------------------------------ |
// | Triggered by        | Red-red violation              | Black-height violation                     |
// | Node color involved | Always starts with red node    | Usually starts after removing black node   |
// | Key violation       | Parent and child both red      | Double-black / missing black height        |
// | Fixup method        | Recolor and/or 1–2 rotations   | Recolor, 0–3 rotations, upward propagation |
// | Complexity          | Simpler, localized             | More complex, can propagate up to root     |
// | Symmetry            | Symmetric for left/right cases | Also symmetric, but more intricate         |
//
// this code works with 'x' being the nil_node, because we are free to clobber
// the parent pointer. It's hacky...
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void   _mulle__rbtree_black_black_fixup( struct mulle__rbtree *a_tree,
                                         struct mulle_rbnode *x)
{
   struct mulle_rbnode   *w;

   while( x != a_tree->_root && _mulle_rbnode_is_black( x))
   {
      if( x == x->_parent->_left)
      {
         w = x->_parent->_right;

         if( _mulle_rbnode_is_red( w))
         {
            /* sibling red -> recolor + rotate parent left */
            _mulle_rbnode_set_black( w);
            _mulle_rbnode_set_red( x->_parent);
            _mulle__rbtree_left_rotate_node( a_tree, x->_parent);
            w = x->_parent->_right;
         }

         if( _mulle_rbnode_is_black( w->_left) &&
             _mulle_rbnode_is_black( w->_right))
         {
            /* sibling and both children black -> recolor sibling, move up */
            _mulle_rbnode_set_red( w);
            x = x->_parent;
         }
         else
         {
            if( _mulle_rbnode_is_black( w->_right))
            {
               /* sibling.right black, sibling.left red -> rotate sibling right */
               _mulle_rbnode_set_black( w->_left);
               _mulle_rbnode_set_red( w);
               _mulle__rbtree_right_rotate_node( a_tree, w);
               w = x->_parent->_right;
            }

            /* sibling.right is red -> copy parent color to sibling, fix and finish */
            {
               int parent_color = _mulle_rbnode_get_color( x->_parent);
               _mulle_rbnode_set_color( w, parent_color);
               _mulle_rbnode_set_black( x->_parent);
               _mulle_rbnode_set_black( w->_right);
               _mulle__rbtree_left_rotate_node( a_tree, x->_parent);
               x = a_tree->_root;
            }
         }
      }
      else
      {
         /* mirror: x is right child */
         w = x->_parent->_left;

         if( _mulle_rbnode_is_red( w))
         {
            /* sibling red -> recolor + rotate parent right */
            _mulle_rbnode_set_black( w);
            _mulle_rbnode_set_red( x->_parent);
            _mulle__rbtree_right_rotate_node( a_tree, x->_parent);
            w = x->_parent->_left;
         }

         if( _mulle_rbnode_is_black( w->_right) &&
             _mulle_rbnode_is_black( w->_left))
         {
            /* sibling and both children black -> recolor sibling, move up */
            _mulle_rbnode_set_red( w);
            x = x->_parent;
         }
         else
         {
            if( _mulle_rbnode_is_black( w->_left))
            {
               /* sibling.left black -> rotate sibling left */
               _mulle_rbnode_set_black( w->_right);
               _mulle_rbnode_set_red( w);
               _mulle__rbtree_left_rotate_node( a_tree, w);
               w = x->_parent->_left;
            }

            /* sibling.left is red -> copy parent color to sibling, fix and finish */
            {
               int parent_color = _mulle_rbnode_get_color( x->_parent);
               _mulle_rbnode_set_color( w, parent_color);
               _mulle_rbnode_set_black( x->_parent);
               _mulle_rbnode_set_black( w->_left);
               _mulle__rbtree_right_rotate_node( a_tree, x->_parent);
               x = a_tree->_root;
            }
         }
      }
      assert_dirty_flag_propagation( x, a_tree);
      assert_dirty_flag_propagation( w, a_tree);
   }

   /* Important: set x black (fixes cases where loop exited because x became red). */
   _mulle_rbnode_set_black( x);

   /* Also ensure the root is black using the tree-aware setter (keeps your asserts). */
   _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
}


//
// Here is a tree where we want to delete 2
//
//           10(B)
//          /     \                 gcc is fing stupid
//       5(R)     15(R)
//      /   \     /   \             gcc is fing stupid
//   2(B)  7(B) 12(B) 17(B)
//
// Root (10) is black (B).
// Its children (5 and 15) are red (R).
// Their children (2,7,12,17) are black (B).
//
// This satisfies:
//     No two reds in a row,
//     Same number of black nodes on all root-to-leaf paths (3 blacks),
//     Root is black.
//
// All paths:
//    10(B) -> 5(R)  -> 2(B)  -> nil_node(B)
//    10(B) -> 5(R)  -> 7(B)  -> nil_node(B)
//    10(B) -> 15(R) -> 12(B) -> nil_node(B)
//    10(B) -> 15(R) -> 17(B) -> nil_node(B)
//
// Step 2: Remove node 2(B) → Before fixup
//           10(B)
//          /     \              gcc is fing stupid
//       5(R)     15(R)
//         \      /   \          gcc is fing stupid
//         7(B)  12(B) 17(B)
//
// Node 2 (black) removed, replaced with nil_node (black leaf).
// Path through left side lost a black node → black-height imbalance.
//
// All paths:
//    10(B) -> 5(R) -> nil_node(B)             → only 2 black nodes
//    10(B) -> 5(R) -> 7(B) -> nil_node(B)
//    10(B) -> 15(R) -> 12(B) -> nil_node(B)
//    10(B) -> 15(R) -> 17(B) -> nil_node(B)
//
// We recolor:
//           10(B)
//          /     \              gcc is fing stupid
//       5(B)     15(R)
//         \      /   \          gcc is fing stupid
//        7(R)  12(B) 17(B)
//
// All paths:
//    10(B) → 5(B)
//    10(B) → 5(B) → 7(R)
//    10(B) → 15(R) → 12(B)
//    10(B) → 15(R) → 17(B)
//
// -------------
//
// Here is a tree where we want to delete node 5
//
//           10(B)
//          /     \              gcc is fing stupid
//       5(R)     15(R)
//      /   \     /   \          gcc is fing stupid
//   2(B)  7(B) 12(B) 17(B)
//
//
//           10(B)
//          /     \              gcc is fing stupid
//       7(R)     15(R)
//      /         /   \          gcc is fing stupid
//   2(B)      12(B) 17(B)
//
//
//           10(B)
//          /     \              gcc is fing stupid
//       7(B)     15(R)
//      /         /   \          gcc is fing stupid
//   2(R)      12(B) 17(B)
//
MULLE__RBTREE_GLOBAL
MULLE_C_NONNULL_FIRST_SECOND
void _mulle__rbtree_remove_node(struct mulle__rbtree *a_tree,
                                struct mulle_rbnode *a_node)
{
   int                   fixup;      // Flag to check if we need to fix the tree's balance after removal
   struct mulle_rbnode   *x, *y;     // Temporary pointers: y is the node we'll actually remove, x is what replaces it
   struct mulle_rbnode   *nil_node;  // A special "empty" node that acts like a leaf (end of branch)

   // Get the special empty node used in this tree
   nil_node = _mulle__rbtree_get_nil_node(a_tree);

   // Make sure we're not trying to remove the empty node itself
   assert( a_node != nil_node);

   _mulle__rbtree_set_node_marked( a_tree,  a_node);
   // Save the original children of the node we're deleting, before we change anything
   struct mulle_rbnode *orig_left  = a_node->_left;
   struct mulle_rbnode *orig_right = a_node->_right;
   struct mulle_rbnode *y_original_parent = nil_node;  // default to nil_node

   _mulle__rbtree_set_node_marked( a_tree, orig_left);
   _mulle__rbtree_set_node_marked( a_tree, orig_right);

   // Decide which node to actually remove:
   // If the node has 0 or 1 real child, we can remove it directly.
   // Otherwise, find the next node in sorted order (the "successor") to swap with it.
   // The successor is always easy to remove because it has at most one child.
   // (It's the leftmost leaf on the right branch)
   //
   // Example: Deleting a_node            Becomes: y (successor) takes its place
   //          /    \                              /      \    gcc is fing stupid
   //        ...   ...                           ...      ...
   //
   if( a_node->_left == nil_node || a_node->_right == nil_node)
      y = a_node;
   else
   {
      y = _mulle__rbtree_next_node( a_tree, a_node);
      _mulle__rbtree_set_node_marked( a_tree,  y);
      y_original_parent = y->_parent;  // capture here before any changes
   }

   // x is the child of y (or the empty node if y has no children).
   // x will move up to take y's spot after removal.
   //
   // Example: y with one child          After: x moves up
   //          / \                              / \         gcc is fing stupid
   //        x   empty                       empty   empty
   //
   if( y->_left != nil_node)
      x = y->_left;
   else
      x = y->_right;

   _mulle__rbtree_set_node_marked( a_tree,  x);

   // Connect x directly to y's parent, effectively cutting y out of the tree.
   //
   // Example: Parent of y               After removal:
   //          /      \                         /      \    gcc is fing stupid
   //         y      ...                       x      ...
   //
   // Note: Even if x is the empty node, updating its parent is okay and sometimes needed for fixes.
   x->_parent = y->_parent;
   _mulle__rbtree_set_node_marked( a_tree,  x->_parent);

   if( y->_parent == nil_node)
   {
      a_tree->_root = x;  // If y was the root, now x is the new root
   }
   else
   {
      // Update the parent's pointer to point to x instead of y
      if( y == y->_parent->_left)
         y->_parent->_left = x;
      else
         y->_parent->_right = x;
   }
   _mulle__rbtree_set_node_marked( a_tree,  y->_parent);

   // Check if y was black (a color in red-black trees). If so, removal might unbalance the tree,
   // and we'll need to fix it later.
   fixup = _mulle_rbnode_is_black( y);

   // If we found a successor (y) that's different from the original node,
   // swap y into the original node's position in the tree.
   // y takes over the original node's children and color.
   //
   // Example: Original a_node           Becomes: y in its place
   //          /    \                           / \    gcc is fing stupid
   //         L      R                         L   R
   //
   if( y != a_node)
   {
      y->_parent = a_node->_parent;  // y gets the original parent's pointer
      y->_left   = orig_left;        // y adopts the original left child

      // Handle the right child carefully
      if( y != orig_right)
      {
         y->_right = orig_right;  // Adopt the original right child
      }
      else
      {
         // If y was already the right child, it keeps its own right subtree
         // (no need to change y->_right)
         x->_parent = y;  // Special fix: Ensure x points to the new y position
         _mulle__rbtree_set_node_marked( a_tree,  x->_parent);
      }

      // Copy the original node's color to y
      _mulle__rbtree_set_node_color(a_tree, y, _mulle_rbnode_get_color(a_node));

      // Update the parent's pointer to now point to y
      if( y->_parent != nil_node)
      {
         if( y->_parent->_left == a_node)
            y->_parent->_left = y;
         else
            y->_parent->_right = y;
         _mulle__rbtree_set_node_marked( a_tree,  y->_parent);
      }
      else
         a_tree->_root = y;  // If original was root, y is now root

      // Update the children's parent pointers to point back to y
      // (Harmless even if they're empty nodes)
      y->_right->_parent = y;
      y->_left->_parent  = y;

      _mulle__rbtree_set_node_marked( a_tree, y->_right);
      _mulle__rbtree_set_node_marked( a_tree, y->_left);
   }

   // If the tree uses "dirty" flags for tracking changes (like for recalculating sizes),
   // mark the affected parts as needing update.

#ifdef USE_CHATGPT_FIX
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
   {
      // The original parent of a_node saw a child change if a_node wasn't root.
      if( a_node->_parent != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, a_node->_parent);

      // If we swapped in successor y, y's own parent also lost a child (y -> x),
      // and must be marked dirty even when x == nil_node.
      if( y != a_node && y->_parent != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, y->_parent);

      // Mark y (now carrying a_node's children) so its new subtree gets recomputed.
      if( y != a_node && y != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, y);

      // If x is a real node, mark it too to propagate up that side.
      if( x != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, x);
   }
#else
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
   {
      // Mark the original parent's branch as changed
      if( a_node->_parent != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, a_node->_parent);

      // If we swapped in y, mark it too
      if( y != a_node && y != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, y);

      if( y != a_node && x == nil_node && y_original_parent != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, y_original_parent);

      // Mark the replacement x if it's real
      if( x != nil_node)
         _mulle__rbtree_mark_node_as_dirty( a_tree, x);
   }
#endif

   // Actually free the memory of the original node we wanted to delete
   _mulle__rbtree_free_node( a_tree, a_node);

   // If we removed a black node, the tree might be unbalanced now.
   // Red-black trees require the same number of black nodes on every path from root to leaf.
   //
   // Simple cases:
   // 1. Removed a red node? No problem, tree is still balanced.
   // 2. Removed a black node? This creates a "missing black" on that path, which we need to fix.
   //
   // The fix involves recoloring nodes and rotating branches to spread out the colors evenly.
   // x tracks where the "missing black" is, and we adjust around it.
   if( fixup)
   {
      _mulle__rbtree_black_black_fixup( a_tree, x);
   }

   assert_dirty_flag_propagation( x, a_tree);
   assert_dirty_flag_propagation( orig_left, a_tree);
   assert_dirty_flag_propagation( orig_right, a_tree);
}


void   _mulle__rbtree_red_red_fixup( struct mulle__rbtree *a_tree,
                                     struct mulle_rbnode *x)
{
   struct mulle_rbnode   *y;

   // Fixup: while parent is red, we need to maintain red-black properties
   while( x != a_tree->_root && _mulle_rbnode_is_red( x->_parent))
   {
      // Case: parent is a left child
      if( x->_parent == x->_parent->_parent->_left)
      {
         y = x->_parent->_parent->_right;  // uncle

         // Case 1: Uncle is red – recolor
         //
         //        G                             G
         //       / \                           / \         gcc is fing stupid
         //      P   U     ->                  B   B
         //     /                             /
         //    x                             R
         //
         // G = grandparent, P = parent, U = uncle
         // Recolor P and U to black, G to red, and move up to G
         if( _mulle_rbnode_is_red( y))
         {
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            _mulle__rbtree_set_node_black( a_tree, y);

            x = _mulle_rbnode_get_grandparent( x);
         }
         else
         {
            // Case 2: Uncle is black, node is right child – rotate left
            //
            //       G                           G
            //      / \                         / \         gcc is fing stupid
            //     P   U      -->              x   U
            //      \                         /
            //       x                       P
            //
            if( x == x->_parent->_right)
            {
               x = x->_parent;
               _mulle__rbtree_left_rotate_node( a_tree, x);
            }

            // Case 3: Uncle is black, node is left child – rotate right
            //
            //         G                          P
            //        / \       -->              / \        gcc is fing stupid
            //       P   U                      x   G
            //      /                                \      gcc is fing stupid
            //     x                                  U
            //
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            x = _mulle_rbnode_get_grandparent( x);

            _mulle__rbtree_right_rotate_node( a_tree, x);
         }
      }
      else
      {
         // Mirror case: parent is right child of grandparent
         y = x->_parent->_parent->_left;  // uncle

         // Case 1: Uncle is red – recolor
         //
         //         G                           G
         //        / \                         / \       gcc is fing stupid
         //       U   P     ->                B   B
         //            \                           \     gcc is fing stupid
         //             x                           R
         if( _mulle_rbnode_is_red( y))
         {
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            _mulle__rbtree_set_node_black( a_tree, y);

            x = _mulle_rbnode_get_grandparent( x);
         }
         else
         {
            // Case 2: Uncle is black, node is left child – rotate right
            //
            //         G                           G
            //        / \                         / \    gcc is fing stupid
            //       U   P        -->            U   x
            //          /                             \  gcc is fing stupid
            //         x                               P
            if( x == x->_parent->_left)
            {
               x = x->_parent;
               _mulle__rbtree_right_rotate_node( a_tree, x);
            }

            // Case 3: Uncle is black, node is right child – rotate left
            //
            //         G                             P
            //        / \                           / \     gcc is fing stupid
            //       U   P         -->             G   x
            //            \                       /
            //             x                     U
            _mulle__rbtree_set_node_black( a_tree, x->_parent);
            _mulle__rbtree_set_node_red( a_tree, x->_parent->_parent);
            x = _mulle_rbnode_get_grandparent( x);

            _mulle__rbtree_left_rotate_node( a_tree, x);
         }
      }
   }

   // fix this up
   _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
}


static void   _mulle__rbtree_attach_node( struct mulle__rbtree *a_tree,
                                         struct mulle_rbnode *a_node,
                                         struct mulle_rbnode *parent,
                                         int direction)
{
   struct mulle_rbnode   *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil_node);

   // must be singular unattached note
   assert( a_node->_parent == nil_node);
   assert( a_node->_left == nil_node);
   assert( a_node->_right == nil_node);
   assert( _mulle_rbnode_is_red( a_node));

   // Attach the new node as a child of parent
   a_node->_parent = parent;

   if( parent == nil_node)
   {
      a_tree->_root = a_node;
      _mulle__rbtree_set_node_black( a_tree, a_tree->_root);
   }
   else
   {
      assert( a_tree->_root != nil_node);
      if( direction < 0)
      {
         assert( parent->_left == nil_node);
         parent->_left = a_node;
      }
      else
      {
         assert( parent->_right == nil_node);
         parent->_right = a_node;
      }

      if( _mulle_rbnode_is_red( parent))
      {
         // Case A: parent is red, node is red -> red-red violation
         // black height not affected
         _mulle__rbtree_red_red_fixup( a_tree, a_node);
      }
   }

   assert( a_tree->_root != nil_node);

   // Mark node and ancestors as dirty after
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle__rbtree_mark_node_as_dirty( a_tree, a_node);
}



/* a_node is always the first argument to a_comp. */
int _mulle__rbtree_insert_node( struct mulle__rbtree *a_tree,
                                struct mulle_rbnode *a_node,
                                int (*a_comp)( void *, void *))
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   int                   c;
   struct mulle_rbnode  *nil_node;
   void                 *a_value;
   void                 *y_value;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil_node);

   x = nil_node;
   y = a_tree->_root;
   c = 0;

   // Traverse the tree to find the appropriate insertion point.
   // 'y' is the current node being inspected.
   // 'x' trails behind 'y' and becomes the parent of the new node.
   if( y != nil_node)
   {
      a_value = _mulle__rbtree_get_node_value( a_tree, a_node);

      do
      {
         x       = y;
         y_value = _mulle__rbtree_get_node_value( a_tree, y);
         c       = (*a_comp)( a_value, y_value);
         if( ! c && ! (a_tree->_options & mulle_rbtree_option_allow_duplicates))
            return( EEXIST);  // Duplicate key not allowed

         if( c < 0)
            y = y->_left;
         else
            y = y->_right;
      }
      while( y != nil_node);
   }

   _mulle__rbtree_attach_node( a_tree, a_node, x, c);
   return( 0);
}



void   _mulle__rbtree_insert_node_before_node( struct mulle__rbtree *a_tree,
                                               struct mulle_rbnode *a_node,
                                               struct mulle_rbnode *successor)
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   struct mulle_rbnode  *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil_node);
   assert( a_node != successor);
   assert( successor != nil_node);

   // go successor left and then right until nil_node
   x = successor;
   y = x->_left;
   if( y == nil_node)
   {
      _mulle__rbtree_attach_node( a_tree, a_node, x, -1);
      return;
   }

   do
   {
      x = y;
      y = y->_right;
   }
   while( y != nil_node);

   _mulle__rbtree_attach_node( a_tree, a_node, x, +1);
}


void  _mulle__rbtree_insert_node_after_node( struct mulle__rbtree *a_tree,
                                             struct mulle_rbnode *a_node,
                                             struct mulle_rbnode *predecessor)
{
   struct mulle_rbnode  *x;
   struct mulle_rbnode  *y;
   struct mulle_rbnode  *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);

   assert( a_node != nil_node);
   assert( a_node != predecessor);
   assert( predecessor != nil_node);

   // go predecessor right and then left until nil_node
   x = predecessor;
   y = x->_right;
   if( y == nil_node)
   {
      _mulle__rbtree_attach_node( a_tree, a_node, x, +1);
      return;
   }

   do
   {
      x = y;
      y = y->_left;
   }
   while( y != nil_node);

   _mulle__rbtree_attach_node( a_tree, a_node, x, -1);
}




#ifndef NDEBUG
static void   __mulle__rbtree_assert_clean( struct mulle__rbtree *a_tree,
                                            struct mulle_rbnode *a_node)
{
   struct mulle_rbnode  *nil_node;

   nil_node = _mulle__rbtree_get_nil_node( a_tree);
   if( a_node == nil_node)
      return;

   __mulle__rbtree_assert_clean( a_tree, a_node->_left);
   __mulle__rbtree_assert_clean( a_tree, a_node->_right);

   assert( ! _mulle_rbnode_is_dirty( a_node));
}


void   _mulle__rbtree_assert_clean( struct mulle__rbtree *a_tree)
{
   __mulle__rbtree_assert_clean( a_tree,
                                 _mulle__rbtree_get_root_node( a_tree));
}
#endif


void   _mulle__rbtree_walk_dirty( struct mulle__rbtree *a_tree,
                                  void (*callback)( void *node,
                                                    void *left,
                                                    void *right))
{
   struct mulle_rbnode   *root;

   root = _mulle__rbtree_get_root_node( a_tree);
   __mulle__rbtree_walk_dirty( a_tree, root, callback);

#ifndef NDEBUG
   _mulle__rbtree_assert_clean( a_tree);
#endif
}


size_t   _mulle__rbtree_get_count( struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *root;

   root = _mulle__rbtree_get_root_node( a_tree);
   return( __mulle__rbtree_walk_count( a_tree, root));
}


