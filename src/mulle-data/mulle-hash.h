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
#ifndef mulle_hash_h__
#define mulle_hash_h__

#include "include.h"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

// from code.google.com/p/smhasher/wiki/MurmurHash3

/**
 * Avalanche the bits of a 32-bit integer hash value.
 *
 * This function is based on the MurmurHash3 algorithm and is used to improve the
 * distribution of the hash value. It performs a series of XOR, multiplication,
 * and bit shift operations to "avalanche" the bits of the input hash value,
 * resulting in a more uniform distribution of the output hash value.
 *
 * @param h The 32-bit integer hash value to be avalanched.
 * @return The avalanched 32-bit integer hash value.
 */
//
// Memo: integer multiplication should be unprolematic
// There is always a 32 bit and a 64 bit version, and uintptr_t is the
// platform specific preferencel, which chooses between both.
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


/**
 * Avalanche the bits of a 64-bit integer hash value.
 *
 * This function is based on the MurmurHash3 algorithm and is used to improve the
 * distribution of the hash value. It performs a series of XOR, multiplication,
 * and bit shift operations to "avalanche" the bits of the input hash value,
 * resulting in a more uniform distribution of the output hash value.
 *
 * @param h The 64-bit integer hash value to be avalanched.
 * @return The avalanched 64-bit integer hash value.
 */
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


/**
 * Avalanche the bits of a hash value.
 *
 * This function is based on the farmhash algorithm and is used to improve the
 * distribution of the hash value. It performs a series of XOR, multiplication,
 * and bit shift operations to "avalanche" the bits of the input hash value,
 * resulting in a more uniform distribution of the output hash value.
 *
 * @param h The hash value to be avalanched.
 * @return The avalanched hash value.
 */
static inline uintptr_t   mulle_hash_avalanche( uintptr_t h)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) mulle_hash_avalanche32( h));
   return( (uintptr_t) mulle_hash_avalanche64( h));
}


/**
 * Calculates a hash value for an integer.
 *
 * This function takes an integer value and applies the mulle_hash_avalanche()
 * function to it, which is based on the farmhash algorithm. The resulting
 * hash value is returned as an uintptr_t.
 *
 * @param p The integer value to hash.
 * @return The avalanched hash value of the input integer.
 */
static inline uintptr_t   mulle_integer_hash( uintptr_t p)
{
   return( mulle_hash_avalanche( p));
}


/**
 * Calculates a hash value for a pointer.
 *
 * This function takes a pointer value and applies the mulle_hash_avalanche()
 * function to it, which is based on the farmhash algorithm. The resulting
 * hash value is returned as an uintptr_t.
 *
 * @param p The pointer value to hash.
 * @return The avalanched hash value of the input pointer.
 */
static inline uintptr_t   mulle_pointer_hash( void *p)
{
   return( mulle_hash_avalanche( (uintptr_t) p));
}


/**
 * Calculates a hash value for a float.
 *
 * This function takes a float value, converts it to a uint32_t, and then applies
 * the mulle_integer_hash() function to it, which is based on the farmhash
 * algorithm. The resulting hash value is returned as an uintptr_t.
 *
 * @param f The float value to hash.
 * @return The avalanched hash value of the input float.
 */
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


/**
 * Calculates a hash value for a double.
 *
 * This function takes a double value, converts it to a float, and then applies
 * the mulle_float_hash() function to it, which is based on the farmhash
 * algorithm. The resulting hash value is returned as an uintptr_t.
 *
 * @param f The double value to hash.
 * @return The avalanched hash value of the input double.
 */
static inline uintptr_t   mulle_double_hash( double f)
{
   return( mulle_float_hash( (float) f));
}


/**
 * Calculates a hash value for a long double.
 *
 * This function takes a long double value, converts it to a double, and then applies
 * the mulle_double_hash() function to it, which is based on the farmhash
 * algorithm. The resulting hash value is returned as an uintptr_t.
 *
 * @param ld The long double value to hash.
 * @return The avalanched hash value of the input long double.
 */
static inline uintptr_t   mulle_long_double_hash( long double ld)
{
   return( mulle_double_hash( (double) ld));
}


/**
 * Calculates a hash value for a long long integer.
 *
 * This function takes a long long integer value and applies the mulle_hash_avalanche64()
 * function to it, which is based on the farmhash algorithm. The resulting hash
 * value is returned as an uintptr_t.
 *
 * @param value The long long integer value to hash.
 * @return The avalanched hash value of the input long long integer.
 */
static inline uintptr_t   mulle_long_long_hash( long long value)
{
   return( mulle_hash_avalanche64( (uint64_t) value));
}



