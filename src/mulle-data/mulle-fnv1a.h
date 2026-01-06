//
//  mulle_objc_fnv1.h
//  mulle-objc-runtime
//
//  Created by Nat! on 19.08.17
//

/***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *      chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!     :-)
 */

#ifndef mulle_fnv1a_h__
#define mulle_fnv1a_h__

/* renamed to mulle_ to keep symbols clean
 * It's an implementation of the [FNV1 hash](//en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash).
 * The 32 bit value is in no form and shape compatible with the
 * 64 bit value.
 * Investigate reading 64 bits instead of just 8.
 */
#include "include.h"

#include <stdint.h>
#include <stddef.h>

#define MULLE_FNV1A_32_INIT    0x811c9dc5
#define MULLE_FNV1A_64_INIT    0xcbf29ce484222325ULL


/**
 * Initializes a 32-bit FNV-1a hash value.
 *
 * @return The initial 32-bit FNV-1a hash value.
 */
static inline uint32_t _mulle_fnv1a_init_32(void)
{
    return MULLE_FNV1A_32_INIT;
}

/**
 * Initializes a 64-bit FNV-1a hash value.
 *
 * @return The initial 64-bit FNV-1a hash value.
 */
static inline uint64_t _mulle_fnv1a_init_64(void)
{
    return MULLE_FNV1A_64_INIT;
}

/**
 * Initializes an FNV-1a hash value with the appropriate size (32-bit or 64-bit)
 * based on the size of `uintptr_t`.
 *
 * @return The initial FNV-1a hash value.
 */
static inline uintptr_t _mulle_fnv1a_init(void)
{
    if (sizeof(uintptr_t) == sizeof(uint32_t))
        return (uintptr_t)_mulle_fnv1a_init_32();
    return (uintptr_t)_mulle_fnv1a_init_64();
}

#define MULLE_FNV1A_32_PRIME   0x01000193
#define MULLE_FNV1A_64_PRIME   0x0100000001b3ULL


/**
 * Performs a single step of the FNV-1a hash algorithm for a 32-bit hash value.
 *
 * @param hash The current 32-bit hash value.
 * @param value The byte value to incorporate into the hash.
 * @return The updated 32-bit hash value.
 */
static inline uint32_t
   _mulle_fnv1a_step_32( uint32_t hash, unsigned char value)
{
   hash ^= value;
   hash *= MULLE_FNV1A_32_PRIME;
   return( hash);
}


/**
 * Performs a single step of the FNV-1a hash algorithm for a 64-bit hash value.
 *
 * @param hash The current 64-bit hash value.
 * @param value The byte value to incorporate into the hash.
 * @return The updated 64-bit hash value.
 */
static inline uint64_t
   _mulle_fnv1a_step_64( uint64_t hash, unsigned char value)
{
   hash ^= value;
   hash *= MULLE_FNV1A_64_PRIME;
   return( hash);
}


/**
 * Performs a single step of the FNV-1a hash algorithm using the appropriate
 * 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param hash The current hash value.
 * @param value The byte value to incorporate into the hash.
 * @return The updated hash value.
 */
static inline uintptr_t   _mulle_fnv1a_step( uintptr_t hash, unsigned char value)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_fnv1a_step_32( hash, value));
   return( (uintptr_t) _mulle_fnv1a_step_64( hash, value));
}


/**
 * Performs a single step of the FNV-1a hash algorithm for a 32-bit hash value.
 *
 * @param hash The current 32-bit hash value.
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The updated 32-bit hash value.
 */
MULLE__DATA_GLOBAL
uint32_t   _mulle_fnv1a_chained_32( void *buf, size_t len, uint32_t hash);

/**
 * Performs a chained FNV-1a hash algorithm for a 64-bit hash value.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @param hash The initial 64-bit hash value.
 * @return The updated 64-bit hash value.
 */
MULLE__DATA_GLOBAL
uint64_t   _mulle_fnv1a_chained_64( void *buf, size_t len, uint64_t hash);


/**
 * Performs a single step of the FNV-1a hash algorithm for a 32-bit hash value.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The 32-bit hash value.
 */
static inline uint32_t   _mulle_fnv1a_32( void *buf, size_t len)
{
   return( _mulle_fnv1a_chained_32( buf, len, MULLE_FNV1A_32_INIT));
}


/**
 * Performs a 64-bit FNV-1a hash on the given buffer.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The 64-bit hash value.
 */
static inline uint64_t   _mulle_fnv1a_64( void *buf, size_t len)
{
   return( _mulle_fnv1a_chained_64( buf, len, MULLE_FNV1A_64_INIT));
}


/**
 * Performs an FNV-1a hash on the given buffer, using the appropriate 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The hashed value as a `uintptr_t`.
 */
static inline uintptr_t   _mulle_fnv1a( void *buf, size_t len)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_fnv1a_32( buf, len));
   return( (uintptr_t) _mulle_fnv1a_64( buf, len));
}


/**
 * Performs an FNV-1a hash on the given buffer, using the appropriate 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @param hash The initial hash value.
 * @return The updated hash value.
 */
static inline uintptr_t   _mulle_fnv1a_chained( void *buf,
                                                size_t len,
                                                uintptr_t hash)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_fnv1a_chained_32( buf, len, (uint32_t) hash));
   return( (uintptr_t) _mulle_fnv1a_chained_64( buf, len, (uint64_t) hash));
}



