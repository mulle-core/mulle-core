#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>


int  main( int argc, char *argv[])
{
   struct mulle_arena   arena;
   char                 *p1;
   char                 *p2;
   size_t               pages_before;
   unsigned int         i;

   _mulle_arena_init( &arena, 128, NULL);

   // allocate enough to create multiple pages
   for( i = 0; i < 20; i++)
      _mulle_arena_alloc( &arena, 100, 1);

   pages_before = _mulle_arena_get_page_count( &arena);
   if( pages_before <= 1)
      abort();

   // default reset frees all pages (back to empty like post-init)
   _mulle_arena_reset( &arena);

   if( _mulle_arena_get_page_count( &arena) != 0)
      abort();

   // should be able to allocate from the reset arena
   p1 = _mulle_arena_alloc( &arena, 64, 1);
   memset( p1, 'X', 64);
   if( p1[0] != 'X' || p1[63] != 'X')
      abort();

   // allocate more to prove arena is fully functional after reset
   for( i = 0; i < 20; i++)
   {
      p2 = _mulle_arena_alloc( &arena, 100, 1);
      memset( p2, (char) i, 100);
   }

   // first allocation still intact
   if( p1[0] != 'X' || p1[63] != 'X')
      abort();

   _mulle_arena_done( &arena);
   return( 0);
}
