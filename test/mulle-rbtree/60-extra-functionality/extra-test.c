//
//  extra-test.c
//  mulle-rbtree
//
//  Test for mulle_rbtree_option_use_extra functionality
//  This test verifies that when using the extra option, the comparison
//  function receives the address of the extra area instead of the payload.
//

#include "include.h"
#include <stdio.h>
#include <string.h>

struct test_node
{
   int   key;
   char  extra_data[32];
};

static int  compare_extra( void *a, void *b)
{
   struct test_node *node_a = (struct test_node *) a;
   struct test_node *node_b = (struct test_node *) b;
   
   printf( "Comparing: %d vs %d (using extra area)\n",
                 node_a->key, node_b->key);
   
   if( node_a->key < node_b->key)
      return( -1);
   if( node_a->key > node_b->key)
      return( 1);
   return( 0);
}

static void  test_extra_functionality( void)
{
   struct mulle_rbtree           tree;
   struct mulle_rbtree_config    config;
   struct test_node              nodes[5];
   struct test_node              *found;
   int                           i;
   
   printf( "Testing mulle_rbtree_option_use_extra functionality\n");
   printf( "==================================================\n");
   
   // Initialize config for extra functionality
   config.comparison = compare_extra;
   config.dirty      = NULL;
   config.callback   = NULL;
   config.node_extra = sizeof( struct test_node) - sizeof( void *); // Extra bytes after payload pointer
   config.options    = mulle_rbtree_option_use_extra;
   config.capacity   = 0;   // 0 = default bucket size; config must be fully set
   
   mulle_rbtree_init_with_config( &tree, &config, NULL);
   
   // Create test nodes with keys and extra data
   for( i = 0; i < 5; i++)
   {
      nodes[i].key = (i + 1) * 10;
      snprintf( nodes[i].extra_data, sizeof( nodes[i].extra_data), "Node%d_extra", i + 1);
   }
   
   printf( "\nInserting nodes:\n");
   for( i = 0; i < 5; i++)
   {
      printf( "  Inserting node with key %d, extra data: %s\n",
                    nodes[i].key, nodes[i].extra_data);
      mulle_rbtree_add( &tree, &nodes[i]);
   }
   
   printf( "\nTree count: %zu\n", _mulle__rbtree_get_count( &tree.base));
   
   // Test finding nodes
   printf( "\nTesting find operations:\n");
   for( i = 0; i < 5; i++)
   {
      struct test_node search_key;
      search_key.key = (i + 1) * 10;
      
      found = mulle_rbtree_find( &tree, &search_key);
      if( found)
      {
         printf( "  Found node with key %d, extra data: %s\n",
                       found->key, found->extra_data);
      }
      else
      {
         printf( "  ERROR: Could not find node with key %d\n", search_key.key);
      }
   }
   
   // Test finding non-existent node
   {
      struct test_node search_key;
      search_key.key = 999;
      
      found = mulle_rbtree_find( &tree, &search_key);
      if( ! found)
      {
         printf( "  Correctly did not find non-existent node with key 999\n");
      }
      else
      {
         printf( "  ERROR: Found unexpected node with key 999\n");
      }
   }
   
   // Test enumeration
   printf( "\nEnumerating tree in order:\n");
   {
      struct test_node *node;
      int count = 0;
      
      mulle_rbtree_for( &tree, node)
      {
         printf( "  Node %d: key=%d, extra_data=%s\n",
                       ++count, node->key, node->extra_data);
      }
   }
   
   // Test reverse enumeration
   printf( "\nEnumerating tree in reverse order:\n");
   {
      struct test_node *node;
      int count = 0;
      
      mulle_rbtree_reversefor( &tree, node)
      {
         printf( "  Node %d: key=%d, extra_data=%s\n",
                       ++count, node->key, node->extra_data);
      }
   }
   
   // Test removal
   printf( "\nTesting removal:\n");
   {
      struct test_node search_key;
      search_key.key = 30;
      
      if( mulle_rbtree_remove( &tree, &search_key) == 0)
      {
         printf( "  Successfully removed node with key 30\n");
      }
      else
      {
         printf( "  ERROR: Failed to remove node with key 30\n");
      }
      
      // Verify removal
      found = mulle_rbtree_find( &tree, &search_key);
      if( ! found)
      {
         printf( "  Confirmed removal: node with key 30 not found\n");
      }
      else
      {
         printf( "  ERROR: Node with key 30 still found after removal\n");
      }
   }
   
   printf( "\nFinal tree count: %zu\n", _mulle__rbtree_get_count( &tree.base));
   
   mulle_rbtree_done( &tree);
   printf( "\nTest completed successfully!\n");
}

int  main( void)
{
   test_extra_functionality();
   return( 0);
}