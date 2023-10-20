//
//  mulle_utf32.h
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
#ifndef mulle_utf32_h__
#define mulle_utf32_h__

#include "mulle-utf-type.h"
#include "mulle-char5.h"

#include <stddef.h>


// used in Foundation and maybe here in the future
struct mulle_utf32data
{
   mulle_utf32_t   *characters;
   size_t          length;
};


static inline struct mulle_utf32data   mulle_utf32data_make( mulle_utf32_t *s, size_t length)
{
   struct mulle_utf32data   data;

   data.characters = s;
   data.length     = length;
   return( data);
}


static inline int   mulle_utf32_is_asciicharacter( mulle_utf32_t c)
{
   return( (uint32_t) c < 0x80);
}


static inline int   mulle_utf32_is_char5character( mulle_utf32_t c)
{
   return( mulle_char5_lookup_character( c) >= 0);
}


MULLE__UTF_GLOBAL
size_t   mulle_utf32_utf8length( mulle_utf32_t *src,
                                 size_t len);

MULLE__UTF_GLOBAL
size_t   mulle_utf32_utf16length( mulle_utf32_t *src,
                                  size_t len);

MULLE__UTF_GLOBAL
int   mulle_utf32_information( mulle_utf32_t *src,
                               size_t len,
                               struct mulle_utf_information *info);

MULLE__UTF_GLOBAL
mulle_utf32_t  *mulle_utf32_validate( mulle_utf32_t *src, size_t len);

//
// these two are just here for completeness
//
static inline mulle_utf32_t   _mulle_utf32_next_utf32character( mulle_utf32_t **s_p)
{
   return( *(*s_p)++);
}


static inline mulle_utf32_t   _mulle_utf32_previous_utf32character( mulle_utf32_t **s_p)
{
   return( *--(*s_p));
}


// low level conversion, no checks dst is assumed to be wide enough
// returns end of dst
MULLE__UTF_GLOBAL
mulle_utf16_t  *_mulle_utf32_convert_to_utf16_as_surrogatepair( mulle_utf32_t x,
                                                                mulle_utf16_t *dst);

MULLE__UTF_GLOBAL
mulle_utf16_t   *_mulle_utf32_convert_to_utf16( mulle_utf32_t *src,
                                                size_t len,
                                                mulle_utf16_t *dst);
MULLE__UTF_GLOBAL
char  *_mulle_utf32_convert_to_utf8( mulle_utf32_t *src, size_t len, char *dst);

// these routines do not skip BOM characters
MULLE__UTF_GLOBAL
void   mulle_utf32_bufferconvert_to_utf8( mulle_utf32_t *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t addbytes);

MULLE__UTF_GLOBAL
void   mulle_utf32_bufferconvert_to_utf16( mulle_utf32_t *src,
                                           size_t len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t addbytes);

// unused nowadays
static inline void
   mulle_utf32_bufferconvert_to_utf16_as_surrogatepair(
            mulle_utf32_t x,
            void *buffer,
            mulle_utf_add_bytes_function_t addbytes)
{
   mulle_utf16_t   hilo[ 2];

   _mulle_utf32_convert_to_utf16_as_surrogatepair( x, hilo);

   // (nat) I used to flip those adds around based on endianness, but I think
   // that was wrong
   (*addbytes)( buffer, hilo, sizeof( mulle_utf16_t) * 2);
}



MULLE__UTF_GLOBAL
char   *_mulle_utf32_as_utf8_not_ascii( mulle_utf32_t x, char *dst);

// no error checks whatsoever
// returns end of `dst`
static inline char   *mulle_utf32_as_utf8( mulle_utf32_t x, char *dst)
{
   if( (uint32_t) x < 0x80)
   {
      *dst++ = (char) x;
      return( dst);
   }
   return( _mulle_utf32_as_utf8_not_ascii( x, dst));
}



MULLE__UTF_GLOBAL
enum mulle_utf_charinfo   _mulle_utf32_charinfo( mulle_utf32_t *src, size_t len);

#endif

