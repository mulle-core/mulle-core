//
//  mulle-hash.h
//  mulle-data
//
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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
 * This function selects between the 32-bit and 64-bit MurmurHash3 finalizer
 * based on pointer width. It performs a series of XOR, multiplication, and bit
 * shift operations to "avalanche" the bits of the input hash value, resulting
 * in a more uniform distribution of the output hash value.
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
 * function (MurmurHash3 finalizer). The resulting hash value is returned as
 * an uintptr_t.
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
 * function (MurmurHash3 finalizer). The resulting hash value is returned as
 * an uintptr_t.
 *
 * @param p The pointer value to hash.
 * @return The avalanched hash value of the input pointer.
 */
static inline uintptr_t   mulle_pointer_hash( const void *p)
{
   return( mulle_hash_avalanche( (uintptr_t) p));
}


/**
 * Calculates a hash value for a float.
 *
 * This function takes a float value, canonicalises negative zero to positive
 * zero, converts it to a uint32_t via type-punning, and then applies the
 * mulle_hash_avalanche() function. The resulting hash value is returned as an
 * uintptr_t.
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
   } x;

   MULLE_C_ASSERT( sizeof( float) == sizeof( uint32_t));

   // canonicalise -0.0 to +0.0 so equal values hash identically
   x.f = f + 0.0f;
   return( mulle_integer_hash( x.v));
}


/**
 * Calculates a hash value for a double.
 *
 * This function hashes the full 64-bit representation of a double, so no
 * precision is lost. Negative zero is canonicalised to positive zero.
 * On a 32-bit platform, the two halves are combined via XOR before
 * avalanching.
 *
 * @param f The double value to hash.
 * @return The avalanched hash value of the input double.
 */
static inline uintptr_t   mulle_double_hash( double f)
{
   union
   {
      double     d;
      uint64_t   v;
   } x;

   MULLE_C_ASSERT( sizeof( double) == sizeof( uint64_t));

   // canonicalise -0.0 to +0.0
   x.d = f + 0.0;

   if( sizeof( uintptr_t) >= sizeof( uint64_t))
      return( mulle_hash_avalanche64( x.v));

   // 32-bit: fold halves together, then avalanche
   return( mulle_hash_avalanche32( (uint32_t) (x.v ^ (x.v >> 32))));
}


/**
 * Calculates a hash value for a long double.
 *
 * This function converts the long double to a double and then hashes it
 * with mulle_double_hash(). Values outside the double range will saturate
 * to infinity. Negative zero is canonicalised to positive zero.
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
 * This function takes a long long integer value and applies the appropriate
 * avalanche function. On 64-bit platforms it uses mulle_hash_avalanche64();
 * on 32-bit it folds the two halves together and uses mulle_hash_avalanche32(),
 * so it agrees in width with mulle_integer_hash().
 *
 * @param value The long long integer value to hash.
 * @return The avalanched hash value of the input long long integer.
 */
static inline uintptr_t   mulle_long_long_hash( long long value)
{
   uint64_t   v = (uint64_t) value;

   if( sizeof( uintptr_t) >= sizeof( uint64_t))
      return( mulle_hash_avalanche64( v));

   // 32-bit: fold halves together, then avalanche
   return( mulle_hash_avalanche32( (uint32_t) (v ^ (v >> 32))));
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
 * @param bytes The byte array to hash. Pass NULL to finalise and retrieve hash.
 * @param length The length of the byte array.
 * @param state_p The opaque hash state
 * @return The 32-bit chained hash value. Only valid when bytes is NULL.
 */
MULLE__DATA_GLOBAL
uint32_t   mulle_hash_chained_32( const void *bytes, size_t length, void **state_p);

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
 * @param bytes The byte array to hash. Pass NULL to finalise and retrieve hash.
 * @param length The length of the byte array.
 * @param state_p The opaque hash state
 * @return The 64-bit chained hash value. Only valid when bytes is NULL.
 */
MULLE__DATA_GLOBAL
uint64_t   mulle_hash_chained_64( const void *bytes, size_t length, void **state_p);


/**
 * Calculates a chained hash value for the given bytes, using either a 32-bit or
 * 64-bit hash algorithm depending on the size of `uintptr_t`.
 *
 * This function takes a pointer to a byte array, the length of the array, and
 * an initial hash value. It then applies xxHash to the bytes and returns the
 * resulting hash value. mulle_hash_chained does not provide intermediate
 * results. Reset the state to NULL after you retrieved the hash, before
 * hashing the next chain.
 *
 * Example:
 * void        *state = NULL; // important!
 * uintptr_t   hash;
 *
 * (void) mulle_hash_chained( "VfL", 3, &state);
 * (void) mulle_hash_chained( "Bochum", 6, &state);
 * hash = mulle_hash_chained( NULL, 0, &state);
 *
 * @param bytes The byte array to hash. Pass NULL to finalise and retrieve hash.
 * @param length The length of the byte array.
 * @param state_p The opaque hash state
 * @return The chained hash value. Only valid when bytes was NULL.
 */
static inline uintptr_t   mulle_hash_chained( const void *bytes,
                                               size_t length,
                                               void **state_p)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) mulle_hash_chained_32( bytes, length, state_p));
   return( (uintptr_t) mulle_hash_chained_64( bytes, length, state_p));
}


