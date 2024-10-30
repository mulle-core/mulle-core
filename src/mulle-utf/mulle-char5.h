//
//  mulle_char5.h
//  mulle-utf
//
//  Created by Nat! on 02.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_char5_h__
#define mulle_char5_h__

#include "mulle-utf-type.h"

#include <assert.h>


//
// char5 is a compression scheme that places small strings
// into 32 bit or 64 bit words
// not all small strings are compatible!
//
// 32 bit can hold up to 6 chars  with 2 bits left over
// 64 bit can hold up to 12 chars with 4 bits left over
//
MULLE__UTF_GLOBAL
int   mulle_char5_encode_character( int c);

// this is usually faster
static inline int   mulle_char5_lookup_character( int c)
{
   extern char   mulle_char5_lookup_table[ 128];

   return( ((size_t) c < 128) ?  mulle_char5_lookup_table[ c] : -1);
}


static inline int   mulle_utf8_is_char5character( char c)
{
   return( mulle_char5_lookup_character( c) >= 0);
}


enum
{
   mulle_char5_maxlength32 = 6,
   mulle_char5_maxlength64 = 12
};


static inline char   *mulle_char5_get_charset( void)
{
   static const char  table[] =
   {
       0,  '.', 'A', 'C',  'D', 'E', 'I', 'N',
      'O', 'P', 'S', 'T',  '_', 'a', 'b', 'c',
      'd', 'e', 'f', 'g',  'h', 'i', 'l', 'm',
      'n', 'o', 'p', 'r',  's', 't', 'u', 'y',
      0  // bonus zero for tests :)
   };
   return( (char *) table);
}


static inline int   mulle_char5_decode_character( int c)
{
   assert( c >= 0 && c < 32);

   return( mulle_char5_get_charset()[ c]);
}


MULLE__UTF_GLOBAL
int   mulle_char5_is_char5string32( char *src, size_t len);
MULLE__UTF_GLOBAL
int   mulle_char5_is_char5string64( char *src, size_t len);

MULLE__UTF_GLOBAL
uint32_t   mulle_char5_encode32( char *src, size_t len);
MULLE__UTF_GLOBAL
uint64_t   mulle_char5_encode64( char *src, size_t len);

MULLE__UTF_GLOBAL
uint32_t   mulle_char5_encode32_utf16( mulle_utf16_t *src, size_t len);
MULLE__UTF_GLOBAL
uint64_t   mulle_char5_encode64_utf16( mulle_utf16_t *src, size_t len);

MULLE__UTF_GLOBAL
uint32_t   mulle_char5_encode32_utf32( mulle_utf32_t *src, size_t len);
MULLE__UTF_GLOBAL
uint64_t   mulle_char5_encode64_utf32( mulle_utf32_t *src, size_t len);

MULLE__UTF_GLOBAL
size_t   mulle_char5_decode32( uint32_t value, char *dst, size_t len);
MULLE__UTF_GLOBAL
size_t   mulle_char5_decode64( uint64_t value, char *src, size_t len);

MULLE__UTF_GLOBAL
int   mulle_char5_get64( uint64_t value, size_t index);
MULLE__UTF_GLOBAL
int   mulle_char5_get32( uint32_t value, size_t index);


static inline int   mulle_char5_next64( uint64_t *value)
{
   int   char5;

   char5    = *value & 0x1F;
   *value >>= 5;

   return( mulle_char5_decode_character( char5));
}


static inline int   mulle_char5_next32( uint32_t *value)
{
   int   char5;

   char5    = *value & 0x1F;
   *value >>= 5;

   return( mulle_char5_decode_character( char5));
}


//
// it's not really clear if strlen or fstrlen is better
//
static inline size_t   mulle_char5_strlen64( uint64_t value)
{
   size_t   len;

   len = 0;
   while( value)
   {
      value >>= 5;
      ++len;
   }
   return( len);
}


static inline size_t  mulle_char5_strlen32( uint32_t value)
{
   size_t   len;

   len = 0;
   while( value)
   {
      value >>= 5;
      ++len;
   }
   return( len);
}


//
// mmmmm.lllll.jjjjj.iiiii.hhhhh.ggggg.ffffff.eeeee.ddddd.ccccc.bbbbb.aaaaa
//
static inline size_t   mulle_char5_fstrlen64( uint64_t value)
{
   uint64_t     mask;
   size_t       len;

   // if any of m.l.j.i.h.g is set, we know f.e.d.c.b.a exist, so len 6 + strlen( m.l.j.i.h.g)
   mask = ~0x3FFFFFFFLL;  // ~ffffff.eeeee.ddddd.ccccc.bbbbb.aaaaa
   len  = 0;
   if( value & (uint64_t) mask)
   {
      len     = 6;
      value >>= 30;  // move fed down do cba
   }
   mask >>= 15;

   if( value & (uint64_t) mask)
   {
      len    += 3;
      value >>= 15;
   }

   mask >>= 10;
   if( value & (uint64_t) mask)
   {
      len    += 2;
      value >>= 10;
   }
   len += (value > 0x1F) ? 2 : (value != 0);
   return( len);
}


