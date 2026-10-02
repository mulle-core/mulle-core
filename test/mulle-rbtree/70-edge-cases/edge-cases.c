//
//  edge-cases.c
//  mulle-rbtree
//
//  Tests for: duplicates, walk_dirty under deletes, marker option,
//  extra+release, and remove-missing-key (Bug 1 regression).
//
#include <mulle-rbtree/mulle-rbtree.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


// ================================================================
// 1) Duplicate handling with allow_duplicates
// ================================================================

static void  test_duplicates( void)
{
   struct mulle_rbtree          tree;
   struct mulle_rbtree_config   config;
   size_t                       count;
   void                         *item;
   int                          rval;

   printf( "=== Test: duplicates ===\n");

   // Without allow_duplicates: second insert of same key returns EEXIST
   config.comparison = (int (*)( void *, void *)) strcmp;
   config.dirty      = NULL;
   config.callback   = &mulle_container_valuecallback_copied_cstring;
   config.node_extra = 0;
   config.options    = 0;
   config.capacity   = 0;   // 0 = default bucket size; config must be fully set

   mulle_rbtree_init_with_config( &tree, &config, NULL);

   rval = mulle_rbtree_add( &tree, "alpha");
   assert( rval == 0);
   rval = mulle_rbtree_add( &tree, "alpha");
   assert( rval == EEXIST);

   count = _mulle__rbtree_get_count( &tree.base);
   printf( "no-dup: count=%zu (expect 1)\n", count);
   assert( count == 1);

   mulle_rbtree_done( &tree);

   // With allow_duplicates: inserting same key twice should give count 2
   config.options = mulle_rbtree_option_allow_duplicates;
   mulle_rbtree_init_with_config( &tree, &config, NULL);

   rval = mulle_rbtree_add( &tree, "alpha");
   assert( rval == 0);
   rval = mulle_rbtree_add( &tree, "alpha");
   assert( rval == 0);
   rval = mulle_rbtree_add( &tree, "alpha");
   assert( rval == 0);
   rval = mulle_rbtree_add( &tree, "bravo");
   assert( rval == 0);

   count = _mulle__rbtree_get_count( &tree.base);
   printf( "dup: count=%zu (expect 4)\n", count);
   assert( count == 4);

   // walk in order: should see alpha, alpha, alpha, bravo
   printf( "dup walk:");
   mulle_rbtree_for( &tree, item)
   {
      printf( " %s", (char *) item);
   }
   printf( "\n");

   // remove one "alpha" — should leave 3
   rval = mulle_rbtree_remove( &tree, "alpha");
   assert( rval == 0);
   count = _mulle__rbtree_get_count( &tree.base);
   printf( "dup after remove: count=%zu (expect 3)\n", count);
   assert( count == 3);

   mulle_rbtree_done( &tree);
   printf( "PASSED\n\n");
}


// ================================================================
// 2) walk_dirty under delete-heavy workload
// ================================================================

static int  dirty_call_count;

static void  dirty_callback( void *node, void *left, void *right)
{
   dirty_call_count++;
}


static void  test_walk_dirty_under_deletes( void)
{
   struct mulle_rbtree          tree;
   struct mulle_rbtree_config   config;
   char                         buf[ 64];
   unsigned int                 i;
   int                          rval;

   printf( "=== Test: walk_dirty under deletes ===\n");

   config.comparison = (int (*)( void *, void *)) strcmp;
   config.dirty      = dirty_callback;
   config.callback   = &mulle_container_valuecallback_copied_cstring;
   config.node_extra = 0;
   config.options    = mulle_rbtree_option_use_dirty;
   config.capacity   = 0;

   mulle_rbtree_init_with_config( &tree, &config, NULL);

   // Insert 50 nodes
   for( i = 0; i < 50; i++)
   {
      sprintf( buf, "key-%03u", i);
      rval = mulle_rbtree_add( &tree, buf);
      assert( rval == 0);
   }

   printf( "dirty calls after 50 inserts: %d\n", dirty_call_count);
   assert( dirty_call_count > 0);

   // Now remove half of them in a pattern
   dirty_call_count = 0;
   for( i = 0; i < 50; i += 2)
   {
      sprintf( buf, "key-%03u", i);
      rval = mulle_rbtree_remove( &tree, buf);
      assert( rval == 0);
   }

   printf( "dirty calls after 25 deletes: %d\n", dirty_call_count);
   assert( dirty_call_count > 0);

   // Verify remaining count
   {
      size_t count = _mulle__rbtree_get_count( &tree.base);
      printf( "remaining count: %zu (expect 25)\n", count);
      assert( count == 25);
   }

   // Verify tree is still in sorted order
   {
      void   *prev = NULL;
      void   *item;

      mulle_rbtree_for( &tree, item)
      {
         if( prev)
            assert( strcmp( (char *) prev, (char *) item) < 0);
         prev = item;
      }
   }

   mulle_rbtree_done( &tree);
   printf( "PASSED\n\n");
}


