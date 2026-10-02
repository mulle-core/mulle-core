//
// The removes below are the test's own setup, so they must not live inside
// assert(): under NDEBUG the whole expression is compiled away, the removes
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
   int                   rval;

   printf( "=== Test: mulle_rbtree_is_empty ===\n");

   mulle_rbtree_init( &tree,
                      string_compare,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);

   // a freshly initialized tree is empty
   printf( "empty after init: %d (expect 1)\n", mulle_rbtree_is_empty( &tree));
   assert( mulle_rbtree_is_empty( &tree));

   // adding values makes it non-empty
   mulle_rbtree_add( &tree, "bravo");
   mulle_rbtree_add( &tree, "alpha");
   mulle_rbtree_add( &tree, "charlie");
   printf( "empty after 3 adds: %d (expect 0)\n", mulle_rbtree_is_empty( &tree));
   assert( ! mulle_rbtree_is_empty( &tree));

   // removing a missing key leaves it non-empty
   rval = mulle_rbtree_remove( &tree, "zulu");
   assert( rval == ENOENT);
   printf( "empty after failed remove: %d (expect 0)\n", mulle_rbtree_is_empty( &tree));
   assert( ! mulle_rbtree_is_empty( &tree));

   // removing all values empties it again
   rval = mulle_rbtree_remove( &tree, "alpha");
   assert( rval == 0);
   rval = mulle_rbtree_remove( &tree, "bravo");
   assert( rval == 0);
   rval = mulle_rbtree_remove( &tree, "charlie");
   assert( rval == 0);
   printf( "empty after removing all: %d (expect 1)\n", mulle_rbtree_is_empty( &tree));
   assert( mulle_rbtree_is_empty( &tree));

   // a NULL tree is treated as empty (graceful)
   printf( "empty for NULL tree: %d (expect 1)\n", mulle_rbtree_is_empty( NULL));
   assert( mulle_rbtree_is_empty( NULL));

   mulle_rbtree_done( &tree);
   printf( "PASSED\n");

   return( 0);
}
