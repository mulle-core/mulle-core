/*
 *  mulle_utf16_string.c
 *  MulleRegularExpressions
 *
 *  Created by Nat! on 09.11.11.
//  Copyright (C) 2011 Nat!, Mulle kybernetiK.
//  Copyright (c) 2011 Codeon GmbH.
//  All rights reserved.
 *
 */
#pragma clang diagnostic ignored "-Wparentheses"

#include "mulle-utf16-string.h"

#include "mulle-utf16.h"
#include "mulle-utf-noncharacter.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>



size_t  mulle_utf16_strnlen( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *sentinel;
   mulle_utf16_t   *p;

   if( ! src)
      return( 0);

   assert( len != (size_t) -1);

   p        = src;
   sentinel = &p[ len];

   while( p < sentinel)
   {
      if( ! *p)
         break;
      ++p;
   }
   return( p - src);
}



mulle_utf16_t  *mulle_utf16_strdup( mulle_utf16_t *s)
{
   size_t          length;
   mulle_utf16_t   *dst;

   length = (size_t) (mulle_utf16_strlen( s) + 1) * sizeof( mulle_utf16_t);
   dst    = mulle_allocator_malloc( NULL, length);
   memcpy( dst, s, length);
   return( dst);
}




/* memo: "string" oriented routines view the utf16 string as an array
         of unsigned shorts, where the only special character is the 0, which
         is the terminator. "string" oriented routines are all mulle_utf16_
         routines except:

         "character" oriented routines: These actually concern themselves also
         with unicode turning surrogates into their unicode. The length of a
         unicode derived from two surrogates is still 2 though.
         These are mulle_utf16_strchr, mulle_utf16_strspn and mulle_utf16_strcspn.
         mulle_utf16_strchr as a special bonus, also searches for surrogate "atoms".

         None of these routines have the concept of a BOM. You need to strip
         it out yourself.
*/


/*
 * only terminates, does not fill with zero
 */
