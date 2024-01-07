//
//  mulle_utf32.c
//  mulle-utf
//
//  Created by Nat! on 19.03.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#include "mulle-utf32.h"

#include "mulle-utf32-string.h"
#include "mulle-utf16.h"
#include "mulle-utf-noncharacter.h"
#include "mulle-char5.h"
#include "mulle-char7.h"
#include <assert.h>
#include <string.h>


#define FORBID_NON_CHARACTERS  1


mulle_utf16_t  *_mulle_utf32_convert_to_utf16_as_surrogatepair( mulle_utf32_t x,
                                                                mulle_utf16_t *dst)
{
   uint16_t  top;
   uint16_t  bottom;
   uint16_t  hi;
   uint16_t  lo;

   assert( x >= 0x10000 && x <= 0x10FFFF);

   x -= 0x10000;

   assert( (x >> 10) <= 0x3FF);

   top    = (uint16_t) (x >> 10);
   bottom = (uint16_t) (x & 0x3FF);

   hi = 0xD800 + top;
   lo = 0xDC00 + bottom;

   assert( hi >= 0xD800 && hi < 0xDC00);
   assert( lo >= 0xDC00 && lo < 0xE000);

   // (nat) I used to flip those adds around based on endianness, but I think
   // that was wrong
   *dst++ = hi;
   *dst++ = lo;
   return( dst);
}


// must be proper UTF32 code!
char  *_mulle_utf32_convert_to_utf8( mulle_utf32_t *src, unsigned int len, char *_dst)
{
   unsigned char   *dst = (unsigned char *) _dst;
   mulle_utf32_t   *sentinel;
   mulle_utf32_t   x;

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)
   assert( len != (unsigned int) -1);

   sentinel = &src[ len];

   while( src < sentinel)
   {
      x = *src++;

      if( x < 0x800)
      {
         if( x < 0x80)
         {
            *dst++ = (unsigned char) x;
            continue;
         }

         *dst++ = 0xC0 | (unsigned char) (x >> 6);
         *dst++ = 0x80 | (x & 0x3F);
         continue;
      }
      else
      {
         if( x < 0x10000)
         {
            assert( ! mulle_utf32_is_surrogatecharacter( x));

            *dst++ = 0xE0 | (unsigned char) (x >> 12);
            *dst++ = 0x80 | ((x >> 6) & 0x3F);
            *dst++ = 0x80 | (x & 0x3F);
            continue;
         }

         assert( x <= 0x10FFFF);

         *dst++ = 0xF0 | (unsigned char) (x >> 18);
         *dst++ = 0x80 | ((x >> 12) & 0x3F);
         *dst++ = 0x80 | ((x >> 6) & 0x3F);
         *dst++ = 0x80 | (x & 0x3F);
      }
   }
   return( (char *) dst);
}


mulle_utf16_t   *_mulle_utf32_convert_to_utf16( mulle_utf32_t *src,
                                                unsigned int len,
                                                mulle_utf16_t *dst)
{
   mulle_utf32_t   *sentinel;
   mulle_utf32_t   x;

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)
   assert( len != (unsigned int) -1);

   sentinel = &src[ len];

   while( src < sentinel)
   {
      x = *src++;

      if( x < 0x10000)
      {
         assert( ! mulle_utf32_is_surrogatecharacter( x));

         *dst++ = (uint16_t) x;
         continue;
      }
      dst = _mulle_utf32_convert_to_utf16_as_surrogatepair( x, dst);
   }
   return( dst);
}


// must be proper UTF32 code!
void  mulle_utf32_bufferconvert_to_utf8( mulle_utf32_t *src,
                                         unsigned int len,
                                         void *buffer,
                                         mulle_utf_add_bytes_function_t *addbytes)
{
   mulle_utf32_t   *sentinel;
   mulle_utf32_t   x;
   unsigned char   *s;
   unsigned char   *s_flush;
   unsigned char   tmp[ 128];

   if( len == (unsigned int) -1)
      len = mulle_utf32_strlen( src);

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];
   s        = tmp;
   s_flush  = &tmp[ sizeof( tmp) / sizeof( char) - 4];

   while( src < sentinel)
   {
      if( s >= s_flush)
      {
         (*addbytes)( buffer, tmp, s - tmp);
         s = tmp;
      }

      x = *src++;

      if( x < 0x800)
      {
         if( x < 0x80)
         {
            *s++ = (unsigned char) x;
            continue;
         }

         *s++ = 0xC0 | (unsigned char) (x >> 6);
         *s++ = 0x80 | (x & 0x3F);
         continue;
      }
      else
      {
         if( x < 0x10000)
         {
            assert( ! mulle_utf32_is_surrogatecharacter( x));

            *s++ = 0xE0 | (unsigned char) (x >> 12);
            *s++ = 0x80 | ((x >> 6) & 0x3F);
            *s++ = 0x80 | (x & 0x3F);
            continue;
         }

         assert( x <= 0x10FFFF);

         *s++ = 0xF0 | (unsigned char) (x >> 18);
         *s++ = 0x80 | ((x >> 12) & 0x3F);
         *s++ = 0x80 | ((x >> 6) & 0x3F);
         *s++ = 0x80 | (x & 0x3F);
      }
   }

   if( s != tmp)
      (*addbytes)( buffer, tmp, s - tmp);
}


