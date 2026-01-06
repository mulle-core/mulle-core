#ifndef mulle_data_h__
#define mulle_data_h__

#include "include.h"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>


/*
 *  (c) 2020 nat <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE__DATA_VERSION  ((0UL << 20) | (5 << 8) | 0)


static inline unsigned int   mulle_data_get_version_major( void)
{
   return( MULLE__DATA_VERSION >> 20);
}


static inline unsigned int   mulle_data_get_version_minor( void)
{
   return( (MULLE__DATA_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_data_get_version_patch( void)
{
   return( MULLE__DATA_VERSION & 0xFF);
}


MULLE__DATA_GLOBAL
uint32_t   mulle_data_get_version( void);


//
// Since length is the max object size according to POSIX, this
// sounds like the sane type to use for length, but then an object isn't
// the possible address space...
//
// Not yet used by mulle-buffer, but will eventually
//
struct mulle_data
{
   void     *bytes;
   size_t   length;
};


/**
 * Creates a new `mulle_data` struct with the given bytes and length.
 *
 * @param bytes The bytes to store in the `mulle_data` struct.
 * @param length The length of the bytes to store.
 * @return A new `mulle_data` struct with the given bytes and length.
 */
static inline struct mulle_data   mulle_data_make( void *bytes, size_t length)
{
   struct mulle_data   data;

   assert( ! length || bytes);

   data.bytes  = bytes;
   data.length = length;
   return( data);
}



/**
 * Creates a new `mulle_data` struct with an empty string and a length of 0.
 *
 * @return A new `mulle_data` struct with an empty string and a length of 0.
 */
static inline struct mulle_data   mulle_data_make_empty( void)
{
   struct mulle_data   data;

   data.bytes  = "";  // assume linker can unify strings, otherwise use global
   data.length = 0;
   return( data);
}


/**
 * Creates a new `mulle_data` struct with invalid bytes and a length of 0.
 *
 * @return A new `mulle_data` struct with invalid bytes and a length of 0.
 */
static inline struct mulle_data   mulle_data_make_invalid( void)
{
   struct mulle_data   data;

   data.bytes  = NULL;
   data.length = 0;
   return( data);
}


/**
 * Checks if the given `mulle_data` struct has an empty string.
 *
 * @param data The `mulle_data` struct to check.
 * @return `1` if the `mulle_data` struct has an empty string, `0` otherwise.
 */
static inline int   mulle_data_is_empty( struct mulle_data data)
{
   return( data.length == 0);
}


/**
 * Checks if the given `mulle_data` struct has invalid bytes.
 *
 * @param data The `mulle_data` struct to check.
 * @return `1` if the `mulle_data` struct has invalid bytes, `0` otherwise.
 */
static inline int   mulle_data_is_invalid( struct mulle_data data)
{
   return( data.bytes == NULL);
}


/**
 * Asserts that the `mulle_data` struct has a valid byte pointer if the length is non-zero.
 *
 * @param data The `mulle_data` struct to assert.
 */
static inline void   mulle_data_assert( struct mulle_data data)
{
   assert( ! data.length || data.bytes);

   MULLE_C_UNUSED( data);
}


// our "standard" hash for bytes, assuming them to be largish
#include "mulle-hash.h"

// fnva1 hash for small strings especially ObjC selectors
#include "mulle-fnv1a.h"

// prime code for hashtables
#include "mulle-prime.h"

// range is convenient for mulle_data and will be used more in the future
#include "mulle-range.h"

// custom qsort implementation for cross platform API sameness
#include "mulle-qsort.h"

/**
 * Computes a hash value for the given `mulle_data` struct.
 *
 * This function computes a hash value for the byte data contained in the `mulle_data` struct. The hash is computed using either a 32-bit or 64-bit hash function, depending on the size of the `uintptr_t` type.
 *
 * @param data The `mulle_data` struct to compute the hash for.
 * @return The computed hash value.
 */
static inline uintptr_t   mulle_data_hash( struct mulle_data data)
{
   mulle_data_assert( data);

   return( (uintptr_t) mulle_hash( data.bytes, data.length));
}


/**
 * Computes a chained hash value for the given `mulle_data` struct.
 *
 * This function computes a chained hash value for the byte data contained in the `mulle_data` struct. The hash is computed using either a 32-bit or 64-bit hash function, depending on the size of the `uintptr_t` type. The hash value is chained with the provided `hash` parameter.
 *
 * @param data The `mulle_data` struct to compute the hash for.
 * @param hash The initial hash value to chain with.
 * @return The computed chained hash value.
 */
static inline uintptr_t   mulle_data_hash_chained( struct mulle_data data, void **state_p)
{
   mulle_data_assert( data);

   return( mulle_hash_chained( data.bytes, data.length, state_p));
}


/**
 * Creates a new `mulle_data` struct that represents a subrange of the given `mulle_data` struct.
 *
 * This function creates a new `mulle_data` struct that represents a subrange of the given `mulle_data` struct. The subrange is specified by the provided `mulle_range` struct, which contains the location and length of the subrange.
 *
 * If the specified subrange is valid (i.e., it is within the bounds of the original `mulle_data` struct), the function returns a new `mulle_data` struct that represents the subrange. If the subrange is not valid, the function returns an invalid `mulle_data` struct.
 *
 * @param data The `mulle_data` struct to create the subrange from.
 * @param range The `mulle_range` struct that specifies the subrange to create.
 * @return A new `mulle_data` struct that represents the specified subrange, or an invalid `mulle_data` struct if the subrange is not valid.
 */
static inline struct mulle_data   mulle_data_subdata( struct mulle_data data,
                                                      struct mulle_range range)
{
   mulle_data_assert( data);

   range = mulle_range_validate_against_length( range, data.length);
   if( mulle_range_is_valid( range))
      return( mulle_data_make( &((char *) data.bytes)[ range.location], range.length));
   return( mulle_data_make_invalid());
}


/**
 * Searches for the given `needle` data within the `haystack` data.
 *
 * This function searches for the `needle` data within the `haystack` data and returns a pointer to the first occurrence of the `needle` data within the `haystack` data, or `NULL` if the `needle` data is not found.
 *
 * @param haystack The `mulle_data` struct containing the data to search.
 * @param needle The `mulle_data` struct containing the data to search for.
 * @return A pointer to the first occurrence of the `needle` data within the `haystack` data, or `NULL` if the `needle` data is not found.
 */
MULLE__DATA_GLOBAL
void   *mulle_data_search_data( struct mulle_data haystack,
                                struct mulle_data needle);


#ifdef __has_include
# if __has_include( "_mulle-data-versioncheck.h")
#  include "_mulle-data-versioncheck.h"
# endif
#endif

#endif