mulle_utf16_t   *mulle_utf16_strncpy( mulle_utf16_t *dst, size_t len, mulle_utf16_t *src)
{
   mulle_utf16_t   *memo;
   mulle_utf16_t   *sentinel;
   mulle_utf16_t   c;

   if( ! dst || ! src)
      return( dst);

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


/*
 * finds composed surrogates and! surrogate pieces
 */
mulle_utf16_t   *mulle_utf16_strchr( mulle_utf16_t *s, mulle_utf32_t c)
{
   mulle_utf16_t   d;
   mulle_utf16_t   e;
   mulle_utf32_t   f;

   if( ! s)
      return( NULL);

   --s;
   for( ;;)
   {
      d = *++s;
      if( d == c)
         return( s); /* compatible to strchr w/respect to c == 0 */
      if( ! d)
         break;

      if( d < 0xD800 || d >= 0xDC00)
         continue;

      if( ! (e = *++s))
         return( NULL);

      assert( e >= 0xDC00 && e < 0xE000);

      /* always HI followed by LO */
      f = mulle_utf16_decode_surrogatepair( d, e);
      if( f == c)
         return( &s[-1]);

      /* allow search for second surrogate */
      if( e == f)
         return( s);
   }

   return( NULL);
}


/*
 * use Rabin–Karp for expected "usual" strings to match
 * this could "partial" match a surrogate, but is this
 * my problem ?
 */
#define B    131

mulle_utf16_t  *mulle_utf16_strstr( mulle_utf16_t *s, mulle_utf16_t *pattern)
{
   size_t   hash_pattern;
   size_t   hash_text;
   size_t   Bm;
   size_t   i;
   size_t   n;

   if( ! s || ! pattern)
      return( NULL);

   if( pattern[ 0] == 0)
      return( s);

   Bm   = 1;
   hash_pattern = hash_text = 0;

   /* initial hash and count pattern length */
   for( n = 0; s[ n] && pattern[ n]; n++)
   {
      Bm           *= B;
      hash_pattern  = hash_pattern * B + pattern[ n];
      hash_text     = hash_text * B + s[ n];
   }

   if( ! s[ n] && pattern[ n])
      return( NULL);

   for( i = n;; i++)
   {
      if( (hash_pattern == hash_text) && ! mulle_utf16_strncmp( &s[ i - n], pattern, n))
         return( &s[ i - n]);

      if( ! s[ i])
         return( NULL);

      hash_text = hash_text * B - s[ i - n] * Bm + s[ i];
   }
}


int   mulle_utf16_strncmp( mulle_utf16_t *s1, mulle_utf16_t *s2, size_t len)
{
   mulle_utf16_t   *sentinel;
   mulle_utf32_t   c;
   mulle_utf32_t   d;

   if( len == (size_t) -1)
      len = mulle_utf16_strlen( s2);

   sentinel = &s2[ len];

   while( s2 < sentinel)
   {
      c = *s1++;
      d = *s2++;

      if( c != d)
         return( c - d);

      if( ! c)
         break;
   }

   return( 0);
}


static int   compare_mulle_utf32( const void *p_a, const void *p_b)
{
   mulle_utf32_t   a = *(mulle_utf32_t *) p_a;
   mulle_utf32_t   b = *(mulle_utf32_t *) p_b;

   return( a - b);
}



static inline mulle_utf32_t   mulle_utf16_pull_surrogatepair( mulle_utf16_t c, mulle_utf16_t **s)
{
   mulle_utf16_t   d;

   if( c < 0xD800 || c >= 0xDC00)
      return( c);

   d = *++*s;
   if( d < 0xDC00 || d >= 0xE000)
   {
      --*s;
      return( c);
   }

   /* always HI followed by LO */
   return( mulle_utf16_decode_surrogatepair( c, d));
}


/* careful: a pattern of 0xD8xx, 0xDCxx will match incoming characters in that
            order. It will not match 0xDCxx, 0xD8xx. It can match a single
            surrogate though if needed.
*/
static size_t   _mulle_utf16_strxspn( mulle_utf16_t *s1, mulle_utf16_t *s2, int flag)
{
   mulle_utf16_t   *start;
   mulle_utf16_t   *tmp;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   mulle_utf32_t   *buf;
   size_t    s2_len;
   size_t    i;
   int             found;
   mulle_utf32_t   space[ 32];

   if( ! s1)
      return( 0);

   assert( flag == 0 || flag == 1);

   s2_len = mulle_utf16_strlen( s2);
   if( ! s2_len)
      return( 0);

   i     = 0;
   start = s1;
   tmp   = s1;

   // we don't have a flexbuffer here...
   if( s2_len < 32)
      buf = space;
   else
      buf = mulle_malloc( 32 * sizeof( mulle_utf32_t));

   --s2;
   while( (d = mulle_utf16_pull_surrogatepair( *++s2, &s2)))
      buf[ i++] = d;
   assert( i <= s2_len);

   qsort( buf, i, sizeof( mulle_utf32_t), compare_mulle_utf32);

   --s1;
   while( tmp = s1, c = mulle_utf16_pull_surrogatepair( *++s1, &s1))
   {
      found = ! bsearch( &c, buf, i, sizeof( mulle_utf32_t), compare_mulle_utf32);
      if( found == flag)
         break;
   }

   if( buf != space)
      mulle_free( buf);

   return( tmp + 1 - start);
}


size_t   mulle_utf16_strspn( mulle_utf16_t *s1, mulle_utf16_t *s2)
{
   return( _mulle_utf16_strxspn( s1, s2, 1));
}


size_t   mulle_utf16_strcspn( mulle_utf16_t *s1, mulle_utf16_t *s2)
{
   return( _mulle_utf16_strxspn( s1, s2, 0));
}


int   _mulle_utf16_atoi( mulle_utf16_t **s_p)
{
   char            buf[ 64];
   char            *sentinel;
   char            *p;
   mulle_utf16_t   *s;
   mulle_utf32_t   c;

   s        = *s_p;
   p        = buf;
   sentinel = &p[ sizeof( buf) - 1];
   while( p < sentinel)
   {
      c = *s;
      if( ! isdigit( c))
         break;
      *p++ = (char) c;
      s++;
   }
   *p   = 0;
   *s_p = s;

   return( (int) strtol( buf, NULL, 10));
}


