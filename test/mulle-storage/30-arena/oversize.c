#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>


int  main( int argc, char *argv[])
{
   struct mulle_arena   arena;
   char                 *p;
   char                 *q;
   size_t               page_count;

   // small page size to force page growth quickly
   _mulle_arena_init( &arena, 64, NULL);

   // fill first page
   p = _mulle_arena_alloc( &arena, 60, 1);
   memset( p, 'A', 60);
   page_count = _mulle_arena_get_page_count( &arena);

   // this should trigger a new page
   q = _mulle_arena_alloc( &arena, 60, 1);
   memset( q, 'B', 60);

   if( _mulle_arena_get_page_count( &arena) <= page_count)
      abort();

   // verify first allocation is still intact
   if( p[0] != 'A' || p[59] != 'A')
      abort();
   if( q[0] != 'B' || q[59] != 'B')
      abort();

   // oversized allocation (bigger than page_size)
   page_count = _mulle_arena_get_page_count( &arena);
   p = _mulle_arena_alloc( &arena, 1024, 1);
   memset( p, 'C', 1024);

   if( _mulle_arena_get_page_count( &arena) <= page_count)
      abort();

   // verify oversized data
   if( p[0] != 'C' || p[1023] != 'C')
      abort();

   _mulle_arena_done( &arena);
   return( 0);
}