void   mulle_utf32_bufferconvert_to_utf16( mulle_utf32_t *src,
                                           unsigned int len,
                                           void *buffer,
                                           mulle_utf_add_bytes_function_t *addbytes)
{
   mulle_utf32_t   *sentinel;
   mulle_utf32_t   x;
   mulle_utf16_t   *s;
   mulle_utf16_t   *s_flush;
   mulle_utf16_t   tmp[ 64];

   if( len == (unsigned int) -1)
      len = mulle_utf32_strlen( src);

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];
   s        = tmp;
   s_flush  = &tmp[ sizeof( tmp) / sizeof( mulle_utf16_t) - 2];

   while( src < sentinel)
   {
      if( s >= s_flush)
      {
         (*addbytes)( buffer, tmp, sizeof( mulle_utf16_t) * (s - tmp));
         s = tmp;
      }

      x = *src++;
      assert( x >= 0 && x <= mulle_utf32_max);

      if( x < 0x10000)
      {
         assert( ! mulle_utf32_is_surrogatecharacter( x));

         *s++ = (uint16_t) x;
         continue;
      }

      s = _mulle_utf32_convert_to_utf16_as_surrogatepair( x, s);
   }

   if( s != tmp)
      (*addbytes)( buffer, tmp, sizeof( mulle_utf16_t) * (s - tmp));
}


// must be proper UTF32 code!
size_t   mulle_utf32_utf8length( mulle_utf32_t *src,
                                 unsigned int len)
{
   mulle_utf32_t  *sentinel;
   uint32_t       x;
   size_t         size;

   if( ! src)
      return( 0);

   len = (len == (unsigned int) -1) ? mulle_utf32_strlen( src) : len;
   if( ! len)
      return( 0);

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];
   size     = len;

   while( src < sentinel)
   {
      x = *src++;

      assert( x >= 0 && x <= mulle_utf32_max);

      if( x < 0x800)
      {
         if( x < 0x80)
            continue;

         ++size;
         continue;
      }

      if( x < 0x10000)
      {
         assert( ! mulle_utf32_is_surrogatecharacter( x));
         size += 2;
         continue;
      }

      assert( x <= 0x10FFFF);
      size += 3;
   }
   return( size);
}


// must be proper UTF32 code!
unsigned int   mulle_utf32_utf16length( mulle_utf32_t *src,
                                        unsigned int len)
{
   mulle_utf32_t  *sentinel;
   uint32_t       x;

   if( ! src)
      return( 0);

   if( len == (unsigned int) -1)
      len = mulle_utf32_strlen( src);
   if( ! len)
      return( 0);

   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];

   // assume double
   while( src < sentinel)
   {
      x = *src++;

      assert( x >= 0 && x <= mulle_utf32_max);

      if( x >= 0xFFFF)
         len++;
   }
   return( len);
}


int   mulle_utf32_information( mulle_utf32_t *src,
                               unsigned int len,
                               struct mulle_utf_information *info)
{
   mulle_utf32_t                  _c;
   mulle_utf32_t                  *start;
   mulle_utf32_t                  *sentinel;
   struct mulle_utf_information   dummy;

   if( ! info)
      info = &dummy;

   info->has_terminating_zero = 0;
   info->invalid              = NULL;
   info->start                = src;
   info->is_ascii             = 1;
   info->is_char5             = 1;
   info->is_utf15             = 1;
   info->utf8len              = 0;
   info->utf16len             = 0;
   info->utf32len             = 0;
   info->has_bom              = 0;

   if( ! len)
      return( 0);
   if( ! src)
      goto fail;

   if( len == (unsigned int) -1)
      len = mulle_utf32_strlen( src);

