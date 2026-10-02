#include <mulle-storage/mulle-storage.h>

#include <stdio.h>
#include <string.h>
#include <stdalign.h>


//
// Test that a mulle_pointerarray works on an arena allocator,
// even with interleaved allocations that defeat the last-alloc
// realloc optimization. Proves correctness in the wasteful case.
//
int  main( int argc, char *argv[])
{
   struct mulle_arena          arena;
   struct mulle_allocator      *allocator;
   struct mulle_pointerarray   array;
   unsigned int                i;
   void                        *noise;
   void                        *p;

   _mulle_arena_init( &arena, 512, NULL);
   allocator = _mulle_arena_get_allocator( &arena);

   mulle_pointerarray_init( &array, 4, allocator);

   for( i = 0; i < 200; i++)
   {
      // interleave: do a noise allocation between every add
      // this defeats the last-allocation realloc trick
      noise = _mulle_allocator_malloc( allocator, 16);
      memset( noise, 0xAB, 16);

      mulle_pointerarray_add( &array, (void *) (uintptr_t) (i + 1));
   }

   // verify all entries are intact
   for( i = 0; i < 200; i++)
   {
      p = mulle_pointerarray_get( &array, i);
      if( (uintptr_t) p != i + 1)
      {
         printf( "FAIL at index %u: expected %u, got %lu\n",
                 i, i + 1, (unsigned long) (uintptr_t) p);
         abort();
      }
   }

   if( mulle_pointerarray_get_count( &array) != 200)
      abort();

   // "done" the array — free is no-op on arena, but shouldn't crash
   mulle_pointerarray_done( &array);

   _mulle_arena_done( &arena);
   return( 0);
}
