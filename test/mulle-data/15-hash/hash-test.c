#include <mulle-data/mulle-data.h>
#include <stdio.h>


int   main( void)
{
   // mulle_hash(NULL, 0) == mulle_hash("", 0) — no sentinel
   printf( "null==empty: %d\n",
           mulle_hash( NULL, 0) == mulle_hash( "", 0));

   // pointer-width hash of data is non-zero
   printf( "data!=0: %d\n",
           mulle_hash( "VfL Bochum 1848", 15) != 0);

   // mulle_integer_hash and mulle_pointer_hash agree for same numeric value
   printf( "int==ptr: %d\n",
           mulle_integer_hash( 1848) == mulle_pointer_hash( (void *) 1848));

   // mulle_integer_hash == mulle_long_long_hash for values that fit
   printf( "int==ll: %d\n",
           mulle_integer_hash( 1848) == mulle_long_long_hash( 1848));

   // float and double of same value may differ (different representation widths)
   // but each should be non-zero
   printf( "float!=0: %d\n", mulle_float_hash( 1848.0f) != 0);
   printf( "double!=0: %d\n", mulle_double_hash( 1848.0) != 0);

   // pinned-width hashes are stable across platforms
   printf( "32: 0x%08lx\n",
           (unsigned long) _mulle_hash_32( "VfL Bochum 1848", 15));
   printf( "64: 0x%016llx\n",
           (unsigned long long) _mulle_hash_64( "VfL Bochum 1848", 15));

   return( 0);
}
