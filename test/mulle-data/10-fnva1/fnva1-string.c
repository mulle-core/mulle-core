#include <mulle-data/mulle-data.h>
#include <stdio.h>


static void   test_32()
{
   uint32_t   hash;

   hash = _mulle_string_hash_32( "VfL Bochum 1848");
   printf( "%#08x\n", hash);
}


static void   test_64()
{
   uint64_t   hash;

   hash = _mulle_string_hash_64( "VfL Bochum 1848");
   printf( "%#016llx\n", (unsigned long long) hash);
}


int   main( void)
{
   test_32();
   test_64();

   return( 0);
}