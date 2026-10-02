#include <mulle-data/mulle-data.h>

#include <stdio.h>
#include <string.h>


//
// Known-answer tests for FNV-1a and xxHash. These values are from the
// reference implementations and must not change across versions/platforms.
// If they do, hash tables serialised to disk or used as wire protocols break.
//
int   main( int argc, char *argv[])
{
   // FNV-1a reference vectors (from http://www.isthe.com/chongo/tech/comp/fnv/)
   printf( "fnv1a_32(\"\")       : 0x%08lx\n",
           (unsigned long) _mulle_fnv1a_32( "", 0));
   printf( "fnv1a_32(\"a\")      : 0x%08lx\n",
           (unsigned long) _mulle_fnv1a_32( "a", 1));
   printf( "fnv1a_32(\"foobar\") : 0x%08lx\n",
           (unsigned long) _mulle_fnv1a_32( "foobar", 6));
   printf( "fnv1a_64(\"\")       : 0x%016llx\n",
           (unsigned long long) _mulle_fnv1a_64( "", 0));
   printf( "fnv1a_64(\"a\")      : 0x%016llx\n",
           (unsigned long long) _mulle_fnv1a_64( "a", 1));
   printf( "fnv1a_64(\"foobar\") : 0x%016llx\n",
           (unsigned long long) _mulle_fnv1a_64( "foobar", 6));

   // xxHash reference vectors (seed 0, from xxhsum verification)
   printf( "xxh32(\"\")          : 0x%08lx\n",
           (unsigned long) _mulle_hash_32( "", 0));
   printf( "xxh32(\"a\")         : 0x%08lx\n",
           (unsigned long) _mulle_hash_32( "a", 1));
   printf( "xxh32(\"foobar\")    : 0x%08lx\n",
           (unsigned long) _mulle_hash_32( "foobar", 6));
   printf( "xxh64(\"\")          : 0x%016llx\n",
           (unsigned long long) _mulle_hash_64( "", 0));
   printf( "xxh64(\"a\")         : 0x%016llx\n",
           (unsigned long long) _mulle_hash_64( "a", 1));
   printf( "xxh64(\"foobar\")    : 0x%016llx\n",
           (unsigned long long) _mulle_hash_64( "foobar", 6));

   // chained hash: "VfL" + "Bochum" must equal single-shot "VfLBochum"
   {
      void       *state = NULL;
      uintptr_t   chained;
      uintptr_t   oneshot;

      (void) mulle_hash_chained( "VfL", 3, &state);
      chained = mulle_hash_chained( "Bochum", 6, &state);
      // not final yet, need to finalise
      chained = mulle_hash_chained( NULL, 0, &state);
      oneshot = _mulle_hash( "VfLBochum", 9);
      printf( "chained==oneshot   : %d\n", chained == oneshot);
   }

   // type-safe _add/_final wrappers: same result as legacy style
   {
      void       *state = NULL;
      uintptr_t   hash;

      mulle_hash_chained_add( "VfL", 3, &state);
      mulle_hash_chained_add( "Bochum", 6, &state);
      hash = mulle_hash_chained_final( &state);
      printf( "add/final==oneshot : %d\n", hash == _mulle_hash( "VfLBochum", 9));
   }

   // mulle_hash(NULL, 0) == mulle_hash("", 0) — no in-band sentinel
   printf( "hash(NULL)==hash(\"\") : %d\n",
           mulle_hash( NULL, 0) == mulle_hash( "", 0));

   // mulle_integer_hash and mulle_long_long_hash must agree for values
   // that fit in uintptr_t (they must produce the same hash on all widths)
   printf( "int==ll(1848)      : %d\n",
           mulle_integer_hash( 1848) == mulle_long_long_hash( 1848LL));
   printf( "int==ll(0)         : %d\n",
           mulle_integer_hash( 0) == mulle_long_long_hash( 0LL));
   printf( "int==ll(UINT32_MAX): %d\n",
           mulle_integer_hash( (uintptr_t) 0xFFFFFFFF) == mulle_long_long_hash( 0xFFFFFFFFLL));

   return( 0);
}
