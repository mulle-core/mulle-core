//
//  mulle_char7.h
//  mulle-utf
//
//  Created by Nat! on 24.07.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_char7_h__
#define mulle_char7_h__

#include "mulle-utf-type.h"

#include <assert.h>

//
// char7 is a scheme that places small 7 bit ASCII strings
// into 32 bit or 64 bit words
//
// 32 bit can hold up to 4 chars, with 4 bits left over
// 64 bit can hold up to 8 chars, with 8 bits left over
// since we need 2-3 bits space for TPS info, the 64 bit is really only 61 bit
enum
{
   mulle_char7_maxlength32 = 4,
   mulle_char7_maxlength64 = 8
};


MULLE_UTF_GLOBAL
int   mulle_char7_is_char7string32( char *src, size_t len);
MULLE_UTF_GLOBAL
int   mulle_char7_is_char7string64( char *src, size_t len);

MULLE_UTF_GLOBAL
uint32_t   mulle_char7_encode32_utf16( mulle_utf16_t *src, size_t len);
MULLE_UTF_GLOBAL
uint64_t   mulle_char7_encode64_utf16( mulle_utf16_t *src, size_t len);

MULLE_UTF_GLOBAL
uint32_t   mulle_char7_encode32_utf32( mulle_utf32_t *src, size_t len);
MULLE_UTF_GLOBAL
uint64_t   mulle_char7_encode64_utf32( mulle_utf32_t *src, size_t len);

MULLE_UTF_GLOBAL
uint32_t   mulle_char7_encode32( char *src, size_t len);
MULLE_UTF_GLOBAL
uint64_t   mulle_char7_encode64( char *src, size_t len);

MULLE_UTF_GLOBAL
size_t   mulle_char7_decode32( uint32_t value, char *dst, size_t len);
MULLE_UTF_GLOBAL
size_t   mulle_char7_decode64( uint64_t value, char *src, size_t len);

MULLE_UTF_GLOBAL
int   mulle_char7_get64( uint64_t value, unsigned int index);
MULLE_UTF_GLOBAL
int   mulle_char7_get32( uint32_t value, unsigned int index);


static inline int  mulle_char7_next64( uint64_t *value)
{
   int   char7;

   char7    = *value & 0x7F;
   *value >>= 7;

   return( char7);
}


static inline int  mulle_char7_next32( uint32_t *value)
{
   int   char7;

   char7    = *value & 0x7F;
   *value >>= 7;

   return( char7);
}


static inline size_t   mulle_char7_strlen64( uint64_t value)
{
   size_t   len;

   len = 0;
   while( value)
   {
      value >>= 7;
      ++len;
   }
   return( len);
}


//
// 0xfe00000   0x1fC000   0x3F80      0x007F
// |||||||     |||||||    |||||||    |||||||
// \     /     \     /    \     /    \     /
//    OR     +   OR     +    OR   +     OR
// Would be simple to do in hardware
//
static inline size_t  mulle_char7_strlen32( uint32_t value)
{
   size_t   len;

   len = 0;
   while( value)
   {
      value >>= 7;
      ++len;
   }
   return( len);
}


//
// hhhhhhh.ggggggg.fffffff.eeeeeee.ddddddd.ccccccc.bbbbbbb.aaaaaaa
//
static inline size_t   mulle_char7_fstrlen64( uint64_t value)
{
   int64_t   mask;
   size_t    len;

   mask = ~0xFFFFFFFLL;
   len  = 0;
   if( value & (uint64_t) mask)
   {
      len     = 4;
      value >>= 28;
   }
   mask >>= 14;

   if( value & (uint64_t) mask)
   {
      len    += 2;
      value >>= 14;
   }

   len += (value > 0x7F) ? 2 : (value != 0);
   return( len);
}


//
// ddddddd.ccccccc.bbbbbbb.aaaaaaa
//                 11.1111.1111.1111
//
static inline size_t   mulle_char7_fstrlen32( uint32_t value)
{
   int32_t   mask;
   size_t    len;

   mask = ~0x3FFF;
   len  = 0;
   if( value & (uint64_t) mask)
   {
      len     = 2;
      value >>= 14;
   }

   len += (value > 0x7F) ? 2 : (value != 0);
   return( len);
}


