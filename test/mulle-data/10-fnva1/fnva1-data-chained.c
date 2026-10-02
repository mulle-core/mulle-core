#include <mulle-data/mulle-data.h>
#include <stdio.h>


static void   test_32()
{
   uint32_t   hash;

   hash = _mulle_fnv1a_init_32();
   hash = _mulle_fnv1a_chained_32( "VfL", 3, hash);
   hash = _mulle_fnv1a_chained_32( " Bochum", 7, hash);
   hash = _mulle_fnv1a_chained_32( " 1848", 5, hash);
   printf( "%#08x\n", hash);
}


static void   test_64()
{
   uint64_t   hash;

   hash = _mulle_fnv1a_init_64();
   hash = _mulle_fnv1a_chained_64( "VfL", 3, hash);
   hash = _mulle_fnv1a_chained_64( " Bochum", 7, hash);
   hash = _mulle_fnv1a_chained_64( " 1848", 5, hash);
   printf( "%#016llx\n", (unsigned long long) hash);
}


int   main( void)
{
   test_32();
   test_64();

   return( 0);
}