/**
 * Performs a 32-bit FNV-1a hash on the given buffer, chaining the hash value.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @param hash The initial hash value.
 * @return The updated hash value.
 */
// unfortunately can't put it into a switch statement label
static inline uint32_t
   _mulle_fnv1a_chained_32_inline( void *buf, size_t len, uint32_t hash)
{
   unsigned char   *s;
   unsigned char   *sentinel;

   s        = buf;
   sentinel = &s[ len];

   /*
    * FNV-1A hash each octet in the buffer
    */
   while( s < sentinel)
   {
      hash ^= (uint32_t) *s++;
      hash *= MULLE_FNV1A_32_PRIME;
   }

   return( hash);
}


/**
 * Performs a 64-bit FNV-1a hash on the given buffer, chaining the hash value.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @param hash The initial hash value.
 * @return The updated hash value.
 */
static inline uint64_t
   _mulle_fnv1a_chained_64_inline( void *buf, size_t len, uint64_t hash)
{
   unsigned char   *s;
   unsigned char   *sentinel;

   s        = buf;
   sentinel = &s[ len];

   /*
    * FNV-1 hash each octet in the buffer
    */
   while( s < sentinel)
   {
      hash ^= (uint64_t) *s++;
      hash *= MULLE_FNV1A_64_PRIME;
   }

   return( hash);
}


/**
 * Performs a 32-bit FNV-1a hash on the given buffer.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The hash value.
 */
static inline uint32_t   _mulle_fnv1a_32_inline( void *buf, size_t len)
{
   return( _mulle_fnv1a_chained_32_inline( buf, len, MULLE_FNV1A_32_INIT));
}


/**
 * Performs a 64-bit FNV-1a hash on the given buffer.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The hash value.
 */
static inline uint64_t   _mulle_fnv1a_64_inline( void *buf, size_t len)
{
   return( _mulle_fnv1a_chained_64_inline( buf, len, MULLE_FNV1A_64_INIT));
}


/**
 * Performs an FNV-1a hash on the given buffer, using the appropriate 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @return The hash value as an `uintptr_t`.
 */
static inline uintptr_t   _mulle_fnv1a_inline( void *buf, size_t len)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_fnv1a_32_inline( buf, len));
   return( (uintptr_t) _mulle_fnv1a_64_inline( buf, len));
}


/**
 * Performs an FNV-1a hash on the given buffer, using the appropriate 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param buf The buffer to hash.
 * @param len The length of the buffer.
 * @param hash The initial hash value to use.
 * @return The hash value as an `uintptr_t`.
 */
static inline uintptr_t   _mulle_fnv1a_chained_inline( void *buf,
                                                       size_t len,
                                                       uintptr_t hash)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_fnv1a_chained_32_inline( buf, len, (uint32_t) hash));
   return( (uintptr_t) _mulle_fnv1a_chained_64_inline( buf, len, (uint64_t) hash));
}


/**
 * Performs a 32-bit FNV-1a hash on the given string.
 *
 * @param s The string to hash.
 * @return The hash value as a 32-bit unsigned integer.
 */
MULLE__DATA_GLOBAL
uint32_t _mulle_string_hash_32(char *s);

/**
 * Performs a 64-bit FNV-1a hash on the given string.
 *
 * @param s The string to hash.
 * @return The hash value as a 64-bit unsigned integer.
 */
MULLE__DATA_GLOBAL
uint64_t _mulle_string_hash_64(char *s);



/**
 * Performs an FNV-1a hash on the given string, using the appropriate 32-bit or 64-bit implementation based on the size of `uintptr_t`.
 *
 * @param s The string to hash.
 * @return The hash value as an `uintptr_t`.
 */
static inline uintptr_t   _mulle_string_hash( char *s)
{
   if( sizeof( uintptr_t) == sizeof( uint64_t))
      return( (uintptr_t) _mulle_string_hash_64( s));
   return( (uintptr_t) _mulle_string_hash_32( s));
}


/**
 * Performs an FNV-1a hash on the given string, chaining the hash value.
 *
 * @param s The string to hash.
 * @param hash The initial hash value to use.
 * @return The hash value as a 32-bit unsigned integer.
 */
MULLE__DATA_GLOBAL
uint32_t _mulle_string_hash_chained_32(char *s, uint32_t hash);

/**
 * Performs an FNV-1a hash on the given string, chaining the hash value.
 *
 * @param s The string to hash.
 * @param hash The initial hash value to use.
 * @return The hash value as a 64-bit unsigned integer.
 */
MULLE__DATA_GLOBAL
uint64_t _mulle_string_hash_chained_64(char *s, uint64_t hash);



/**
 * Performs an FNV-1a hash on the given string, chaining the hash value.
 *
 * @param s The string to hash.
 * @param hash The initial hash value to use.
 * @return The hash value as an `uintptr_t`.
 */
static inline uintptr_t   _mulle_string_hash_chained( char *s, uintptr_t hash)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_string_hash_chained_32( s, (uint32_t) hash));
   return( (uintptr_t) _mulle_string_hash_chained_64( s, (uint64_t) hash));
}

#endif /* mulle _objc_fnv1a_h */
