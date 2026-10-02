#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>
#include <stdalign.h>


//
// Test that freeing the last allocation reclaims space,
// while freeing non-last is a silent no-op.
//
int  main( int argc, char *argv[])
{
   struct mulle_arena       arena;
   struct mulle_allocator   *allocator;
   void                     *p;
   void                     *q;
   void                     *r;

   _mulle_arena_init( &arena, 256, NULL);
   allocator = _mulle_arena_get_allocator( &arena);

   // alloc + free last: should get same address back
   p = _mulle_allocator_malloc( allocator, 32);
   _mulle_allocator_free( allocator, p);

   q = _mulle_allocator_malloc( allocator, 32);
   if( q != p)
      abort();  // space was reclaimed, same address reused

   // alloc two things, free the first (not last): no reclaim
   p = _mulle_allocator_malloc( allocator, 32);
   q = _mulle_allocator_malloc( allocator, 32);
   _mulle_allocator_free( allocator, p);  // not last, no-op

   r = _mulle_allocator_malloc( allocator, 32);
   // r should NOT be p (p wasn't reclaimed)
   if( r == p)
      abort();

   // free the actual last, then alloc should reuse
   _mulle_allocator_free( allocator, r);
   q = _mulle_allocator_malloc( allocator, 32);
   if( q != r)
      abort();

   // double free of "last" should be no-op (last is NULL after first free)
   _mulle_allocator_free( allocator, q);
   _mulle_allocator_free( allocator, q);  // should not crash, q != NULL _last

   _mulle_arena_done( &arena);
   return( 0);
}
