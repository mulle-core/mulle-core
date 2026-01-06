//
//  mulle--rbtree-debug.c
//  mulle-rbtree-debug
//
//  Copyright (c) <|YEAR|> Nat! - Mulle kybernetiK.
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
#include "mulle-rbtree-debug.h"

#include <stdio.h>
#include <assert.h>


// Helper function to validate black-height property from a node to leaves
// Returns NULL if the black-height property is satisfied, otherwise an error message.
// Sets *black_height to the computed black height if valid.
static char *_mulle__rbtree_validate_black_height(struct mulle__rbtree *a_tree,
                                                  struct mulle_rbnode *node,
                                                  int *black_height)
{
   struct mulle_rbnode *nil;
   char *left_err, *right_err;
   int left_height, right_height;

   nil = _mulle__rbtree_get_nil_node(a_tree);

   // NIL nodes are black and have height 1
   if (node == nil)
   {
      *black_height = 1;
      return NULL;
   }

   // Recursively validate and compute black heights of left and right subtrees
   left_err = _mulle__rbtree_validate_black_height( a_tree, node->_left, &left_height);
   if( left_err != NULL)
      return left_err;

   right_err = _mulle__rbtree_validate_black_height( a_tree, node->_right, &right_height);
   if( right_err != NULL)
      return right_err;

   // Both subtrees must have the same black height
   if (left_height != right_height) {
      return "Black heights do not match between left and right subtrees";
   }

   // Add 1 if current node is black
   *black_height = left_height + (_mulle_rbnode_is_black(node) ? 1 : 0);
   return NULL;
}


// Helper function to validate red-black tree properties recursively
// Returns NULL if the node and its subtrees are valid, otherwise an error message.
static char   *_mulle__rbtree_validate_node( struct mulle__rbtree *a_tree,
                                             struct mulle_rbnode *node)
{
   struct mulle_rbnode *nil;
   char *err;

   nil = _mulle__rbtree_get_nil_node(a_tree);

   // NIL nodes are always valid
   if (node == nil) {
      return NULL;
   }

   // Rule 4: Red nodes can't have red children
   if (_mulle_rbnode_is_red(node)) {
      if (node->_left != nil && _mulle_rbnode_is_red(node->_left)) {
         return "Red-red violation: Red node has red left child";
      }
      if (node->_right != nil && _mulle_rbnode_is_red(node->_right)) {
         return "Red-red violation: Red node has red right child";
      }
   }

   // Validate parent-child consistency
   if (node->_left != nil && node->_left->_parent != node) {
      return "Parent-child inconsistency: Left child's parent pointer is incorrect";
   }
   if (node->_right != nil && node->_right->_parent != node) {
      return "Parent-child inconsistency: Right child's parent pointer is incorrect";
   }

   if( _mulle_rbnode_is_dirty( node) && ! _mulle_rbnode_is_dirty( node->_parent))
   {
      if( _mulle__rbtree_get_root_node( a_tree) != node)
         return "Dirty flag inconsistency: Parent is not marked dirty but child is";
   }

   // Recursively validate left and right subtrees
   err = _mulle__rbtree_validate_node( a_tree, node->_left);
   if (err != NULL) {
      return err;
   }
   err = _mulle__rbtree_validate_node( a_tree, node->_right);
   if (err != NULL) {
      return err;
   }

   return NULL;
}


/*
 * Validates that the red-black tree satisfies all five red-black tree properties:
 *
 * 1. Every node is red or black (enforced by data structure)
 * 2. Root is black
 * 3. All leaves (NIL) are black (enforced by implementation)
 * 4. Red nodes can't have red children
 * 5. All paths from root to leaves have the same number of black nodes
 *
 * Returns:
 *   NULL - Tree is valid
 *   error message string - Tree violates red-black properties (describes the violation)
 */
char   *mulle__rbtree_validate(struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode   *root;
   struct mulle_rbnode   *nil;
   char                  *err;
   int                   black_height;  // Unused, but needed for validation call

   if( ! a_tree)
      return( NULL);

   nil = _mulle__rbtree_get_nil_node(a_tree);
   root = _mulle__rbtree_get_root_node(a_tree);

   // Rule 3: NIL node must be black (sanity check)
   if (!_mulle_rbnode_is_black(nil)) {
      return "NIL node is not black";
   }

   // Empty tree is valid
   if (root == nil) {
      return NULL;
   }

   // Rule 2: Root must be black
   if (!_mulle_rbnode_is_black(root)) {
      return "Root node is not black";
   }

   // Validate node properties and red-red violations (Rule 4)
   err = _mulle__rbtree_validate_node(a_tree, root);
   if (err != NULL) {
      return err;
   }

   // Rule 5: Validate black-height consistency
   err = _mulle__rbtree_validate_black_height(a_tree, root, &black_height);
   if (err != NULL) {
      return err;
   }

   return NULL;  // Tree is valid
}


