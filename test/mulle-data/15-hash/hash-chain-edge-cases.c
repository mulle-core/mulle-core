#include <mulle-data/mulle-data.h>
#include <stdio.h>


int   main( void)
{
   uintptr_t  hash;
   void       *state;

   // Test 1: Single chunk vs multiple small chunks
   hash = _mulle_hash_64( "ABC", 3);
   printf( "single: %#016tx\n", hash);

   state = NULL;
   mulle_hash_chained_64( "A", 1, &state);
   mulle_hash_chained_64( "B", 1, &state);
   mulle_hash_chained_64( "C", 1, &state);
   hash = mulle_hash_chained_64( NULL, 0, &state);
   printf( "chained: %#016tx\n", hash);

   // Test 2: Zero-length chunks in the middle
   state = NULL;
   mulle_hash_chained_64( "X", 1, &state);
   mulle_hash_chained_64( "", 0, &state);
   hash = mulle_hash_chained_64( NULL, 0, &state);
   printf( "zero-mid: %#016tx\n", hash);

   // Test 3: Only zero-length finalize
   state = NULL;
   hash = mulle_hash_chained_64( NULL, 0, &state);
   printf( "zero-only: %#016tx\n", hash);

   // Test 4: Multiple zero-length before finalize
   state = NULL;
   mulle_hash_chained_64( "Y", 1, &state);
   mulle_hash_chained_64( "", 0, &state);
   mulle_hash_chained_64( "", 0, &state);
   hash = mulle_hash_chained_64( NULL, 0, &state);
   printf( "multi-zero: %#016tx\n", hash);

   return( 0);
}