   //
   // remove leading BOM
   //
   info->has_bom = mulle_utf32_is_bomcharacter( *src);
   if( info->has_bom)
   {
      src += 1;
      len -= 1;
   }

   info->start = src;
   start       = src;
   sentinel    = &src[ len];

   for( ; src < sentinel; src++)
   {
      _c = *src;

      if( mulle_utf32_is_asciicharacter( _c))
      {
         if( ! _c)
         {
            info->has_terminating_zero = 1;
            break;
         }
         if( info->is_char5 && ! mulle_utf32_is_char5character( _c))
            info->is_char5 = 0;
         continue;
      }

      info->is_ascii = 0;
      info->utf8len++;

      if( _c >= 0x0800)
         info->utf8len++;

      if( _c < 0 || _c > mulle_utf32_max)
         goto fail;

#if FORBID_NON_CHARACTERS
      if( mulle_utf32_is_invalidcharacter( _c))
         goto fail;
#endif

      if( _c >= 0x8000)
         info->is_utf15 = 0;

      if( _c >= 0x10000)
      {
         info->utf8len++;
         info->utf16len++;
      }
   }

   info->utf8len  += src - start;
   info->utf16len += src - start;
   info->utf32len += src - start;
   info->is_char5 &= info->is_ascii;

   return( 0);

fail:
   memset( info, 0, sizeof( *info));
   info->invalid = src;
   return( -1);
}



char   *_mulle_utf32_as_utf8_not_ascii( mulle_utf32_t x, char *_dst)
{
   unsigned char  *dst = (unsigned char *) _dst;
   assert( x >= 0 && x <= mulle_utf32_max);

   if( x < 0x800)
   {
      assert( x >= 0x80);
      *dst++ = 0xC0 | (unsigned char) (x >> 6);
      *dst++ = 0x80 | (x & 0x3F);
      return( (char *) dst);
   }

   if( x < 0x10000)
   {
      assert( ! mulle_utf32_is_surrogatecharacter( x));

      *dst++ = 0xE0 | (unsigned char) (x >> 12);
      *dst++ = 0x80 | ((x >> 6) & 0x3F);
      *dst++ = 0x80 | (x & 0x3F);
      return( (char *) dst);
   }

   assert( x <= 0x10FFFF);

   *dst++ = 0xF0 | (unsigned char) (x >> 18);
   *dst++ = 0x80 | ((x >> 12) & 0x3F);
   *dst++ = 0x80 | ((x >> 6) & 0x3F);
   *dst++ = 0x80 | (x & 0x3F);

   return( (char *) dst);
}


// same as mulle_utf16_is_invalid_char really
static inline int  mulle_utf32_is_invalid_char( mulle_utf32_t c)
{
   return( c == 0xFFFE || c == 0xFFFF || (c >= 0xFDD0 && c <= 0xFDEF));
}


mulle_utf32_t  *mulle_utf32_validate( mulle_utf32_t *src, unsigned int len)
{
   mulle_utf32_t   c;
   mulle_utf32_t   *sentinel;

   if( ! src)
      return( NULL);

   if( len == (unsigned int) -1)
      len = mulle_utf32_strlen( src);

   sentinel = &src[ len];

   for( ; src < sentinel; src++)
   {
      c = *src;

      if( ! c || c > mulle_utf32_max)
         return( src);

      if( mulle_utf32_is_invalid_char( c))
         return( src);

      if( mulle_utf32_is_surrogatecharacter( c))
         return( src);
   }
   return( 0);
}


enum mulle_utf_charinfo   _mulle_utf32_charinfo( mulle_utf32_t *src, unsigned int len)
{
   mulle_utf32_t   _c;
   mulle_utf32_t   *start;
   mulle_utf32_t   *sentinel;

   assert( len);
   assert( len != (unsigned int) -1);

   if( len > mulle_char5_get_maxlength())
      return( mulle_utf_is_not_char5_or_char7);

   start    = src;
   sentinel = &start[ len];
   if( len <= mulle_char7_get_maxlength())
   {
      for( ; src < sentinel; src++)
      {
         _c = *src;
         if( ! mulle_utf32_is_asciicharacter( _c))
            return( mulle_utf_is_not_char5_or_char7);
      }
      return( mulle_utf_is_char7);
   }

   for( ; src < sentinel; src++)
   {
      _c = *src;
      if( ! mulle_utf32_is_char5character( _c))
         return( mulle_utf_is_not_char5_or_char7);
   }
   return( mulle_utf_is_char5);
}