/**
 * Calculates a chained 32-bit hash value for the given bytes.
 *
 * This function takes a pointer to a byte array, the length of the array, and
 * a pointer, that on the first call points to void pointer that is NULL.
 * On subsequent calls you need to pass the contents of this void pointer
 * unchanged to mulle_hash_chained_32.
 *
 * Example:
 * void        *state = NULL; // important!
 * uint32_t    hash;
 *
 * (void) mulle_hash_chained_32( "VfL", 3, &state);
 * (void) mulle_hash_chained_32( "Bochum", 6, &state);
 * hash = mulle_hash_chained_32( NULL, 0, &state);
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array. Call with 0 for last chunk.
 * @param state_p The opaque hash state
 * @return The 32-bit chained hash value once length is zero
 */
MULLE__DATA_GLOBAL
uint32_t   mulle_hash_chained_32( void *bytes, size_t length, void **state_p);

/**
 * Calculates a chained 64-bit hash value for the given bytes.
 *
 * This function takes a pointer to a byte array, the length of the array, and
 * a pointer, that on the first call points to void pointer that is NULL.
 * On subsequent calls you need to pass the contents of this void pointer
 * unchanged to mulle_hash_chained_64.
 *
 * Example:
 * void        *state = NULL; // important!
 * uint64_t    hash;
 *
 * (void) mulle_hash_chained_64( "VfL", 3, &state);
 * (void) mulle_hash_chained_64( "Bochum", 6, &state);
 * hash = mulle_hash_chained_64( NULL, 0, &state);
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array. Call with 0 for last chunk.
 * @param state_p The opaque hash state
 * @return The 64-bit chained hash value once length is zero
 */
MULLE__DATA_GLOBAL
uint64_t   mulle_hash_chained_64( void *bytes, size_t length, void **state_p);



/**
 * Calculates a chained hash value for the given bytes, using either a 32-bit or
 * 64-bit hash algorithm depending on the size of `uintptr_t`.
 *
 * This function takes a pointer to a byte array, the length of the array, and
 * an initial hash value. It then applies the farmhash algorithm to the bytes
 * and returns the resulting hash value. mulle_hash_chained does not provide
 * intermediate results. Reset the state to NULL after you retrieved
 * the hash, before hashing the next chain.
 *
 * Example:
 * void        *state = NULL; // important!
 * uintptr_t   hash;
 *
 * (void) mulle_hash_chained( "VfL", 3, &state);
 * (void) mulle_hash_chained( "Bochum", 6, &state);
 * hash = mulle_hash_chained( NULL, 0, &state);
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array. Call with 0 for hash retrieval.
 * @param state_p The opaque hash state
 * @return The chained hash value. Only valid of length was 0.
 */
static inline uintptr_t   mulle_hash_chained( void *bytes,
                                               size_t length,
                                               void **state_p)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) mulle_hash_chained_32( bytes, length, state_p));
   return( (uintptr_t) mulle_hash_chained_64( bytes, length, state_p));
}



/**
 * Calculates a 32-bit hash value for the given byte array.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the farmhash algorithm to calculate a 32-bit hash value.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The 32-bit hash value.
 */
MULLE__DATA_GLOBAL
uint32_t   _mulle_hash_32( void *bytes, size_t length);


/**
 * Calculates a 64-bit hash value for the given byte array.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the farmhash algorithm to calculate a 64-bit hash value.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The 64-bit hash value.
 */
MULLE__DATA_GLOBAL
uint64_t   _mulle_hash_64( void *bytes, size_t length);


/**
 * Calculates a hash value for the given byte array using the MurmurHash3 algorithm.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the farmhash algorithm to calculate a hash value. The size of
 * the returned hash value depends on the size of the `uintptr_t` type on the
 * current platform, being either 32-bit or 64-bit.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The hash value.
 */
static inline uintptr_t   _mulle_hash( void *bytes, size_t length)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_32( bytes, length));
   return( (uintptr_t) _mulle_hash_64( bytes, length));
}


/**
 * Calculates a hash value for the given byte array using the MurmurHash3 algorithm.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the farmhash algorithm to calculate a hash value. The size of
 * the returned hash value depends on the size of the `uintptr_t` type on the
 * current platform, being either 32-bit or 64-bit.
 *
 * If the `bytes` parameter is `NULL`, this function will return 0.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The hash value.
 */
static inline uintptr_t   mulle_hash( void *bytes, size_t length)
{
   if( ! bytes)
      return( 0);
   return( _mulle_hash( bytes, length));
}

#endif
