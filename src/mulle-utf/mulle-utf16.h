//
//  mulle_utf16.h
//  mulle-utf
//
//  Copyright (C) 2011 Nat!, Mulle kybernetiK.
//  Copyright (c) 2011 Codeon GmbH.
//  All rights reserved.
//
//  Coded by Nat!
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

#ifndef mulle_utf16_h__
#define mulle_utf16_h__

#include "mulle-utf-type.h"
#include "mulle-char5.h"

#include <assert.h>
#include <stddef.h>


// used in Foundation and maybe here in the future
struct mulle_utf16data
{
   mulle_utf16_t   *characters;
   size_t          length;
};


static inline size_t   mulle_utf16_strlen( mulle_utf16_t *s)
{
   mulle_utf16_t   *p;

   if( ! s)
      return( 0);

   for( p = s - 1; *++p;);

   return( p - s);
}


static inline struct mulle_utf16data   mulle_utf16data_make( mulle_utf16_t *s,
                                                             size_t length)
{
   struct mulle_utf16data      data;
   const static mulle_utf16_t  zero = { 0 };

   data.length     = (length == (size_t) -1) ? mulle_utf16_strlen( s) : length;
   data.characters = data.length ? s : (mulle_utf16_t *) &zero;
   return( data);
}


static inline int   mulle_utf16_is_asciicharacter( mulle_utf16_t c)
{
   return( c < 0x80);
}


static inline int   mulle_utf16_is_char5character( mulle_utf16_t c)
{
   return( mulle_char5_lookup_character( c) >= 0);
}


MULLE__UTF_GLOBAL
int     mulle_utf16_information( mulle_utf16_t *src, size_t len, struct mulle_utf_information *info);

MULLE__UTF_GLOBAL
size_t  mulle_utf16_utf8length( mulle_utf16_t *src, size_t len);

MULLE__UTF_GLOBAL
size_t  mulle_utf16_utf32length( mulle_utf16_t *src, size_t len);

MULLE__UTF_GLOBAL
size_t  mulle_utf16_length( mulle_utf16_t *src, size_t len);

MULLE__UTF_GLOBAL
int   mulle_utf16_contains_character_larger_or_equal( mulle_utf16_t *src,
                                                      size_t len,
                                                      mulle_utf16_t d);

static inline int   mulle_utf16_is_ascii( mulle_utf16_t *src, size_t len)
{
   return( ! mulle_utf16_contains_character_larger_or_equal( src, len, 0x80));
}


static inline int   mulle_utf16_is_utf15( mulle_utf16_t *src, size_t len)
{
   return( ! mulle_utf16_contains_character_larger_or_equal( src, len, 0x8000));
}


static inline size_t  mulle_utf16_utf8maxlength( size_t len)
{
   return( len * 4);
}

MULLE__UTF_GLOBAL
mulle_utf16_t  *mulle_utf16_validate( mulle_utf16_t *src, size_t len);

// hi and lo MUST be surrogates
MULLE__UTF_GLOBAL
int  mulle_utf16_is_valid_surrogatepair( mulle_utf16_t hi, mulle_utf16_t lo);

// step through string characters
MULLE__UTF_GLOBAL
mulle_utf32_t   _mulle_utf16_next_utf32character( mulle_utf16_t **s_p);

MULLE__UTF_GLOBAL
mulle_utf32_t   _mulle_utf16_previous_utf32character( mulle_utf16_t **s_p);


// low level conversion, no checks dst is assumed to be wide enough
// returns end of dst
MULLE__UTF_GLOBAL
mulle_utf32_t  *_mulle_utf16_convert_to_utf32( mulle_utf16_t *src,
                                               size_t len,
                                               mulle_utf32_t *dst);
MULLE__UTF_GLOBAL
char  *_mulle_utf16_convert_to_utf8( mulle_utf16_t *src,
                                     size_t len,
                                     char *dst);
//
// This will not stop on a zero. It will not by itself append a zero.
// return value = 0  means OK!
// these routines do not skip BOM characters
//
MULLE__UTF_GLOBAL
void   mulle_utf16_bufferconvert_to_utf8( mulle_utf16_t *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t *addbytes);

MULLE__UTF_GLOBAL
void   mulle_utf16_bufferconvert_to_utf32( mulle_utf16_t *src,
                                           size_t len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t *addbytes);

MULLE__UTF_GLOBAL
enum mulle_utf_charinfo   _mulle_utf16_charinfo( mulle_utf16_t *src, size_t len);


// would not work, because we need two mulle_utf16_taa
// MULLE__UTF_GLOBAL
// char   *_mulle_utf16_as_utf8_not_ascii( mulle_utf16_t x, char *dst);


#endif