/**
 * Finalises and releases the chained 32-bit hash state, discarding the result.
 *
 * Call this to abandon a chained hash computation that will not be completed.
 * After this call, `*state_p` is NULL and may be reused for a new chain.
 * Safe to call with a NULL state_p or an already-NULL *state_p.
 *
 * @param state_p The opaque hash state to release.
 */
static inline void   mulle_hash_chained_32_done( void **state_p)
{
   mulle_hash_chained_32( NULL, 0, state_p);
}

/**
 * Finalises and releases the chained 64-bit hash state, discarding the result.
 *
 * Call this to abandon a chained hash computation that will not be completed.
 * After this call, `*state_p` is NULL and may be reused for a new chain.
 * Safe to call with a NULL state_p or an already-NULL *state_p.
 *
 * @param state_p The opaque hash state to release.
 */
static inline void   mulle_hash_chained_64_done( void **state_p)
{
   mulle_hash_chained_64( NULL, 0, state_p);
}


/**
 * Finalises and releases the chained hash state, discarding the result (pointer-width variant).
 *
 * Call this to abandon a chained hash computation that will not be completed.
 * After this call, `*state_p` is NULL and may be reused for a new chain.
 *
 * @param state_p The opaque hash state to release.
 */
static inline void   mulle_hash_chained_done( void **state_p)
{
   mulle_hash_chained( NULL, 0, state_p);
}


// -----------------------------------------------------------------------
// Type-safe chained hash wrappers
//
// Use mulle_hash_chained_add() to feed data and mulle_hash_chained_final()
// to retrieve the hash. This avoids the in-band sentinel: _add returns void
// (nothing to misinterpret), and _final returns only the real hash.
// -----------------------------------------------------------------------

/**
 * Feeds bytes into an ongoing chained 32-bit hash computation.
 *
 * @param bytes  The byte array to hash (must not be NULL).
 * @param length The length of the byte array.
 * @param state_p The opaque hash state (initialise *state_p to NULL).
 */
static inline void   mulle_hash_chained_32_add( const void *bytes,
                                                size_t length,
                                                void **state_p)
{
   mulle_hash_chained_32( bytes, length, state_p);
}


/**
 * Finalises a chained 32-bit hash computation and returns the hash.
 *
 * After this call, `*state_p` is NULL and may be reused.
 *
 * @param state_p The opaque hash state.
 * @return The final 32-bit hash value.
 */
static inline uint32_t   mulle_hash_chained_32_final( void **state_p)
{
   return( mulle_hash_chained_32( NULL, 0, state_p));
}


/**
 * Feeds bytes into an ongoing chained 64-bit hash computation.
 *
 * @param bytes  The byte array to hash (must not be NULL).
 * @param length The length of the byte array.
 * @param state_p The opaque hash state (initialise *state_p to NULL).
 */
static inline void   mulle_hash_chained_64_add( const void *bytes,
                                                size_t length,
                                                void **state_p)
{
   mulle_hash_chained_64( bytes, length, state_p);
}


/**
 * Finalises a chained 64-bit hash computation and returns the hash.
 *
 * After this call, `*state_p` is NULL and may be reused.
 *
 * @param state_p The opaque hash state.
 * @return The final 64-bit hash value.
 */
static inline uint64_t   mulle_hash_chained_64_final( void **state_p)
{
   return( mulle_hash_chained_64( NULL, 0, state_p));
}


/**
 * Feeds bytes into an ongoing chained hash computation (pointer-width).
 *
 * @param bytes  The byte array to hash (must not be NULL).
 * @param length The length of the byte array.
 * @param state_p The opaque hash state (initialise *state_p to NULL).
 */
static inline void   mulle_hash_chained_add( const void *bytes,
                                             size_t length,
                                             void **state_p)
{
   mulle_hash_chained( bytes, length, state_p);
}


