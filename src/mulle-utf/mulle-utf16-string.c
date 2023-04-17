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
#if _WIN32
#include <malloc.h> // for alloca
#endif


size_t  mulle_utf16_strnlen( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *sentinel;
   mulle_utf16_t   *p;

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
mulle_utf16_t   *mulle_utf16_strncpy( mulle_utf16_t *dst, mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *memo;
   mulle_utf16_t   *sentinel;
   mulle_utf16_t   c;

   assert( dst);
   assert( src);
   assert( src >= &dst[len] || &src[len] <= dst);

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
   unsigned int   hash_pattern;
   unsigned int   hash_text;
   unsigned int   Bm;
   unsigned int   i;
   unsigned int   n;

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

   sentinel = &s1[ len];

   while( s1 < sentinel)
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



static int   _compare_mulle_utf32_t( mulle_utf32_t *a, mulle_utf32_t *b)
{
   return( *a - *b);
}

#define compare_mulle_utf32_t   ((int (*)( const void *, const void *)) _compare_mulle_utf32_t)


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
static size_t   _mulle_utf16_strspn( mulle_utf16_t *s1, mulle_utf16_t *s2, int flag)
{
   mulle_utf16_t   *start;
   mulle_utf16_t   *tmp;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   size_t          s2_len;
   unsigned int    i;

   assert( flag == 0 || flag == 1);

   start  = s1;
   s2_len = mulle_utf16_strlen( s2);
   if( ! s2_len)
      return( 0);

   if( s2_len == 1)
   {
      d = *s2;

      --s1;
      while( (c = *++s1))
         if( c != d)
            break;
      return( s1 - start);
   }

   i = 0;
   {
#if _WIN32
      mulle_utf32_t   *buf = alloca( sizeof( mulle_utf32_t) * s2_len);
#else
      mulle_utf32_t   buf[ sizeof( mulle_utf32_t) * s2_len];
#endif

      --s2;
      while( (d = mulle_utf16_pull_surrogatepair( *++s2, &s2)))
         buf[ i++] = d;
      assert( i <= s2_len);

      qsort( buf, i, sizeof( mulle_utf32_t), compare_mulle_utf32_t);

      --s1;
      while( tmp = s1, c = mulle_utf16_pull_surrogatepair( *++s1, &s1))
      {
         if( (! bsearch( &c, buf, i, sizeof( mulle_utf32_t), compare_mulle_utf32_t)) == flag)
            break;
      }
      return( tmp + 1 - start);
   }
}


size_t   mulle_utf16_strspn( mulle_utf16_t *s1, mulle_utf16_t *s2)
{
   return( _mulle_utf16_strspn( s1, s2, 1));
}


size_t   mulle_utf16_strcspn( mulle_utf16_t *s1, mulle_utf16_t *s2)
{
   return( _mulle_utf16_strspn( s1, s2, 0));
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


