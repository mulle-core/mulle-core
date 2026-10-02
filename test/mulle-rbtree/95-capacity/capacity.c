//
//  capacity.c
//  mulle-rbtree
//
//  Regression test for the node pool bucket size reachable through
//  mulle_rbtree_config.capacity:
//
//    * capacity 0 means "caller does not care" and must behave as the default
//      (MULLE__RBTREE_DEFAULT_CAPACITY == 32),
//    * capacity 1 is the smallest useful bucket and exercises the
//      capacity / 32 == 0 freed array,
//    * capacity 512 is a tree-sized bucket.
//
//  Every capacity must produce the same tree: 100 inserts, 100 enumerated,
//  values in order.
//
//
// The inserts below are the test's own setup, so they must not live inside
// assert(): under NDEBUG the whole expression is compiled away, the inserts
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


static int   string_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}


static void   run_capacity( unsigned int capacity,
                            unsigned int expected_bucket)
{
   struct mulle_rbtree_config   config = { 0 };
   struct mulle_rbtree          tree;
   char                         values[ 100][ 8];
   void                         *item;
   int                          i;
   int                          enumerated;
   int                          rval;

   config.comparison = string_compare;
   config.callback   = &mulle_container_valuecallback_nonowned_cstring;
   config.capacity   = capacity;

   mulle_rbtree_init_with_config( &tree, &config, NULL);

   // the configured number really is the node pool's bucket size
   assert( tree.base._nodes._structs.base._bucket_size == expected_bucket);

   for( i = 0; i < 100; i++)
   {
      sprintf( values[ i], "%03d", i);
      rval = mulle_rbtree_add( &tree, values[ i]);
      assert( rval == 0);
   }

   enumerated = 0;
   mulle_rbtree_for( &tree, item)
   {
      assert( strcmp( (char *) item, values[ enumerated]) == 0);
      enumerated++;
   }
   assert( enumerated == 100);

   printf( "capacity %4u: bucket %4u, 100 inserts, %d enumerated in order\n",
           capacity, expected_bucket, enumerated);

   mulle_rbtree_done( &tree);
}


int   main( void)
{
   printf( "=== Test: node pool bucket size through mulle_rbtree_config.capacity ===\n");

   // capacity / 32 == 0 for the freed array; mulle_structqueue raises a bucket
   // below 4 to 4, so the effective bucket here is 4
   run_capacity( 1, 4);

   // 0 -> MULLE__RBTREE_DEFAULT_CAPACITY (32)
   run_capacity( 0, MULLE__RBTREE_DEFAULT_CAPACITY);

   // 512 -> 512
   run_capacity( 512, 512);

   printf( "PASSED\n");
   return( 0);
}