//
// ffffff.eeeee.ddddd.ccccc.bbbbb.aaaaa
//
static inline size_t  mulle_char5_fstrlen32( uint32_t value)
{
   uint32_t   mask;
   size_t     len;

   // if any of f.e.d is set, we know c.b.a exist, so len 3 + strlen( ffffff.eeeee.ddddd)
   mask = ~0x7FFF;  // ~ccccc.bbbbb.aaaaa
   len  = 0;
   if( value & (uint32_t) mask)
   {
      len     = 3;
      value >>= 15;  // move fed down do cba
   }
   mask >>= 10;   // same idea for lower 3 bytes, so len 2 + strlen( aaaaa)

   if( value & (uint32_t) mask)
   {
      len    += 2;
      value >>= 10;
   }
   len += (value > 0x1F) ? 2 : (value != 0);
   return( len);
}


static inline uint64_t   mulle_char5_substring64( uint64_t value, size_t location, size_t length)
{
   assert( location + length <= mulle_char5_strlen64( value));

   value >>= location * 5;
   value  &= ~((~(uint64_t) 0) << (length * 5));
   return( value);
}


static inline uint32_t   mulle_char5_substring32( uint32_t value, size_t location, size_t length)
{
   assert( location + length <= mulle_char5_strlen32( value));

   value >>= location * 5;
   value  &= ~((~(uint32_t) 0) << (length * 5));
   return( value);
}


# pragma mark -
# pragma mark uintptr_t interface

static inline int   mulle_char5_is_char5string( char *src, size_t len)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_is_char5string32( src, len));
   return( mulle_char5_is_char5string64( src, len));
}


static inline mulle_char5_t   mulle_char5_encode( char *src, size_t len)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( (mulle_char5_t) mulle_char5_encode32( src, len));
   return( (mulle_char5_t) mulle_char5_encode64( src, len));
}

static inline mulle_char5_t   mulle_char5_encode_utf16( mulle_utf16_t *src, size_t len)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( (mulle_char5_t) mulle_char5_encode32_utf16( src, len));
   return( (mulle_char5_t) mulle_char5_encode64_utf16( src, len));
}


static inline mulle_char5_t   mulle_char5_encode_utf32( mulle_utf32_t *src, size_t len)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( (mulle_char5_t) mulle_char5_encode32_utf32( src, len));
   return( (mulle_char5_t) mulle_char5_encode64_utf32( src, len));
}


static inline size_t   mulle_char5_decode( mulle_char5_t value, char *src, size_t len)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_decode32( (uint32_t) value, src, len));
   return( mulle_char5_decode64( value, src, len));
}


static inline int   mulle_char5_get( mulle_char5_t value, size_t index)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_get32( (uint32_t) value, index));
   return( mulle_char5_get64( value, index));
}

static inline int   mulle_char5_next( mulle_char5_t *value)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_next32( (uint32_t *) value));
   return( mulle_char5_next64( (uint64_t *) value));
}


static inline size_t   mulle_char5_strlen( mulle_char5_t value)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_strlen32( (uint32_t) value));
   return( mulle_char5_strlen64( value));
}


static inline size_t   mulle_char5_fstrlen( mulle_char5_t value)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_fstrlen32( (uint32_t) value));
   return( mulle_char5_fstrlen64( value));
}


static inline size_t  mulle_char5_get_maxlength( void)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( mulle_char5_maxlength32);
   return( mulle_char5_maxlength64);
}


static inline mulle_char5_t  mulle_char5_substring( mulle_char5_t value,
                                                    size_t location,
                                                    size_t length)
{
   if( sizeof( mulle_char5_t) == sizeof( uint32_t))
      return( (mulle_char5_t) mulle_char5_substring32( (uint32_t) value, location, length));
   return( (mulle_char5_t) mulle_char5_substring64( value, location, length));
}


static inline uint32_t   _mulle_char5_fnv1a_32( uint32_t value)
{
   uint32_t    hash;

   /*
    * FNV-1A hash each octet in the buffer
    */
   hash = 0x811c9dc5;
   while( value)
   {
      hash   ^= (uint32_t) mulle_char5_decode_character( value & 0x1F);
      hash   *= 0x01000193;
      value >>= 5;
   }

   return( hash);
}


static inline uint64_t   _mulle_char5_fnv1a_64( uint64_t value)
{
   uint64_t    hash;

   /*
    * FNV-1A hash each octet in the buffer
    */
   hash = 0xcbf29ce484222325ULL;
   while( value)
   {
      hash   ^= (uint64_t) mulle_char5_decode_character( value & 0x1F);
      hash   *= 0x0100000001b3ULL;
      value >>= 5;
   }

   return( hash);
}


static inline uintptr_t   _mulle_char5_fnv1a( uintptr_t value)
{
   if( sizeof( uintptr_t) == sizeof( uint32_t))
      return( (uintptr_t) _mulle_char5_fnv1a_32( value));
   return( (uintptr_t) _mulle_char5_fnv1a_64( value));
}

#endif
