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
#ifndef mulle_utf8_string_h__
#define mulle_utf8_string_h__

#include "include.h"

#include "mulle-utf-type.h"

#include "mulle-utf8.h"

#include <stddef.h>
#include <string.h>


static inline int  mulle_utf8_strcmp( char *s, char *other)
{
   return( strcmp( s, other));
}


static inline int  mulle_utf8_strncmp( char *s, char *other, int len)
{
   return( strncmp( s, other, len));
}


static inline char  *mulle_utf8_strdup( char *s)
{
   return( mulle_allocator_strdup( NULL, s));
}


//
// hand coded because linux doesn't have it by default, and I want to get rid
// of the warning without having to define __USE_XOPEN2K8
//
static inline size_t  mulle_utf8_strnlen( char *s, size_t len)
{
   char   *start;
   char   *sentinel;

   if( ! s)
      return( 0);

   assert( len != (size_t) -1);

   start    = s;
   sentinel = &s[ len];

   while( s < sentinel)
   {
      if( ! *s)
         break;
      ++s;
   }
   return( (size_t) (s - start));
}


/*
 * only terminates, does not fill with zero
 */
MULLE__UTF_GLOBAL
char   *mulle_utf8_strncpy( char *dst, size_t len, char *src);


// strstr
MULLE__UTF_GLOBAL
char   *mulle_utf8_strnstr( char *s, size_t len, char *search);


static inline char  *mulle_utf8_strstr( char *s, char *search)
{
   return( mulle_utf8_strnstr( s, (size_t) -1, search));
}


// strchr
MULLE__UTF_GLOBAL
char   *mulle_utf8_strnchr( char *s, size_t len, mulle_utf32_t c);

static inline char  *mulle_utf8_strchr( char *s, mulle_utf32_t c)
{
   return( mulle_utf8_strnchr( s, (size_t) -1, c));
}


// strspn 
MULLE__UTF_GLOBAL
size_t   mulle_utf8_strspn( char *s, char *search);


// strcspn
MULLE__UTF_GLOBAL
size_t   mulle_utf8_strcspn( char *s, char *search);


// p_n: contains utf32 chars to skip, returns actually skipped
//
MULLE__UTF_GLOBAL
char   *mulle_utf8_skiputf32( char *s, size_t *p_n);


// MEMO: need these variety for ObjC where there might not be a terminating 0
static inline size_t   mulle_utf8_strnspn( char *s, size_t length, char *search)
{
   MULLE__UTF_GLOBAL
   size_t   _mulle_utf8_strnxspn( char *string, size_t length, char *search, int flag);

   if( ! s)
      return( 0);
   return( _mulle_utf8_strnxspn( s, length, search, 1));
}


static inline size_t   mulle_utf8_strncspn( char *s, size_t length, char *search)
{
   MULLE__UTF_GLOBAL
   size_t   _mulle_utf8_strnxspn( char *string, size_t length, char *search, int flag);

   if( ! s)
      return( 0);   
   return( _mulle_utf8_strnxspn( s, length, search, 0));
}


static inline void   mulle_utf8_memcpy( char *dst, char *src, size_t len)
{
   memcpy( dst, src, len);
}


static inline void   mulle_utf8_memmove( char *dst, char *src, size_t len)
{
   memmove( dst, src, len);
}


#endif
