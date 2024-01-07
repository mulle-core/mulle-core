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
#define MULLE__DATA_VERSION  ((0UL << 20) | (3 << 8) | 0)


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


static inline struct mulle_data   mulle_data_make( void *bytes, size_t length)
{
   struct mulle_data   data;

   assert( ! length || bytes);

   data.bytes  = bytes;
   data.length = length;
   return( data);
}



static inline struct mulle_data   mulle_data_make_empty( void)
{
   struct mulle_data   data;

   data.bytes  = "";  // assume linker can unify strings, otherwise use global
   data.length = 0;
   return( data);
}


static inline struct mulle_data   mulle_data_make_invalid( void)
{
   struct mulle_data   data;

   data.bytes  = NULL;
   data.length = 0;
   return( data);
}


static inline int   mulle_data_is_empty( struct mulle_data data)
{
   return( data.length == 0);
}


static inline int   mulle_data_is_invalid( struct mulle_data data)
{
   return( data.bytes == NULL);
}


static inline void   mulle_data_assert( struct mulle_data data)
{
   assert( ! data.length || data.bytes);
}


// our "standard" hash for bytes, assuming them to be largish
#include "mulle-hash.h"

// fnva1 hash for small strings especially ObjC selectors
#include "mulle-fnv1a.h"

// prime code for hashtables
#include "mulle-prime.h"

// range is convenient for mulle_data and will be used more in the future
#include "mulle-range.h"

static inline uintptr_t   mulle_data_hash( struct mulle_data data)
{
   mulle_data_assert( data);

   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_32( data.bytes, data.length));
   return( (uintptr_t) _mulle_hash_64( data.bytes, data.length));
}


static inline uintptr_t   mulle_data_hash_chained( struct mulle_data data, uintptr_t hash)
{
   mulle_data_assert( data);

   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_hash_chained_32( data.bytes, data.length, (uint32_t) hash));
   return( (uintptr_t) _mulle_hash_chained_64( data.bytes, data.length, (uint64_t) hash));
}


static inline struct mulle_data   mulle_data_subdata( struct mulle_data data,
                                                      struct mulle_range range)
{
   mulle_data_assert( data);

   range = mulle_range_validate_against_length( range, data.length);
   if( mulle_range_is_valid( range))
      return( mulle_data_make( &((char *) data.bytes)[ range.location], range.length));
   return( mulle_data_make_invalid());
}


MULLE__DATA_GLOBAL
void   *mulle_data_search_data( struct mulle_data haystack,
                                struct mulle_data needle);


#ifdef __has_include
# if __has_include( "_mulle-data-versioncheck.h")
#  include "_mulle-data-versioncheck.h"
# endif
#endif

#endif