/**
 * Finalises a chained hash computation and returns the hash (pointer-width).
 *
 * After this call, `*state_p` is NULL and may be reused.
 *
 * @param state_p The opaque hash state.
 * @return The final hash value.
 */
static inline uintptr_t   mulle_hash_chained_final( void **state_p)
{
   return( mulle_hash_chained( NULL, 0, state_p));
}



/**
 * Calculates a 32-bit hash value for the given byte array.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the xxHash (XXH32) algorithm to calculate a 32-bit hash value.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The 32-bit hash value.
 */
MULLE__DATA_GLOBAL
uint32_t   _mulle_hash_32( const void *bytes, size_t length);


/**
 * Calculates a 64-bit hash value for the given byte array.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies the xxHash (XXH64) algorithm to calculate a 64-bit hash value.
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The 64-bit hash value.
 */
MULLE__DATA_GLOBAL
uint64_t   _mulle_hash_64( const void *bytes, size_t length);


// -----------------------------------------------------------------------
// Width-independent public API
//
// Use mulle_hash_32 / mulle_hash_64 when you need a hash value with a
// guaranteed fixed width, independent of the platform's pointer size.
// These are suitable for persistence, wire protocols, or any scenario
// where a 32-bit and a 64-bit process must agree on the same hash.
//
// mulle_hash() by contrast returns a pointer-width hash that is optimal
// for in-process hash tables but MUST NOT be persisted or shared across
// processes of different pointer widths.
// -----------------------------------------------------------------------

/**
 * Calculates a 32-bit hash (xxHash XXH32) for the given byte array.
 *
 * This function always produces a 32-bit result regardless of the platform's
 * pointer width. Use this when the hash must be stable across architectures.
 * NULL bytes are treated as "" (hashes the empty string).
 *
 * @param bytes The byte array to hash (may be NULL).
 * @param length The length of the byte array.
 * @return The 32-bit hash value.
 */
static inline uint32_t   mulle_hash_32( const void *bytes, size_t length)
{
   assert( bytes || ! length);
   if( ! bytes)
      bytes = "";
   return( _mulle_hash_32( bytes, length));
}


/**
 * Calculates a 64-bit hash (xxHash XXH64) for the given byte array.
 *
 * This function always produces a 64-bit result regardless of the platform's
 * pointer width. Use this when the hash must be stable across architectures.
 * NULL bytes are treated as "" (hashes the empty string).
 *
 * @param bytes The byte array to hash (may be NULL).
 * @param length The length of the byte array.
 * @return The 64-bit hash value.
 */
static inline uint64_t   mulle_hash_64( const void *bytes, size_t length)
{
   assert( bytes || ! length);
   if( ! bytes)
      bytes = "";
   return( _mulle_hash_64( bytes, length));
}


/**
 * Calculates a pointer-width hash value for the given byte array using xxHash.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies xxHash to calculate a hash value. The size of the returned hash
 * value depends on the size of the `uintptr_t` type on the current platform,
 * being either 32-bit or 64-bit. Result is process-local; do not persist or
 * share across pointer widths.
 *
 * @param bytes The byte array to hash (must not be NULL).
 * @param length The length of the byte array.
 * @return The hash value.
 */
static inline uintptr_t   _mulle_hash( const void *bytes, size_t length)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_32( bytes, length));
   return( (uintptr_t) _mulle_hash_64( bytes, length));
}


/**
 * Calculates a hash value for the given byte array using xxHash.
 *
 * This function takes a pointer to a byte array and the length of the array,
 * and applies xxHash to calculate a hash value. The size of the returned hash
 * value depends on the size of the `uintptr_t` type on the current platform,
 * being either 32-bit or 64-bit.
 *
 * WARNING: The result of mulle_hash() is POINTER-WIDTH DEPENDENT. The same
 * bytes produce DIFFERENT hash values on 32-bit vs 64-bit platforms. Do NOT
 * persist this value to disk, send it over a wire, or share it via IPC/shared
 * memory between processes of different pointer widths. For width-independent
 * hashing, use mulle_hash_32() or mulle_hash_64() instead.
 *
 * If the `bytes` parameter is `NULL`, it is treated as the empty string "".
 * This avoids an in-band sentinel: mulle_hash(NULL, 0) == mulle_hash("", 0).
 *
 * @param bytes The byte array to hash.
 * @param length The length of the byte array.
 * @return The hash value.
 */
static inline uintptr_t   mulle_hash( const void *bytes, size_t length)
{
   assert( bytes || ! length);
   if( ! bytes)
      bytes = "";
   return( _mulle_hash( bytes, length));
}

#endif