static void
_mulle__rbtree_node_dot_fprintf( FILE *fp,
                                 unsigned long node_id,
                                 struct mulle_rbnode *node,
                                 struct mulle__rbtree *a_tree,
                                 void (*print_value_fn)( FILE *fp, void *))
{
   struct mulle_rbnode *nil;
   unsigned long       left_id;
   unsigned long       right_id;
   char                *fillstyle;
   char                *fillcolor;
   char                *fontcolor;

   nil = _mulle__rbtree_get_nil_node( a_tree);

   if( node == nil)
      return;

   left_id  = (node_id << 1);
   right_id = (node_id << 1) + 1;

   // Node definition
   mulle_fprintf( fp, "  \"%ld\" [label=\"", node_id);
   if( print_value_fn)
      (*print_value_fn)( fp, _mulle__rbtree_get_node_value( a_tree, node));
   else
      mulle_fprintf( fp, "%ld", node_id);

   if( _mulle_rbnode_is_dirty( node))
      fillcolor = _mulle_rbnode_is_red( node) ? "lightcoral" : "darkgray";
   else
      fillcolor = _mulle_rbnode_is_red( node)  ? "red" : "black";

   fontcolor = "white";
   fillstyle = "filled";

   // in marker mode, we make "boring" unmarked nodes not filled
   if( a_tree->_options & mulle_rbtree_option_use_marker)
   {
      if( ! _mulle_rbnode_is_marked( node))
      {
         fillstyle = _mulle_rbnode_is_dirty( node) ? "dashed" : "dotted";
         fontcolor = fillcolor;
         fillcolor = "white";
      }
   }

    mulle_fprintf(fp, "\", style=\"%s,bold\", fillcolor=%s, fontcolor=%s];\n",
               fillstyle, fillcolor, fontcolor);


   // Left child
   if( node->_left != nil)
   {
      mulle_fprintf( fp, "  \"%ld\" -> \"%ld\" [label=\" L\"];\n", node_id, left_id);
      _mulle__rbtree_node_dot_fprintf( fp, left_id, node->_left, a_tree, print_value_fn);
   }
   else
   {
      // Show nil leaves for clarity
      mulle_fprintf( fp, "  nil_%ld [shape=point];\n", left_id);
      mulle_fprintf( fp, "  \"%ld\" -> nil_%ld [label=\" L\"];\n", node_id, left_id);
   }

   // Right child
   if (node->_right != nil)
   {
      mulle_fprintf( fp, "  \"%ld\" -> \"%ld\" [label=\" R\"];\n", node_id, right_id);
      _mulle__rbtree_node_dot_fprintf( fp, right_id, node->_right, a_tree, print_value_fn);
   }
   else
   {
      mulle_fprintf( fp, "  nil_%ld [shape=point];\n", right_id);
      mulle_fprintf( fp, "  \"%ld\" -> nil_%ld [label=\" R\"];\n", node_id, right_id);
   }
}




void  mulle__rbtree_node_dot_fprintf( FILE *fp,
                                      struct mulle__rbtree *a_tree,
                                      void (*print_value_fn)( FILE *fp, void *))
{

   fp = fp ? fp : stdout;

   mulle_fprintf( fp, "digraph RBTree {\n");
   mulle_fprintf( fp, "  rankdir=LR;\n");             // Top to Bottom layout nicer only for small trees
   mulle_fprintf( fp, "  node [shape=circle, fontcolor=white, fontsize=10];\n");

   if( a_tree)
      _mulle__rbtree_node_dot_fprintf( fp, 1, a_tree->_root, a_tree, print_value_fn);

   mulle_fprintf( fp, "}\n");
}




// Structure for tree printing
struct tree_line
{
   char *buffer;
   int   length;
};


struct tree_node_info
{
   struct mulle_rbnode   *node;
   int                   level;
   int                   x;
   int                   width;
};


// Calculate tree height
static int   _tree_height( struct mulle_rbnode *node,
                           struct mulle__rbtree *a_tree)
{
   struct mulle_rbnode *nil;
   int                  left_height;
   int                  right_height;

   nil = _mulle__rbtree_get_nil_node( a_tree);
   if( node == nil)
      return( 0);

   left_height  = _tree_height( node->_left, a_tree);
   right_height = _tree_height( node->_right, a_tree);