// ================================================================
// 3) Marker option — nodes touched by rotations get marked
// ================================================================

static void  test_marker( void)
{
   struct mulle__rbtree         tree;
   struct mulle_rbnode          *node;
   struct mulle_rbnode          *nil_node;
   unsigned int                 i;
   int                          marked_count;
   // Insert in sorted order — worst case that guarantees rotations
   char                         *keys[] = { "a", "b", "c", "d", "e", "f", "g",
                                            "h", "i", "j" };

   printf( "=== Test: marker ===\n");

   _mulle__rbtree_init_with_options( &tree, 0,
                                     mulle_rbtree_option_use_marker,
                                     NULL);

   nil_node = _mulle__rbtree_get_nil_node( &tree);

   // Insert nodes in sorted order — rotations must occur in a RB tree
   for( i = 0; i < 10; i++)
   {
      node = _mulle__rbtree_new_node( &tree, keys[ i]);
      _mulle__rbtree_insert_node( &tree, node,
                                  (int (*)( void *, void *)) strcmp);
   }

   // Count marked nodes
   marked_count = 0;
   node = _mulle__rbtree_find_leftmost_node( &tree,
                                             _mulle__rbtree_get_root_node( &tree));
   while( node != nil_node)
   {
      if( _mulle_rbnode_is_marked( node))
         marked_count++;
      node = _mulle__rbtree_next_node( &tree, node);
   }

   printf( "marked nodes after inserts: %d (of 10)\n", marked_count);
   // With sorted insertion of 10 nodes, rotations are guaranteed
   assert( marked_count > 0);

   // Clear markers
   node = _mulle__rbtree_find_leftmost_node( &tree,
                                             _mulle__rbtree_get_root_node( &tree));
   while( node != nil_node)
   {
      _mulle_rbnode_clear_marker( node);
      node = _mulle__rbtree_next_node( &tree, node);
   }

   // Remove nodes — may mark some nodes if rotations occur
   node = _mulle__rbtree_find_node( &tree, "b",
                                    (int (*)( void *, void *)) strcmp);
   assert( node != nil_node);
   _mulle__rbtree_remove_node( &tree, node);

   node = _mulle__rbtree_find_node( &tree, "d",
                                    (int (*)( void *, void *)) strcmp);
   assert( node != nil_node);
   _mulle__rbtree_remove_node( &tree, node);

   marked_count = 0;
   node = _mulle__rbtree_find_leftmost_node( &tree,
                                             _mulle__rbtree_get_root_node( &tree));
   while( node != nil_node)
   {
      if( _mulle_rbnode_is_marked( node))
         marked_count++;
      node = _mulle__rbtree_next_node( &tree, node);
   }

   printf( "marked nodes after removes: %d\n", marked_count);
   // Verify tree still has correct count (10 - 2 = 8)
   {
      size_t count = _mulle__rbtree_get_count( &tree);
      printf( "count after removes: %zu (expect 8)\n", count);
      assert( count == 8);
   }

   _mulle__rbtree_done( &tree);
   printf( "PASSED\n\n");
}


// ================================================================
// 4) Extra mode + release callback
// ================================================================

struct extra_value
{
   int   key;
   int   data;
};


static int   compare_extra_value( void *a, void *b)
{
   struct extra_value  *va = (struct extra_value *) a;
   struct extra_value  *vb = (struct extra_value *) b;

   if( va->key < vb->key)
      return( -1);
   if( va->key > vb->key)
      return( 1);
   return( 0);
}


