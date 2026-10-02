#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <stdalign.h>


int  main( int argc, char *argv[])
{
   struct mulle_arena   arena;
   void                 *p;
   unsigned int         i;
   unsigned int         alignment;

   _mulle_arena_init( &arena, 512, NULL);

   // test various alignments: 1, 2, 4, 8, 16, 32, 64
   for( i = 0; i < 7; i++)
   {
      alignment = 1u << i;
      p = _mulle_arena_alloc( &arena, 17, alignment);
      if( (uintptr_t) p % alignment != 0)
      {
         printf( "alignment %u failed: p=%p\n", alignment, p);
         abort();
      }
   }

   // interleave different sizes and alignments
   for( i = 0; i < 200; i++)
   {
      alignment = 1u << (i % 5);  // 1, 2, 4, 8, 16
      p = _mulle_arena_alloc( &arena, (i % 37) + 1, alignment);
      if( (uintptr_t) p % alignment != 0)
         abort();
   }

   _mulle_arena_done( &arena);
   return( 0);
}