   return( 1 + (left_height > right_height ? left_height : right_height));
}



static char *   _node_label( struct mulle__rbtree *a_tree,
                             struct mulle_rbnode *node,
                             char *(*print_value_fn)( void *))
{
   void     *value;
   char     *s;
   size_t   len;
   char     *str;
   char     status_char;

   if( _mulle_rbnode_is_red( node))
   {
      if( _mulle_rbnode_is_dirty( node))
         status_char = 'R';
      else
         status_char = 'r';
   }
   else
   {
      if( _mulle_rbnode_is_dirty( node))
         status_char = 'B';
      else
         status_char = 'b';
   }

   value = _mulle__rbtree_get_node_value( a_tree, node);
   if( ! value)
      s = mulle_strdup( "NULL");
   else
      s = (*print_value_fn)( value);

   s = s ? s : mulle_strdup( "???");

   len   = snprintf( NULL, 0, "%s(%c)", s, status_char);
   str   = mulle_malloc( len + 1);
   snprintf( str, len + 1, "%s(%c)", s, status_char);
   mulle_free( s);

   return( str);
}



// Calculate width needed for a node's string representation
static int   _node_width( struct mulle__rbtree *a_tree,
                          struct mulle_rbnode *node,
                          char *(*print_value_fn)( void *))
{
   size_t   len;
   char     *s;

   s     = _node_label( a_tree, node, print_value_fn);
   len   = strlen( s);
   mulle_free( s);
   return( len);
}


// Build tree structure with positions (bottom-up approach)
static void   _build_tree_positions( struct mulle_rbnode *node,
                                     struct mulle__rbtree *a_tree,
                                     char *(*print_value_fn)( void *),
                                     struct tree_node_info *nodes,
                                     int *index,
                                     int level,
                                     int *x_offset)
{
   struct mulle_rbnode *nil;
//   int                  left_width = 0;
//   int                  right_width = 0;
   int                  node_width;
   int                  my_index;

   nil = _mulle__rbtree_get_nil_node( a_tree);
   if( node == nil)
      return;

   my_index = (*index)++;

   // Process left subtree
   if( node->_left != nil)
   {
      // left_offset = *x_offset;
      _build_tree_positions( node->_left, a_tree, print_value_fn, nodes, index,
                            level + 1, x_offset);
      // left_width = *x_offset - left_offset;
   }

   // Process right subtree
   if( node->_right != nil)
   {
      // right_offset = *x_offset;
      _build_tree_positions( node->_right, a_tree, print_value_fn, nodes, index,
                            level + 1, x_offset);
      // right_width = *x_offset - right_offset;
   }

   // Calculate position for this node
   node_width = _node_width( a_tree, node, print_value_fn);
   
   if( node->_left == nil && node->_right == nil)
   {
      // Leaf node
      nodes[ my_index].x = *x_offset + node_width / 2;
   }
   else if( node->_left != nil && node->_right != nil)
   {
      // Both children
      int left_child_x = nodes[ my_index + 1].x;
      int right_child_x = nodes[ *index - 1].x;
      nodes[ my_index].x = (left_child_x + right_child_x) / 2;
   }
   else if( node->_left != nil)
   {
      // Only left child
      nodes[ my_index].x = nodes[ my_index + 1].x + node_width / 2;
   }
   else
   {
      // Only right child
      nodes[ my_index].x = nodes[ *index - 1].x - node_width / 2;
   }

   // Ensure minimum spacing
   if( *x_offset < nodes[ my_index].x - node_width / 2)
      *x_offset = nodes[ my_index].x - node_width / 2;

   nodes[ my_index].node = node;
   nodes[ my_index].level = level;
   nodes[ my_index].width = node_width;

   *x_offset += node_width + 2;
}

