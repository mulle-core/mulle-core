#include "include.h"

#include <stdio.h>
#include <string.h>


static char *print_int( void *value)
{
   char *s;
   
   s = mulle_malloc( 16);
   sprintf( s, "%d", (int)(intptr_t)value);
   return( s);
}


int main( void)
{
   struct mulle__rbtree  tree;
   struct mulle_rbnode   *node1;
   struct mulle_rbnode   *node2;
   struct mulle_rbnode   *node3;
   
   _mulle__rbtree_init( &tree, NULL);
   
   // Create a simple 3-node tree manually like the stress tests
   node1 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node1, (void*)5);
   node1->_color = mulle__rbtree_black;
   tree._root = node1;
   
   node2 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node2, (void*)3);
   node2->_color = mulle__rbtree_red;
   node2->_parent = node1;
   node1->_left = node2;
   
   node3 = _mulle_storage_malloc( &tree._nodes);
   _mulle__rbtree_init_node( &tree, node3, (void*)7);
   node3->_color = mulle__rbtree_red;
   node3->_parent = node1;
   node1->_right = node3;
   
   printf( "ASCII tree output:\n");
   mulle__rbtree_node_ascii_fprintf( stdout, &tree, print_int);
   
   _mulle__rbtree_done( &tree);
   
   return( 0);
}
