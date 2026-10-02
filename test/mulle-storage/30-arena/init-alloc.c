#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <stdalign.h>


int  main( int argc, char *argv[])
{
   struct mulle_arena   arena;
   void                 *p;
   int                  *ip;
   double               *dp;

   _mulle_arena_init( &arena, 256, NULL);

   // basic allocation
   p = _mulle_arena_alloc( &arena, 64, 1);
   if( ! p)
      abort();

   // aligned allocation
   ip = _mulle_arena_alloc( &arena, sizeof( int), alignof( int));
   if( (uintptr_t) ip % alignof( int) != 0)
      abort();
   *ip = 1848;

   dp = _mulle_arena_alloc( &arena, sizeof( double), alignof( double));
   if( (uintptr_t) dp % alignof( double) != 0)
      abort();
   *dp = 3.14;

   // verify data survives subsequent allocations
   _mulle_arena_alloc( &arena, 128, 1);

   if( *ip != 1848)
      abort();
   if( *dp != 3.14)
      abort();

   if( _mulle_arena_get_page_count( &arena) == 0)
      abort();

   _mulle_arena_done( &arena);
   return( 0);
}