static int  release_count;

static void  extra_release( const struct mulle_container_valuecallback *callback,
                            void *value,
                            struct mulle_allocator *allocator)
{
   release_count++;
}


static void  *extra_retain( const struct mulle_container_valuecallback *callback,
                            void *value,
                            struct mulle_allocator *allocator)
{
   return( value);
}


static void  test_extra_with_release( void)
{
   struct mulle_rbtree          tree;
   struct mulle_rbtree_config   config;
   struct extra_value           values[ 5];
   struct extra_value           *found;
   struct extra_value           search;
   int                          i;
   int                          rval;

   printf( "=== Test: extra + release callback ===\n");

   struct mulle_container_valuecallback  cb = {
      .retain  = extra_retain,
      .release = extra_release,
   };

   config.comparison = compare_extra_value;
   config.dirty      = NULL;
   config.callback   = &cb;
   config.node_extra = sizeof( struct extra_value);
   config.options    = mulle_rbtree_option_use_extra;
   config.capacity   = 0;

   mulle_rbtree_init_with_config( &tree, &config, NULL);
   release_count = 0;

   for( i = 0; i < 5; i++)
   {
      values[ i].key  = (i + 1) * 10;
      values[ i].data = (i + 1) * 100;
      rval = mulle_rbtree_add( &tree, &values[ i]);
      assert( rval == 0);
   }

   // Verify find works
   search.key = 30;
   found = mulle_rbtree_find( &tree, &search);
   assert( found != NULL);
   printf( "found key=%d data=%d\n", found->key, found->data);
   assert( found->key == 30);
   assert( found->data == 300);

   // Remove one — release should be called
   rval = mulle_rbtree_remove( &tree, &search);
   assert( rval == 0);
   printf( "release_count after remove: %d (expect 1)\n", release_count);
   assert( release_count == 1);

   // Verify it's gone
   found = mulle_rbtree_find( &tree, &search);
   assert( found == NULL);

   // Remove missing key — should return ENOENT, not crash
   search.key = 999;
   rval = mulle_rbtree_remove( &tree, &search);
   printf( "remove missing: rval=%d (expect ENOENT=%d)\n", rval, ENOENT);
   assert( rval == ENOENT);

   // Done — should release remaining 4
   mulle_rbtree_done( &tree);
   printf( "release_count after done: %d (expect 5)\n", release_count);
   assert( release_count == 5);

   printf( "PASSED\n\n");
}


// ================================================================
// 5) Remove missing key — regression test for Bug 1
// ================================================================

static void  test_remove_missing_key( void)
{
   struct mulle_rbtree   tree;
   int                   rval;
   size_t                count;

   printf( "=== Test: remove missing key (Bug 1 regression) ===\n");

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_copied_cstring,
                      NULL);

   mulle_rbtree_add( &tree, "alpha");
   mulle_rbtree_add( &tree, "bravo");
   mulle_rbtree_add( &tree, "charlie");

   count = _mulle__rbtree_get_count( &tree.base);
   printf( "count before: %zu (expect 3)\n", count);
   assert( count == 3);

   // Remove a key that doesn't exist
   rval = mulle_rbtree_remove( &tree, "zulu");
   printf( "remove(zulu): rval=%d (expect ENOENT=%d)\n", rval, ENOENT);
   assert( rval == ENOENT);

   // Tree must be unchanged
   count = _mulle__rbtree_get_count( &tree.base);
   printf( "count after: %zu (expect 3)\n", count);
   assert( count == 3);

   // Verify all original keys still findable
   assert( mulle_rbtree_find( &tree, "alpha") != NULL);
   assert( mulle_rbtree_find( &tree, "bravo") != NULL);
   assert( mulle_rbtree_find( &tree, "charlie") != NULL);

   mulle_rbtree_done( &tree);
   printf( "PASSED\n\n");
}


// ================================================================
// main
// ================================================================

int  main( void)
{
   test_duplicates();
   test_walk_dirty_under_deletes();
   test_marker();
   test_extra_with_release();
   test_remove_missing_key();

   printf( "All edge-case tests passed.\n");
   return( 0);
}
