//
//  count.c
//  mulle-rbtree
//
//  Documented contract (working as expected, not a missing feature):
//  mulle_rbtree has NO mulle_rbtree_get_count(). It is a direct-mutation
//  container -- the caller inserts and removes every value, so the caller owns
//  the count. The live node count already exists O(1) in the node pool as
//  _mulle_storage_get_count( &tree.base._nodes), but there is deliberately no
//  public accessor: exposing it would promote _nodes, an implementation
//  detail, to a public guarantee. Rank/select is a different structure.
//
//  _mulle__rbtree_get_count() is a deliberate O(n) recursive walk. This test
//  pins that it agrees with the live node count through add and remove.
//
//
// The mutations below are the test's own setup, so they must not live inside
// assert(): under NDEBUG the whole expression is compiled away, the adds
// never happen and the test quietly exercises nothing. Keep the assertions
// live so the test means the same thing in every build (the convention the
// mulle-container tests use).
//
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#include <mulle-rbtree/mulle-rbtree.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>


static int  string_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}


int  main( void)
{
   struct mulle_rbtree   tree;
   char                  values[ 100][ 8];
   size_t                walk_count;
   size_t                storage_count;
   int                   i;
   int                   rval;

   printf( "=== Test: count is caller-owned, the walk is O(n) ===\n");

   mulle_rbtree_init( &tree,
                      string_compare,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);

   for( i = 0; i < 100; i++)
   {
      sprintf( values[ i], "%d", i);
      rval = mulle_rbtree_add( &tree, values[ i]);
      assert( rval == 0);
   }

   storage_count = _mulle_storage_get_count( &tree.base._nodes);
   walk_count    = _mulle__rbtree_get_count( &tree.base);
   printf( "after 100 adds: walk %zu, storage %zu (expect both 100)\n",
           walk_count, storage_count);
   assert( storage_count == 100);
   assert( walk_count == storage_count);

   for( i = 0; i < 40; i++)
   {
      rval = mulle_rbtree_remove( &tree, values[ i]);
      assert( rval == 0);
   }

   storage_count = _mulle_storage_get_count( &tree.base._nodes);
   walk_count    = _mulle__rbtree_get_count( &tree.base);
   printf( "after removing 40: walk %zu, storage %zu (expect both 60)\n",
           walk_count, storage_count);
   assert( storage_count == 60);
   assert( walk_count == storage_count);

   for( i = 200; i < 210; i++)
   {
      sprintf( values[ i - 200], "%d", i);
      rval = mulle_rbtree_add( &tree, values[ i - 200]);
      assert( rval == 0);
   }

   storage_count = _mulle_storage_get_count( &tree.base._nodes);
   walk_count    = _mulle__rbtree_get_count( &tree.base);
   printf( "after adding 10: walk %zu, storage %zu (expect both 70)\n",
           walk_count, storage_count);
   assert( storage_count == 70);
   assert( walk_count == storage_count);

   mulle_rbtree_done( &tree);
   printf( "PASSED\n");

   return( 0);
}