// Print tree using bottom-up approach with malloced buffers
static void   _print_bottom_up_tree( FILE *fp,
                                     struct mulle__rbtree *a_tree,
                                     char *(*print_value_fn)( void *))
{
   char                    **connector_lines;
   char                    **lines;
   char                    *node_str;
   int                     height;
   int                     i;
   int                     j;
   int                     max_nodes;
   size_t                  max_width = 0;
   int                     node_count = 0;
   int                     parent_pos;
   int                     x_offset = 0;
   struct mulle_rbnode     *nil;
   struct mulle_rbnode     *root;
   struct tree_node_info   *nodes;


   nil  = _mulle__rbtree_get_nil_node( a_tree);
   root = _mulle__rbtree_get_root_node( a_tree);

   if( root == nil)
   {
      mulle_fprintf( fp, "(empty tree)\n");
      return;
   }

   height    = _tree_height( root, a_tree);
   max_nodes = (1 << height) - 1;  // 2^height - 1
   nodes     = mulle_calloc( max_nodes, sizeof( struct tree_node_info));

   // Build tree positions bottom-up
   _build_tree_positions( root, a_tree, print_value_fn, nodes, &node_count, 0, &x_offset);

   // Find maximum width
   for( i = 0; i < node_count; i++)
   {
      if( (size_t) (nodes[ i].x + nodes[ i].width / 2) > max_width)
         max_width = nodes[ i].x + nodes[ i].width / 2;
   }
   max_width += 2;

   // Allocate lines for output
   lines           = mulle_calloc( height, sizeof( char *));
   connector_lines = mulle_calloc( height - 1, sizeof( char *));
   
   for( i = 0; i < height; i++)
   {
      lines[ i] = mulle_calloc( max_width + 1, 1);
      memset( lines[ i], ' ', max_width);
      lines[ i][ max_width] = '\0';
      
      if( i < height - 1)
      {
         connector_lines[ i] = mulle_calloc( max_width + 1, 1);
         memset( connector_lines[ i], ' ', max_width);
         connector_lines[ i][ max_width] = '\0';
      }
   }

   // Fill lines with node strings
   for( i = 0; i < node_count; i++)
   {
      node_str = _node_label( a_tree, nodes[ i].node, print_value_fn);
      int start_pos = nodes[ i].x - strlen( node_str) / 2;
      
      if( start_pos >= 0 && start_pos + strlen( node_str) <= max_width)
      {
         memcpy( lines[ nodes[ i].level] + start_pos, node_str, strlen( node_str));
      }
      mulle_free( node_str);

      // Add connectors to parent
      if( nodes[ i].level > 0)
      {
         struct mulle_rbnode *parent = nodes[ i].node->_parent;
         int parent_idx = -1;
         
         for( j = 0; j < node_count; j++)
         {
            if( nodes[ j].node == parent)
            {
               parent_idx = j;
               break;
            }
         }
         
         if( parent_idx != -1)
         {
            parent_pos = nodes[ parent_idx].x;
            
            if( nodes[ i].node == parent->_left)
            {
               // Left child - draw '/'
               for( j = nodes[ i].x + 1; j < parent_pos; j++)
               {
                  if( j >= 0 && (size_t) j < max_width)
                     connector_lines[ nodes[ i].level - 1][ j] = '/';
               }
            }
            else
            {
               // Right child - draw '\'
               for( j = parent_pos + 1; j < nodes[ i].x; j++)
               {
                  if( j >= 0 && (size_t) j < max_width)
                     connector_lines[ nodes[ i].level - 1][ j] = '\\';
               }
            }
         }
      }
   }

   // Print the tree
   for( i = 0; i < height; i++)
   {
      // Trim trailing spaces
      int len = strlen( lines[ i]);
      while( len > 0 && lines[ i][ len - 1] == ' ')
         len--;
      lines[ i][ len] = '\0';
      
      if( len > 0)
         mulle_fprintf( fp, "%s\n", lines[ i]);

      if( i < height - 1)
      {
         len = strlen( connector_lines[ i]);
         while( len > 0 && connector_lines[ i][ len - 1] == ' ')
            len--;
         connector_lines[ i][ len] = '\0';
         
         if( len > 0)
            mulle_fprintf( fp, "%s\n", connector_lines[ i]);
      }
   }

   // Clean up
   for( i = 0; i < height; i++)
      mulle_free( lines[ i]);
   for( i = 0; i < height - 1; i++)
      mulle_free( connector_lines[ i]);
   mulle_free( lines);
   mulle_free( connector_lines);
   mulle_free( nodes);
}


void   mulle__rbtree_node_ascii_fprintf( FILE *fp,
                                         struct mulle__rbtree *a_tree,
                                         char *(*print_value_fn)( void *))
{
   struct mulle_rbnode *root;
   struct mulle_rbnode *nil;

   if( ! a_tree)
   {
      mulle_fprintf( fp, "NULL\n");
      return;
   }

   if( ! print_value_fn)
      abort();

   fp = fp ? fp : stdout;

   nil = _mulle__rbtree_get_nil_node( a_tree);
   root = _mulle__rbtree_get_root_node( a_tree);

   if( root == nil)
   {
      mulle_fprintf( fp, "(empty tree)\n");
      return;
   }

   mulle_fprintf( fp, "Red-Black Tree (r=red, b=black, R=red+dirty, B=black+dirty):\n");
   _print_bottom_up_tree( fp, a_tree, print_value_fn);
}
