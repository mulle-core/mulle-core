#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>
#include <stdalign.h>


//
// Test that mulle_arena works as a mulle_allocator via the vtable
//
int  main( int argc, char *argv[])
{
   struct mulle_arena       arena;
   struct mulle_allocator   *allocator;
   void                     *p;
   void                     *q;
   int                      *ip;
   unsigned int             i;

   _mulle_arena_init( &arena, 256, NULL);

   // get the allocator interface
   allocator = _mulle_arena_get_allocator( &arena);

   // malloc via realloc(NULL, size) — should be aligned to max_align_t
   p = _mulle_allocator_malloc( allocator, 64);
   if( ! p)
      abort();
   if( (uintptr_t) p % alignof( max_align_t) != 0)
      abort();
   memset( p, 'A', 64);

   // calloc
   ip = _mulle_allocator_calloc( allocator, 10, sizeof( int));
   if( ! ip)
      abort();
   for( i = 0; i < 10; i++)
   {
      if( ip[ i] != 0)
         abort();
   }

   // realloc the last allocation (should extend in place if room)
   ip[ 0] = 42;
   ip[ 9] = 99;
   q = _mulle_allocator_realloc( allocator, ip, 20 * sizeof( int));
   if( ! q)
      abort();
   ip = q;
   if( ip[ 0] != 42)
      abort();
   if( ip[ 9] != 99)
      abort();

   // free is a no-op, should not crash
   _mulle_allocator_free( allocator, p);
   _mulle_allocator_free( allocator, ip);

   // allocate a bunch to prove it still works after "frees"
   for( i = 0; i < 100; i++)
   {
      p = _mulle_allocator_malloc( allocator, 32);
      memset( p, (char) i, 32);
   }

   _mulle_arena_done( &arena);
   return( 0);
}
