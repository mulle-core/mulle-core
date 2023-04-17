//
//  mulle-fnv1a.c
//  mulle-data
//
//  Created by Nat! on 19.08.17


#include "mulle-fnv1a.h"

#include <string.h>



uint32_t   _mulle_fnv1a_chained_32( void *buf, size_t len, uint32_t hash)
{
   return( _mulle_fnv1a_chained_32_inline( buf, len, hash));
}


uint64_t   _mulle_fnv1a_chained_64( void *buf, size_t len, uint64_t hash)
{
   return( _mulle_fnv1a_chained_64_inline( buf, len, hash));
}


uint32_t   _mulle_string_hash_32( char *s)
{
   return( _mulle_fnv1a_32( s, strlen( s)));
}


uint64_t   _mulle_string_hash_64( char *s)
{
   return( _mulle_fnv1a_64( s, strlen( s)));
}


uint32_t   _mulle_string_hash_chained_32( char *s, uint32_t hash)
{
   return( _mulle_fnv1a_chained_32( s, strlen( s), hash));
}


uint64_t   _mulle_string_hash_chained_64( char *s, uint64_t hash)
{
   return( _mulle_fnv1a_chained_64( s, strlen( s), hash));
}



// Build it with:
//
// cc -o mulle_objc_fnv1a -DMAIN mulle_fnv1a.c
// to check if it produces same results as reference implementation
//
#ifdef MAIN

#include <stdio.h>
#include <string.h>


int   main( int argc, char * argv[])
{
   if( argc < 2)
      return( -1);

   if( argc == 3)
   {
      switch( argv[ 2][ 1])
      {
      case '3' :
         printf( "0x%08lx\n", (unsigned long) _mulle_objc_fnv1a_32( argv[ 1], strlen( argv[ 1])));
         return( 0);

      case '6' :
         printf( "0x%08llx\n", (unsigned long long) _mulle_objc_fnv1a_64( argv[ 1], strlen( argv[ 1])));
         return( 0);
      }
   }

   printf( "0x%08llx\n", (unsigned long long) _mulle_objc_fnv1a( argv[ 1], strlen( argv[ 1])));
   return 0;
}

#endif
