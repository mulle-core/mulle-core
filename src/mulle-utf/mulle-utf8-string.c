//
//  mulle-utf8-string.c
//  mulle-utf
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
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
#include "mulle-utf8-string.h"

#include "include-private.h"

#include "mulle-utf32.h"



/*
 * only terminates, does not fill with zero
 */
char   *mulle_utf8_strncpy( char *dst, size_t len, const char *src)
{
   char   *memo;
   char   *sentinel;
   char   c;

   if( ! dst || ! src)
      return( dst);

   assert( src);
   assert( len != (size_t) -1);
   assert( src >= &dst[ len] || src <= dst); // len for dst is known, but can't be inferred for src

   memo     = dst;
   sentinel = &dst[ len];

   while( dst < sentinel)
   {
      c      = *src++;
      *dst++ = c;
      if( ! c)
         break;
   }
   return( memo);
}

//
// different API to strnstr, and on linux strnstr is only available
// with BSD...
// You can't search for '\0' with this function.
//
char   *mulle_utf8_strnstr( const char *s, size_t len, const char *search)
{
   const char   *sentinel;
   const char   *candidate;
   const char   *p;
   const char   *q;
   size_t  search_len;

   if( ! s || ! search)
      return( NULL);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s);

   search_len = mulle_utf8_strlen( search);
   if( ! search_len)
      return( (char *) s);

   if( search_len > len)
      return( NULL);

   sentinel = &s[ len - search_len + 1];

   for( candidate = s; candidate < sentinel; candidate++)
   {
      if( *candidate != *search)
         continue;

      p = candidate + 1;
      q = search + 1;
      while( *q && *p == *q)
      {
         p++;
         q++;
      }
      if( ! *q)
         return( (char *) candidate);
   }
   return( NULL);
}


char   *mulle_utf8_strnchr( const char *s, size_t len, mulle_utf32_t c)
{
   char     buf[ 16];
   char     *end;
   size_t   n;

   if( ! s)
      return( NULL);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s) + 1;

   end = mulle_utf32_as_utf8( c, buf);
   n   = end - buf;
   if( ! n)
      return( NULL);

   // special code for 'c' == 0 and simplifies other ascii searches
   if( n == 1)
      return( (char *) memchr( s, c, len));

   assert( n < sizeof( buf));
   *end = 0;

   return( mulle_utf8_strnstr( s, len, buf));
}


static size_t   _mulle_utf8_strxspn( const char *string, const char *search, int flag)
{
   const char   *s;
   char   c;
   int    found;

   if( ! string)
      return( 0);

   for( s = string; (c = *s); s++)
   {
      found = mulle_utf8_strchr( search, c) != NULL;
      if( found != flag)
         break;
   }
   return( s - string);
}


size_t   mulle_utf8_strspn( const char *s, const char *search)
{
   return( _mulle_utf8_strxspn( s, search, 1));
}


size_t   mulle_utf8_strcspn( const char *s, const char *search)
{
   return( _mulle_utf8_strxspn( s, search, 0));
}



size_t   _mulle_utf8_strnxspn( const char *string, size_t length, const char *search, int flag)
{
   const char   *s;
   const char   *sentinel;
   char   c;
   int    found;

   assert( ! length || string);

   for( s = string, sentinel = &s[ length]; s < sentinel; s++)
   {
      c = *s;
      assert( c);

      found = mulle_utf8_strchr( search, c) != NULL;
      if( found != flag)
         break;
   }
   return( s - string);
}




char   *mulle_utf8_skiputf32( char *s, size_t *p_n)
{
   size_t   n;
   size_t   i;
   mulle_utf32_t  c;

   i = 0;
   if( s)
   {
      n = *p_n;
      while( i < n)
      {
         c = _mulle_utf8_next_utf32character( &s);
         if( ! c)
            break;
         ++i;
      }
   }
   *p_n = i;
   return( s);
}
