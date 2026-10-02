#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>
#include <stdalign.h>


struct point
{
   double   x;
   double   y;
};


int  main( int argc, char *argv[])
{
   struct mulle_arena   arena;
   struct point         src;
   struct point         *copy;
   char                 *s;
   int                  *zeroed;

   _mulle_arena_init( &arena, 256, NULL);

   // strdup
   s = _mulle_arena_strdup( &arena, "hello world");
   if( strcmp( s, "hello world") != 0)
      abort();

   // memdup
   src.x = 1.0;
   src.y = 2.0;
   copy = _mulle_arena_memdup( &arena, &src, sizeof( struct point));
   if( copy->x != 1.0 || copy->y != 2.0)
      abort();

   // calloc should zero
   zeroed = _mulle_arena_calloc( &arena, 16 * sizeof( int), alignof( int));
   if( zeroed[ 0] != 0 || zeroed[ 15] != 0)
      abort();

   // null-safe wrappers
   if( mulle_arena_alloc( NULL, 10, 1) != NULL)
      abort();
   if( mulle_arena_strdup( NULL, "x") != NULL)
      abort();
   if( mulle_arena_memdup( NULL, &src, sizeof( src)) != NULL)
      abort();
   if( mulle_arena_calloc( NULL, 10, 1) != NULL)
      abort();
   if( mulle_arena_get_page_count( NULL) != 0)
      abort();
   if( mulle_arena_get_allocator( NULL) != NULL)
      abort();

   mulle_arena_done( NULL);  // should not crash

   _mulle_arena_done( &arena);
   return( 0);
}
