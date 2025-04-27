//
//  Copyright (C) 2011 Nat!, Mulle kybernetiK.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#include "mulle-hash.h"

#define XXH_STATIC_LINKING_ONLY /* access advanced declarations */
#define XXH_IMPLEMENTATION      /* access definitions */

#include "xxhash.h"

#include <string.h>
// lots of other code snipped

uint32_t   _mulle_hash_32( void *bytes, size_t length)
{
   return( XXH32( bytes, length, 0));
}


uint64_t   _mulle_hash_64( void *bytes, size_t length)
{
   return( XXH64( bytes, length, 0));
}


uint32_t   mulle_hash_chained_32( void *bytes, size_t length, void **state_p)
{
   XXH32_state_t  *state;
   XXH32_hash_t   hash;

   if( ! state_p)
      return( (uint32_t) -1);

   state = *state_p;
   if( ! state)
   {
      state = XXH32_createState();
      assert( state);

      XXH32_reset( state, 0);

      *state_p = state;
   }

   if( length)
   {
      assert( bytes);

      XXH32_update( state, bytes, length);
      return( (uint32_t) -1);
   }

   hash = XXH32_digest( state);

   XXH32_freeState( state);
#ifdef DEBUG
   *state_p = (void *) (uintptr_t) 0xDEADFACEDEADFACEULL;
#else
   *state_p = NULL;
#endif

   return( hash);
}


uint64_t   mulle_hash_chained_64( void *bytes, size_t length, void **state_p)
{
   XXH64_state_t  *state;
   XXH64_hash_t   hash;

   if( ! state_p)
      return( (uint64_t) -1);

   state = *state_p;
   if( ! state)
   {
      state = XXH64_createState();
      assert( state);

      XXH64_reset( state, 0);

      *state_p = state;
   }

   if( length)
   {
      assert( bytes);

      XXH64_update( state, bytes, length);
      return( (uint64_t) -1);
   }

   hash = XXH64_digest( state);

   XXH64_freeState( state);
#ifdef DEBUG
   *state_p = (void *) (uintptr_t) 0xDEADFACEDEADFACEULL;
#else
   *state_p = NULL;
#endif

   return( hash);
}


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
         printf( "0x%08lx\n", (unsigned long) _mulle_hash_32( argv[ 1], strlen( argv[ 1])));
         return( 0);

      case '6' :
         printf( "0x%08llx\n", (unsigned long long) _mulle_hash_64( argv[ 1], strlen( argv[ 1])));
         return( 0);
      }
   }

   printf( "0x%08llx\n", (unsigned long long) _mulle_hash( argv[ 1], strlen( argv[ 1])));
   return 0;
}

#endif
