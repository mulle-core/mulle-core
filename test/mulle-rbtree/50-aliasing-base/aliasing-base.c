//
//  aliasing-base.c
//
//  Direct regression test for the nested-member layout of mulle-rbtree:
//
//      struct mulle_rbtree  ->  struct mulle__rbtree  base
//
//  `base` is a real member at offset 0, so `&tree` and `&tree.base` denote
//  the same object.
//
//  The failure this guards against: the old layout repeated the internal
//  fields by macro, which made `struct mulle_rbtree` and `struct
//  mulle__rbtree` distinct types of identical layout. Accessing an object of
//  one through a pointer to the other is a strict-aliasing violation (C11
//  6.5p7) that GCC exploits at -O2/-O3: the wrappers used to do
//
//      _mulle__rbtree_find_node( (struct mulle__rbtree *) a_tree, ...)
//
//  and the enumerator stored `(struct mulle__rbtree *) a_tree`, so a store
//  through one type could be assumed not to alias a load through the other.
//  The wrappers now hand out `&tree->base` instead of casting.
//
//  The hard guarantee is the layout: the `offsetof` assert and the
//  `&tree->base` accesses below stop compiling if the member is removed or
//  moved, i.e. if the macro-repeated prefix comes back. The interleaved
//  store/load and the live enumeration then exercise the access pattern the
//  old cast got wrong, under whatever optimization level the build uses.
//
#ifdef NDEBUG
# undef NDEBUG
#endif
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <mulle-rbtree/mulle-rbtree.h>


_Static_assert( offsetof( struct mulle_rbtree, base) == 0,
                "mulle_rbtree.base must be the first member");
_Static_assert( sizeof( struct mulle_rbtree) >= sizeof( struct mulle__rbtree),
                "mulle_rbtree must be at least as large as its base");


// The innermost view: `struct mulle__rbtree *`. `static inline` so the
// optimizer reasons about the aliasing within one TU.
static inline unsigned int   _rbtree_get_options( struct mulle__rbtree *tree)
{
   return( tree->_options);
}

static inline void   _rbtree_set_options( struct mulle__rbtree *tree,
                                          unsigned int options)
{
   tree->_options = options;
}

static inline struct mulle_rbnode   *_rbtree_get_root( struct mulle__rbtree *tree)
{
   return( tree->_root);
}


static void   test_identity( void)
{
   struct mulle_rbtree   tree;
   struct mulle_rbtree   *p;

   memset( &tree, 0, sizeof( tree));
   p = &tree;

   assert( (void *) p == (void *) &p->base);
   assert( sizeof( *p) > sizeof( p->base));
}


//
// Store through the outer handle, read through the inner type (and back).
// The interleaved stores matter: the last store must be observed by the
// differently-typed load, which is what a macro-repeated prefix would forbid.
//
static void   test_store_load_nested( void)
{
   struct mulle_rbtree   tree;

   memset( &tree, 0, sizeof( tree));

   // outer store, inner store, outer load
   tree.base._options = mulle_rbtree_option_allow_duplicates;
   _rbtree_set_options( &tree.base, mulle_rbtree_option_use_dirty);
   assert( tree.base._options == mulle_rbtree_option_use_dirty);

   // inner store, outer store, inner load
   _rbtree_set_options( &tree.base, mulle_rbtree_option_allow_duplicates);
   tree.base._options = mulle_rbtree_option_use_dirty;
   assert( _rbtree_get_options( &tree.base) == mulle_rbtree_option_use_dirty);

   // same for the node pointers
   tree.base._root = (struct mulle_rbnode *) (uintptr_t) 0x10;
   _rbtree_set_options( &tree.base, mulle_rbtree_option_allow_duplicates);
   tree.base._root = (struct mulle_rbnode *) (uintptr_t) 0x30;
   assert( _rbtree_get_root( &tree.base) == (struct mulle_rbnode *) (uintptr_t) 0x30);
}


//
// `mulle_rbtree_is_empty` is `static inline` and computes root/nil through
// `&tree->base`. Fabricate the pointers through the outer handle and let the
// public accessor read them.
//
static void   test_public_accessor_through_base( void)
{
   struct mulle_rbtree   tree;

   memset( &tree, 0, sizeof( tree));

   // an empty tree has root == &_nil
   tree.base._root = &tree.base._nil;
   assert( mulle_rbtree_is_empty( &tree) == 1);

   tree.base._root = (struct mulle_rbnode *) (uintptr_t) 0x10;
   assert( mulle_rbtree_is_empty( &tree) == 0);
}


static int   string_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}


static int   walk_callback( void *value, void *userinfo)
{
   char   ***cursor = (char ***) userinfo;

   assert( *cursor);
   assert( ! strcmp( **cursor, (char *) value));
   ++*cursor;
   return( 1);
}


//
// Live tree. The build goes through the public API; the read goes through the
// nested base and through the public enumerator/find/walk, which all reach
// into `&tree->base`. A stale or hoisted load of the base state would show up
// as a wrong count, a missing value or a wrong order.
//
static void   test_live_tree( void)
{
   struct mulle_rbtree             tree;
   struct mulle_rbtreeenumerator   rover;
   char                            *expected[ 3 ] = { "1848", "Bochum", "VfL" };
   char                            **cursor;
   void                            *item;
   int                             count;

   mulle_rbtree_init( &tree,
                      string_compare,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);
   mulle_rbtree_add( &tree, "VfL");
   mulle_rbtree_add( &tree, "Bochum");
   mulle_rbtree_add( &tree, "1848");

   // the caller-owned count, read through the nested base, is 3
   assert( _mulle__rbtree_get_count( &tree.base) == 3);
   assert( ! mulle_rbtree_is_empty( &tree));

   // find and the public walk compute through &tree->base
   assert( mulle_rbtree_find( &tree, "Bochum") != NULL);
   assert( ! strcmp( "Bochum", (char *) mulle_rbtree_find( &tree, "Bochum")));
   assert( mulle_rbtree_find( &tree, "zulu") == NULL);

   cursor = expected;
   mulle_rbtree_walk( &tree, walk_callback, &cursor);
   assert( cursor == expected + 3);

   // the enumerator stores the nested base, not a pun of the outer tree
   rover = mulle_rbtree_enumerate( &tree);
   assert( rover._tree == &tree.base);

   count = 0;
   while( count < 8 && _mulle_rbtreeenumerator_next( &rover, &item))
   {
      assert( count < 3);
      assert( ! strcmp( expected[ count], (char *) item));
      ++count;
   }
   assert( count == 3);

   mulle_rbtreeenumerator_done( &rover);
   mulle_rbtree_done( &tree);
}


int   main( void)
{
   test_identity();
   test_store_load_nested();
   test_public_accessor_through_base();
   test_live_tree();

   return( 0);
}
