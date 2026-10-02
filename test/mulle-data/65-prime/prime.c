#include <mulle-data/mulle-data.h>

#include <stdio.h>


//
// mulle_prime_for_depth reads a table, mulle_prime_hash_for_depth uses a
// switch with the same numbers written out a second time. This checks that
// both stay in sync.
//
int   main( int argc, char *argv[])
{
   uintptr_t   expect;
   uintptr_t   prime;
   uintptr_t   value;
   uintptr_t   values[] = { 0, 1, 2, 17, 1848, 0x7FFFFFFF };
   int         depth;
   unsigned    i;
   unsigned    fail;

   fail = 0;
   for( depth = -32; depth <= 32; depth++)
   {
      prime = mulle_prime_for_depth( depth);
      if( prime != mulle_prime_for_depth( -depth))
      {
         printf( "depth %d: table not symmetric\n", depth);
         ++fail;
      }

      for( i = 0; i < sizeof( values) / sizeof( values[ 0]); i++)
      {
         value  = values[ i];
         expect = prime ? value % prime : 0;
         if( mulle_prime_hash_for_depth( value, depth) != expect)
         {
            printf( "depth %d value %td: switch says %td, table says %td\n",
                    depth,
                    value,
                    mulle_prime_hash_for_depth( value, depth),
                    expect);
            ++fail;
         }
      }
   }

   printf( "mismatches: %u\n", fail);
   printf( "prime_for_depth(  0) is zero  : %d\n", mulle_prime_for_depth( 0) == 0);
   printf( "prime_for_depth(  1) is one   : %d\n", mulle_prime_for_depth( 1) == 1);
   printf( "hash_for_depth( 1848, 1)      : %td\n", mulle_prime_hash_for_depth( 1848, 1));
   printf( "hash_for_depth( 1848, 16)     : %td\n", mulle_prime_hash_for_depth( 1848, 16));

   return( 0);
}
