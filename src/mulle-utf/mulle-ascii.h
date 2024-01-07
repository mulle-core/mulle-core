//
//  mulle_utf8.h
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
#ifndef mulle_ascii_h__
#define mulle_ascii_h__

#include "mulle-utf-type.h"

#include <string.h>


// used in Foundation and maybe here in the future
struct mulle_asciidata
{
   char     *characters;
   size_t   length;
};


static inline struct mulle_asciidata
   mulle_asciidata_make( char *s, size_t length)
{
   struct mulle_asciidata   data;

   data.characters = s;
   data.length     = length;
   return( data);
}



// low level conversion, no checks dst is assumed to be wide enough
// returns end of dst
MULLE__UTF_GLOBAL
mulle_utf16_t   *_mulle_ascii_convert_to_utf16( char *src,
                                                size_t len,
                                                mulle_utf16_t *dst);

MULLE__UTF_GLOBAL
mulle_utf32_t   *_mulle_ascii_convert_to_utf32( char *src,
                                                size_t len,
                                                mulle_utf32_t *dst);

MULLE__UTF_GLOBAL
void   mulle_ascii_bufferconvert_to_utf16( char *src,
                                           size_t len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t *addbytes);

MULLE__UTF_GLOBAL
void   mulle_ascii_bufferconvert_to_utf32( char *src,
                                           size_t len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t *addbytes);

#endif
