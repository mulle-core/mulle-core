//
//  mulle-char7.c
//  mulle-utf
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
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
#include "mulle-char7.h"

#include "mulle-utf8.h"
#include "mulle-utf16.h"
#include "mulle-utf32.h"


int   mulle_char7_is_char7string32( char *src, size_t len)
{
   char   *sentinel;

   if( ! src)
      return( 0);

   if( len == (size_t ) -1)
      len = mulle_utf8_strlen( src);

   if( len > mulle_char7_maxlength32)
      return( 0);

   sentinel = &src[ len];
   while( src < sentinel)
   {
      if( ! *src)
         return( 0);   // invalid char
      if( *src & 0x80)
         return( 0);   // invalid char
      ++src;
   }

   return( 1);
}


int   mulle_char7_is_char7string64( char *src, size_t len)
{
   char   *sentinel;

   if( ! src)
      return( 0);

   if( len == (size_t ) -1)
      len = mulle_utf8_strlen( src);

   if( len > mulle_char7_maxlength64)
      return( 0);

   sentinel = &src[ len];
   while( src < sentinel)
   {
      if( ! *src)
         return( 0);   // invalid char
      if( *src & 0x80)
         return( 0);   // invalid char
      ++src;
   }

   return( 1);
}


uint32_t  mulle_char7_encode32( char *src, size_t len)
{
   char       *s;
   char       *sentinel;
   int        char7;
   uint32_t   value;

   if( len == (size_t ) -1)
      len = mulle_utf8_strlen( src);

   assert( len <= mulle_char7_maxlength32);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


uint64_t  mulle_char7_encode64( char *src, size_t len)
{
   char       *s;
   char       *sentinel;
   int        char7;
   uint64_t   value;

   if( len == (size_t ) -1)
      len = mulle_utf8_strlen( src);

   assert( len <= mulle_char7_maxlength64);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


uint32_t  mulle_char7_encode32_utf16( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *s;
   mulle_utf16_t   *sentinel;
   int             char7;
   uint32_t        value;

   if( len == (size_t ) -1)
      len = mulle_utf16_strlen( src);

   assert( len <= mulle_char7_maxlength32);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


uint64_t  mulle_char7_encode64_utf16( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *s;
   mulle_utf16_t   *sentinel;
   int             char7;
   uint64_t        value;

   if( len == (size_t ) -1)
      len = mulle_utf16_strlen( src);

   assert( len <= mulle_char7_maxlength64);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


uint32_t  mulle_char7_encode32_utf32( mulle_utf32_t *src, size_t len)
{
   mulle_utf32_t   *s;
   mulle_utf32_t   *sentinel;
   int             char7;
   uint32_t        value;

   if( len == (size_t ) -1)
      len = mulle_utf32_strlen( src);

   assert( len <= mulle_char7_maxlength32);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


uint64_t  mulle_char7_encode64_utf32( mulle_utf32_t *src, size_t len)
{
   mulle_utf32_t   *s;
   mulle_utf32_t   *sentinel;
   int             char7;
   uint64_t        value;

   if( len == (size_t ) -1)
      len = mulle_utf32_strlen( src);

   assert( len <= mulle_char7_maxlength64);

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      char7 = *--s;
      assert( char7);

      assert( ! (char7 & 0x80));
      value <<= 7;
      value  |= char7;
   }
   return( value);
}


size_t  mulle_char7_decode32( uint32_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      *s++    = value & 0x7F;
      value >>= 7;
   }
   return( s - dst);
}


size_t  mulle_char7_decode64( uint64_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      *s++    = value & 0x7F;
      value >>= 7;
   }
   return( s - dst);
}


int  mulle_char7_get64( uint64_t value, size_t index)
{
   int   char7;

   assert( index < 8);
   do
   {
      char7 = value & 0x7F;
      value >>= 7;
   }
   while( index--);

   return( char7);
}


int  mulle_char7_get32( uint32_t value, size_t index)
{
   int   char7;

   assert( index < 4);
   do
   {
      char7   = value & 0x7F;
      value >>= 7;
   }
   while( index--);

   return( char7);
}
