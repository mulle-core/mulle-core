//
//  mulle_utf8.c
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
#include "mulle-ascii.h"

#include "mulle-utf16.h"
#include "mulle-utf32.h"

#include <string.h>

//
// this also does not do any error checking, the UTF8 string must be perfect
//
mulle_utf16_t   *_mulle_ascii_convert_to_utf16( char *src,
                                                size_t len,
                                                mulle_utf16_t *dst)
{
   char   *sentinel;
   char   _c;

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( _c <= 0x7F);
      *dst++ = _c;
   }
   return( dst);
}

//
// this also does not do any error checking, the ASCII string must be perfect
//
mulle_utf32_t   *_mulle_ascii_convert_to_utf32( char *src,
                                                size_t len,
                                                mulle_utf32_t *dst)
{
   char   *sentinel;
   char   _c;

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( _c <= 0x7F);
      *dst++ = _c;
   }
   return( dst);
}


//
// this does not do any error checking, the ASCII string must be perfect
//
void   mulle_ascii_bufferconvert_to_utf16( char *src,
                                           size_t len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t addbytes)
{
   char            *sentinel;
   char            _c;
   mulle_utf16_t   x;

   if( len == (size_t) -1)
      len = strlen( src);
   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( _c <= 0x7F);

      x = _c;
      (*addbytes)( buffer, &x, sizeof( x));
   }
}

//
// this does not do any error checking, the ASCII string must be perfect
//
void  mulle_ascii_bufferconvert_to_utf32( char *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t addbytes)
{
   char            *sentinel;
   char            _c;
   mulle_utf32_t   x;

   if( len == (size_t) -1)
      len = strlen( src);
   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( _c <= 0x7F);

      x = _c;
      (*addbytes)( buffer, &x, sizeof( x));
   }
}