static inline uint64_t   mulle_char7_substring64( uint64_t value, unsigned int location, unsigned int length)
{
   assert( location + length <= mulle_char7_strlen64( value));

   value >>= location * 7;
   value  &= ~((~(uint64_t) 0) << (length * 7));
   return( value);
}


static inline uint32_t   mulle_char7_substring32( uint32_t value, unsigned int location, unsigned int length)
{
   assert( location + length <= mulle_char7_strlen32( value));

   value >>= location * 7;
   value  &= ~((~(uint32_t) 0) << (length * 7));
   return( value);
}


# pragma mark - uintptr_t interface

static inline int   mulle_char7_is_char7string( char *src, size_t len)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( mulle_char7_is_char7string32( src, len));
   return( mulle_char7_is_char7string64( src, len));
}


static inline mulle_char7_t   mulle_char7_encode( char *src, size_t len)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( (mulle_char7_t) mulle_char7_encode32( src, len));
   return( (mulle_char7_t) mulle_char7_encode64( src, len));
}


static inline mulle_char7_t   mulle_char7_encode_utf16( mulle_utf16_t *src, size_t len)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( (mulle_char7_t) mulle_char7_encode32_utf16( src, len));
   return( (mulle_char7_t) mulle_char7_encode64_utf16( src, len));
}


static inline mulle_char7_t   mulle_char7_encode_utf32( mulle_utf32_t *src, size_t len)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( (mulle_char7_t) mulle_char7_encode32_utf32( src, len));
   return( (mulle_char7_t) mulle_char7_encode64_utf32( src, len));
}


static inline size_t   mulle_char7_decode( mulle_char7_t value, char *src, size_t len)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_decode32( (uint32_t) value, src, len));
   return( mulle_char7_decode64( value, src, len));
}


static inline int   mulle_char7_next( mulle_char7_t *value)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_next32( (uint32_t *) value));
   return( mulle_char7_next64( (uint64_t *) value));
}


static inline int   mulle_char7_get( mulle_char7_t value, unsigned int index)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_get32( (uint32_t) value, index));
   return( mulle_char7_get64( value, index));
}


static inline size_t  mulle_char7_strlen( mulle_char7_t value)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_strlen32( (uint32_t) value));
   return( mulle_char7_strlen64( value));
}


static inline size_t  mulle_char7_fstrlen( mulle_char7_t value)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_fstrlen32( (uint32_t) value));
   return( mulle_char7_fstrlen64( value));
}


static inline size_t  mulle_char7_get_maxlength( void)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( mulle_char7_maxlength32);
   return( mulle_char7_maxlength64);
}


static inline mulle_char7_t  mulle_char7_substring( mulle_char7_t value,
                                                    unsigned int location,
                                                    unsigned int length)
{
   if( sizeof( mulle_char7_t) == sizeof( uint32_t))
      return( (mulle_char7_t) mulle_char7_substring32( (uint32_t) value, location, length));
   return( (mulle_char7_t) mulle_char7_substring64( value, location, length));
}


static inline uint32_t   _mulle_char7_fnv1a_32( uint32_t value)
{
   uint32_t    hash;

   /*
    * FNV-1A hash each octet in the buffer
    */
   hash = 0x811c9dc5;
   while( value)
   {
      hash   ^= value & 0x7F;
      hash   *= 0x01000193;
      value >>= 7;
   }

   return( hash);
}


static inline uint64_t   _mulle_char7_fnv1a_64( uint64_t value)
{
   uint64_t    hash;

   /*
    * FNV-1A hash each octet in the buffer
    */
   hash = 0xcbf29ce484222325ULL;
   while( value)
   {
      hash   ^= value & 0x7F;
      hash   *= 0x0100000001b3ULL;
      value >>= 7;
   }

   return( hash);
}


static inline uintptr_t   _mulle_char7_fnv1a( uintptr_t value)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_char7_fnv1a_32( value));
   return( (uintptr_t) _mulle_char7_fnv1a_64( value));
}

#endif

