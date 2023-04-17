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
#ifndef mulle_hash__h__
#define mulle_hash__h__

#include "include.h"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

// from code.google.com/p/smhasher/wiki/MurmurHash3

//
// Memo: integer multiplication should be unprolematic
//
static inline uint32_t  mulle_hash_avalanche32( uint32_t h)
{
   h ^= h >> 16;
   h *= 0x85ebca6b;
   h ^= h >> 13;
   h *= 0xc2b2ae35;
   h ^= h >> 16;
   return h;
}


// from code.google.com/p/smhasher/wiki/MurmurHash3
static inline uint64_t   mulle_hash_avalanche64( uint64_t h)
{
   h ^= h >> 33;
   h *= 0xff51afd7ed558ccd;
   h ^= h >> 33;
   h *= 0xc4ceb9fe1a85ec53;
   h ^= h >> 33;
   return h;
}


static inline uintptr_t   mulle_hash_avalanche( uintptr_t h)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) mulle_hash_avalanche32( h));
   return( (uintptr_t) mulle_hash_avalanche64( h));
}


static inline uintptr_t   mulle_integer_hash( uintptr_t p)
{
   return( mulle_hash_avalanche( p));
}


static inline uintptr_t   mulle_pointer_hash( void *p)
{
   return( mulle_hash_avalanche( (uintptr_t) p));
}


static inline uintptr_t   mulle_float_hash( float f)
{
   union
   {
      float      f;
      uint32_t   v;
   } x =
   {
      .f = f
   };

   MULLE_C_ASSERT( sizeof( float) == sizeof( uint32_t));

   return( mulle_integer_hash( x.v));
}


static inline uintptr_t   mulle_double_hash( double f)
{
   return( mulle_float_hash( (float) f));
}


static inline uintptr_t   mulle_long_double_hash( long double ld)
{
   return( mulle_double_hash( (double) ld));
}


static inline uintptr_t   mulle_long_long_hash( long long value)
{
   return( mulle_hash_avalanche64( (uint64_t) value));
}



//
// the chained hash and the regular hash are not compatible!
//
// is the hash littleendian/bigendian agnostic ?
//
// length == -1 is not supported here. Use _mulle_hash_string for C
// strings.
//
MULLE_DATA_GLOBAL
uint32_t   _mulle_hash_chained_32( void *bytes, size_t length, uint32_t hash);

MULLE_DATA_GLOBAL
uint64_t   _mulle_hash_chained_64( void *bytes, size_t length, uint64_t hash);


static inline uintptr_t   _mulle_hash_chained( void *bytes, size_t length, uintptr_t hash)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_chained_32( bytes, length, hash));
   return( (uintptr_t) _mulle_hash_chained_64( bytes, length, hash));
}


static inline uintptr_t   mulle_hash_chained( void *bytes, size_t length, uintptr_t hash)
{
   if( ! bytes)
      return( 0);
   return( _mulle_hash_chained( bytes, length, hash));
}



MULLE_DATA_GLOBAL
uint32_t   _mulle_hash_32( void *bytes, size_t length);

MULLE_DATA_GLOBAL
uint64_t   _mulle_hash_64( void *bytes, size_t length);


static inline uintptr_t   _mulle_hash( void *bytes, size_t length)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_32( bytes, length));
   return( (uintptr_t) _mulle_hash_64( bytes, length));
}


static inline uintptr_t   mulle_hash( void *bytes, size_t length)
{
   if( ! bytes)
      return( 0);
   return( _mulle_hash( bytes, length));
}

#endif
