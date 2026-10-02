#include <mulle-data/mulle-data.h>
#include <stdio.h>


int   main( void)
{
   uint32_t    hash32;
   uint64_t    hash64;
   void        *state;

   // 32-bit chained == one-shot
   state = NULL;
   mulle_hash_chained_32( "VfL Bochum", 10, &state);
   mulle_hash_chained_32( " 1848", 5, &state);
   hash32 = mulle_hash_chained_32( NULL, 0, &state);
   printf( "chain32==shot32: %d\n",
           hash32 == _mulle_hash_32( "VfL Bochum 1848", 15));
   printf( "chain32: 0x%08lx\n", (unsigned long) hash32);

   // 64-bit chained == one-shot
   state = NULL;
   mulle_hash_chained_64( "VfL Bochum", 10, &state);
   mulle_hash_chained_64( " 1848", 5, &state);
   hash64 = mulle_hash_chained_64( NULL, 0, &state);
   printf( "chain64==shot64: %d\n",
           hash64 == _mulle_hash_64( "VfL Bochum 1848", 15));
   printf( "chain64: 0x%016llx\n", (unsigned long long) hash64);

   // pointer-width chained == one-shot (via _add/_final)
   {
      uintptr_t   hash;

      state = NULL;
      mulle_hash_chained_add( "VfL Bochum", 10, &state);
      mulle_hash_chained_add( " 1848", 5, &state);
      hash = mulle_hash_chained_final( &state);
      printf( "add/final==shot: %d\n",
              hash == _mulle_hash( "VfL Bochum 1848", 15));
   }

   return( 0);
}
