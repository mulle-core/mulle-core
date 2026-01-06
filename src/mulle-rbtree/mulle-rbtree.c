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
#include "mulle-rbtree.h"


static void   nop_dirty( void *a, void *b, void *c)
{
   MULLE_C_UNUSED( a);
   MULLE_C_UNUSED( b);
   MULLE_C_UNUSED( c);
}


void   _mulle_rbtree_init_with_config( struct mulle_rbtree *a_tree,
                                       struct mulle_rbtree_config *config,
                                       struct mulle_allocator *allocator)
{
   assert( a_tree);

   memset( a_tree, 0, sizeof( *a_tree));

   assert( config->comparison);

   allocator = allocator ? allocator : &mulle_default_allocator;
   _mulle__rbtree_init_with_options( (struct mulle__rbtree *) a_tree,
                                     config->node_extra,
                                     config->options,
                                     allocator);

   a_tree->callback   = config->callback
                        ? *config->callback
                        : mulle_container_valuecallback_nonowned_cstring;
   a_tree->comparison = config->comparison;
   a_tree->dirty      = config->dirty ? config->dirty : nop_dirty;
}


void   _mulle_rbtree_init( struct mulle_rbtree *a_tree,
                           int (*a_comp)( void *, void *),
                           struct mulle_container_valuecallback *callback,
                           struct mulle_allocator *allocator)
{
   struct mulle_rbtree_config   config = {
                                            .comparison = a_comp,
                                            .callback   = callback
                                         };

   _mulle_rbtree_init_with_config( a_tree, &config, allocator);
}



void   _mulle_rbtree_done( struct mulle_rbtree *a_tree)
{
   void                     *value;
   struct mulle_allocator   *allocator;
   struct mulle_rbnode      *node;
   struct mulle_rbnode      *root_node;
   struct mulle_rbnode      *nil_node;
   struct mulle__rbtree     *rb__tree = (struct mulle__rbtree *) a_tree;

   assert( a_tree);
   
   if( _mulle_container_valuecallback_releases( &a_tree->callback))
   {
      allocator = _mulle_rbtree_get_allocator( a_tree);
      nil_node  = _mulle__rbtree_get_nil_node( rb__tree);
      root_node = _mulle__rbtree_get_root_node( rb__tree);
      node      = _mulle__rbtree_find_leftmost_node( rb__tree, root_node);

      while( node != nil_node)
      {
         value = _mulle__rbtree_get_node_value( rb__tree, node);
         (*a_tree->callback.release)( &a_tree->callback,
                                      value,
                                      allocator);
         node = _mulle__rbtree_next_node( rb__tree, node);
      }
   }

   _mulle__rbtree_done( rb__tree);
}


int   _mulle_rbtree_add( struct mulle_rbtree *a_tree, void *value)
{
   struct mulle_rbnode      *node;
   struct mulle_allocator   *allocator;
   int                      rval;
   struct mulle__rbtree     *rb__tree = (struct mulle__rbtree *) a_tree;

   assert( a_tree);

   allocator = _mulle_rbtree_get_allocator( a_tree);
   // need payload for comparison before insertion, don't change it
   // afterwards
   value = (*a_tree->callback.retain)( &a_tree->callback,
                                       value,
                                       allocator);
   node  = _mulle__rbtree_new_node( rb__tree, value);

   rval  = _mulle__rbtree_insert_node( rb__tree,
                                       node,
                                       a_tree->comparison);
   if( rval)
   {
      (*a_tree->callback.release)( &a_tree->callback,
                                   value,
                                   allocator);
      _mulle__rbtree_free_node( rb__tree, node);
   }

   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle_rbtree_walk_dirty( a_tree);

   return( rval);
}


void   _mulle_rbtree_remove_node( struct mulle_rbtree *a_tree,
                                  struct mulle_rbnode *node)
{
   struct mulle_allocator   *allocator;
   void                     *value;
   struct mulle__rbtree     *rb__tree = (struct mulle__rbtree *) a_tree;

   assert( a_tree);

   value     = _mulle__rbtree_get_node_value( rb__tree, node);
   allocator = _mulle_rbtree_get_allocator( a_tree);
   (*a_tree->callback.release)( &a_tree->callback, value, allocator);
   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle__rbtree_mark_node_as_dirty( rb__tree, node); // this will marker parents
   _mulle__rbtree_remove_node( rb__tree, node);

   if( a_tree->_options & mulle_rbtree_option_use_dirty)
      _mulle_rbtree_walk_dirty( a_tree);
}


int   _mulle_rbtree_remove( struct mulle_rbtree *a_tree, void *a_key)
{
   struct mulle_rbnode      *node;

   assert( a_tree);

   node = _mulle__rbtree_find_node( (struct mulle__rbtree *) a_tree,
                                    a_key,
                                    a_tree->comparison);
   if( ! node)
      return( ENOENT);

   _mulle_rbtree_remove_node( a_tree, node);
   return( 0);
}


