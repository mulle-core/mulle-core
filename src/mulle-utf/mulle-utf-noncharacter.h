//
//  mulle-utf-noncharacter.h
//  mulle-utf
//
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#ifndef mulle_utf_noncharacter_h__
#define mulle_utf_noncharacter_h__

#include "mulle-utf-type.h"
#include <assert.h>


MULLE__UTF_GLOBAL
int   mulle_utf16_is_noncharacter( mulle_utf16_t c);
MULLE__UTF_GLOBAL
int   mulle_utf32_is_noncharacter( mulle_utf32_t c);
MULLE__UTF_GLOBAL
int   mulle_utf_is_noncharacterplane( size_t plane);


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
