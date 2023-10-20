//
//  mulle_utf_noncharacter.h
//  mulle-utf
//
//  Created by Nat! on 15.04.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_utf_noncharacter_h__
#define mulle_utf_noncharacter_h__

#include "mulle-utf-type.h"
#include <assert.h>


MULLE__UTF_GLOBAL
int   mulle_utf16_is_noncharacter( mulle_utf16_t c);
MULLE__UTF_GLOBAL
int   mulle_utf32_is_noncharacter( mulle_utf32_t c);
MULLE__UTF_GLOBAL
int   mulle_utf_is_noncharacterplane( unsigned int plane);


// some more assorted funcions


static inline mulle_utf32_t  mulle_utf32_get_bomcharacter( void)
{
   return( 0xFEFF);  // only native encoding so far...
}


static inline mulle_utf16_t  mulle_utf16_get_bomcharacter( void)
{
   return( 0xFEFF);  // only native encoding so far...
}


static inline int  mulle_utf32_is_bomcharacter( mulle_utf32_t c)
{
   return( c == mulle_utf32_get_bomcharacter());  // only native encoding so far...
}


static inline int  mulle_utf16_is_bomcharacter( mulle_utf16_t c)
{
   return( c == mulle_utf16_get_bomcharacter());  // only native encoding so far...
}


static inline int   mulle_utf32_is_surrogatecharacter( mulle_utf32_t c)
{
   return( c >= 0xD800 && c <= 0xE000);
}


static inline int   mulle_utf32_is_highsurrogatecharacter( mulle_utf32_t c)
{
   return( c >= 0xD800 && c < 0xDC00);
}


static inline int   mulle_utf32_is_lowsurrogatecharacter( mulle_utf32_t c)
{
   return( c >= 0xDC00 && c < 0xE000);
}


// somewhat arbitrary
static inline int   mulle_utf32_is_invalidcharacter( mulle_utf32_t c)
{
   return( mulle_utf32_is_noncharacter( c));   // e.g. utf-16 surrogate pair
}


static inline void  mulle_utf32_encode_surrogatepair( mulle_utf32_t x,
                                                      mulle_utf16_t *hi,
                                                      mulle_utf16_t *lo)
{
   mulle_utf16_t  top;
   mulle_utf16_t  bottom;

   assert( x >= 0x10000 && x <= 0x10FFFF);

   x -= 0x10000;

   assert( (x >> 10) <= 0x3FF);

   top    = (mulle_utf16_t) (x >> 10);
   bottom = (mulle_utf16_t) (x & 0x3FF);

   *hi = 0xD800 + top;
   *lo = 0xDC00 + bottom;

   assert( *hi >= 0xD800 && *hi < 0xDC00);
   assert( *lo >= 0xDC00 && *lo < 0xE000);
}


static inline mulle_utf32_t  mulle_utf16_decode_surrogatepair( mulle_utf16_t hi,
                                                               mulle_utf16_t lo)
{
   mulle_utf32_t   top;
   mulle_utf32_t   bottom;

   assert( mulle_utf32_is_highsurrogatecharacter( hi));
   assert( mulle_utf32_is_lowsurrogatecharacter( lo));

   top    = (mulle_utf32_t) (hi - 0xD800);
   bottom = (mulle_utf32_t) (lo - 0xDC00);

   return( 0x10000 + (top << 10) + bottom);
}


#endif /* mulle_utf_is_alphanumeric_h */
