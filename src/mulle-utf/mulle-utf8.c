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
#include "mulle-utf8.h"

#include "mulle-char5.h"
#include "mulle-utf16.h"
#include "mulle-utf32.h"
#include "mulle-utf-noncharacter.h"
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


// it's not compatible to Apple
#define FORBID_NON_CHARACTERS  1
// #define USE_SMART_CODE  // hard to measure if this is faster


enum
{
   mulle_utf8_ascii_start_character,
   mulle_utf8_multiple_start_character,
   mulle_utf8_invalid_start_character
};



// https://en.wikipedia.org/wiki/UTF-8#Invalid_byte_sequences
// 0x80-0xBF can only appear after a start character
// 0xC0-0xC1 and 0xF5-0xFF are unsupported
//
static inline int   mulle_utf8_is_invalidstartcharacter( mulle_utf8_t c)
{
   return( (c >= 0x80 && c < 0xC2) || c >= 0xF5);
}


//static inline int   mulle_utf8_is_start_character( mulle_utf8_t c)
//{
//   return( !  mulle_utf8_is_invalidstartcharacter( c));
//}



static inline int   mulle_utf8_get_startcharactertype( mulle_utf8_t c)
{
   if( mulle_utf8_is_asciicharacter( c))
      return( mulle_utf8_ascii_start_character);

   if( mulle_utf8_is_invalidstartcharacter( c))
      return( mulle_utf8_invalid_start_character);

   return( mulle_utf8_multiple_start_character);
}


static inline int   mulle_utf8_is_validcontinuationcharacter( mulle_utf8_t c)
{
   return( c >= 0x80 && c < 0xC0);
}


// length excluding 'c'
static inline unsigned int  mulle_utf8_get_extracharacterslength( mulle_utf8_t c)
{
   assert( mulle_utf8_get_startcharactertype( c) == mulle_utf8_multiple_start_character);

   if( c < 0xE0)
      return( 1);  // 11 bits

   if( c < 0xF0)
      return( 2); //  16 bits

   return( 3);  // 21 bits -> UTF32
}


//
// this also works with extra_len = 0
//
static mulle_utf32_t   mulle_utf8_extracharactersvalue( mulle_utf8_t *src,
                                                        size_t extra_len)
{
   mulle_utf8_t    _c;
   mulle_utf32_t   x;

   assert( src);
   assert( extra_len >=0 && extra_len <= 3);

   _c = *src++;
   x  = _c;

   switch( extra_len)
   {
   case 1 : // 11 bits
      x   = (_c & 0x1F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F);

      assert( x >= 128);
      break;

   case 2 :  // 16 bits
      x   = (_c & 0x0F) << 12;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F);

      assert( x >= 0x800 && (x < 0xD800 || x >= 0xE000));
      break;

   case 3 :   // 21 bits -> UTF32
      x   = (_c & 0x7) << 18;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F) << 12;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( -2);
      x  |= (_c & 0x3F);

      assert( x >= 0x10000 && x <= 0x10FFFF);
   }
   return( x);
}


mulle_utf32_t   _mulle_utf8_next_utf32character( mulle_utf8_t **s_p)
{
   mulle_utf8_t    *s;
   mulle_utf8_t    c;
   mulle_utf32_t   value;
   unsigned int    len;

   s = *s_p;
   c = *s++;
   if( mulle_utf8_is_asciicharacter( c))
   {
      *s_p  = s;
      return( c);
   }

   len   = mulle_utf8_get_extracharacterslength( c);
   value = mulle_utf8_extracharactersvalue( &s[ -1], len);
   *s_p  = &s[ len]; // extra len does not include 'c'

   return( value);
}


mulle_utf32_t   __mulle_utf8data_next_utf32character( struct mulle_utf8data *rover,
                                                      mulle_utf8_t c)
{
   mulle_utf32_t   value;
   unsigned int    len;

   len = mulle_utf8_get_extracharacterslength( c);
   if( len >= rover->length)
      return( -2);

   value = mulle_utf8_extracharactersvalue( rover->characters, len);

   len               += 1;  // 'c' + extra_len
   rover->characters += len;
   rover->length     -= len;

   return( value);
}


mulle_utf32_t   _mulle_utf8data_next_utf32character( struct mulle_utf8data *rover)
{
   mulle_utf8_t   c;

   if( ! rover->length)
      return( -1);

   c = *rover->characters;
   if( mulle_utf8_is_asciicharacter( c))
   {
      rover->characters++;
      rover->length--;
      return( c);
   }

   return( __mulle_utf8data_next_utf32character( rover, c));
}



// sorta undoes _mulle_utf8_next_utf32_value
mulle_utf32_t   _mulle_utf8_previous_utf32character( mulle_utf8_t **s_p)
{
   mulle_utf8_t    *s;
   mulle_utf8_t    c;
   mulle_utf32_t   value;
   unsigned int    extra_len;

   extra_len = 0;
   s         = *s_p;
   for(;;)
   {
      c = *--s;

      if( mulle_utf8_get_startcharactertype( c) != mulle_utf8_invalid_start_character)
         break;

      ++extra_len;
   }

   value = mulle_utf8_extracharactersvalue( s, extra_len);
   *s_p  = s;

   return( value);
}



//
// the slower non-crashing code ...
//
int   mulle_utf8_are_valid_extracharacters( mulle_utf8_t *src, unsigned int len, mulle_utf32_t *p_x)
{
   mulle_utf8_t    _c;
   mulle_utf32_t   x;

   assert( src);
   assert( len >= 1 && len <= 3);

   _c = *src++;

   switch( len)
   {
   case 1 : // 11 bits
      x  = (_c & 0x1F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);

      x  |= (_c & 0x3F);
      if( x < 0x80)   // can't be ASCII
         return( 0);
      break;

   case 2 :  // 16 bits
      x  = (_c & 0x0F) << 12;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);
      x  |= (_c & 0x3F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);
      x  |= (_c & 0x3F);

      if( x < 0x800)  // can't be in case 1 range
         return( 0);
      break;

   case 3 :   // 21 bits -> UTF32
      x   = (_c & 0x7) << 18;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);
      x  |= (_c & 0x3F) << 12;

      _c = *src++;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);
      x  |= (_c & 0x3F) << 6;

      _c = *src;
      if( ! mulle_utf8_is_validcontinuationcharacter( _c))
         return( 0);
      x  |= (_c & 0x3F);

      if( x < 0x10000 || x > 0x0010FFFF)  // // can't be in case 2 range or totally too large
         return( 0);
   }

#if FORBID_NON_CHARACTERS
   if( mulle_utf32_is_invalidcharacter( x))
      return( 0);
#endif
   *p_x = x;
   return( 1);
}



//
// dst should be 2 * len
//
mulle_utf8_t   *_mulle_iso1_convert_to_utf8( char *iso1,
                                             size_t len,
                                             mulle_utf8_t *dst)
{
   unsigned char   *src;
   unsigned char   *sentinel;
   unsigned char   _c;

   assert( len != (size_t) -1);

   src      = (unsigned char *) iso1;
   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      if( _c < 0x80)
      {
         *dst++ = _c;
         continue;
      }
      *dst++ = 0xc0 | (_c >> 6);
      *dst++ = 0x80 | (_c & 0x3F);
   }
   return( dst);
}


char   *_mulle_utf8_convert_to_iso1( mulle_utf8_t *src,
                                     size_t len,
                                     char *iso1,
                                     int unknown)
{
   mulle_utf8_t   *sentinel;
   mulle_utf8_t   _c;
   mulle_utf8_t   _d;
   unsigned char  *dst;

   assert( len != (size_t) -1);

   sentinel = &src[ len];
   dst      = (unsigned char *) iso1;

   while( src < sentinel)
   {
      _c = *src++;
      if( _c < 0x80)
      {
         *dst++ = _c;
         continue;
      }

      if( (_c & 0xFC) == 0xC0) // we have 110000nn
      {                        // and     10nnnnnn
         if( src < sentinel)
         {
            _d = *src++;
            if( (_c & 0xC0) == 0x80)
            {
               *dst++ = ((_c & 0x3) << 6) | (_d & 0x3F);
               continue;
            }
         }
      }
      if( unknown < 0)
         return( NULL);
      if( ! unknown)
         continue;
      // so if you pass 0x100 you can still put in 0
      *dst++ = (unsigned char) unknown;
   }
   return( (char *) dst);
}


// this also does not do any error checking, the UTF8 string must be perfect
//
mulle_utf16_t   *_mulle_utf8_convert_to_utf16( mulle_utf8_t *src,
                                               size_t len,
                                               mulle_utf16_t *dst)
{
   mulle_utf8_t    *next;
   mulle_utf8_t    *sentinel;
   mulle_utf8_t    _c;
   size_t          extra_len;
   uint32_t        x;

   assert( len != (size_t) -1);

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( mulle_utf8_get_startcharactertype( _c) != mulle_utf8_invalid_start_character);

      if( (char) _c >= 0)
      {
         *dst++ = (uint16_t) _c;
         continue;
      }

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      next      = &src[ extra_len];
      assert( next <= sentinel);

      x   = mulle_utf8_extracharactersvalue( src - 1, extra_len);
      src = next;
      if( x < 0x10000)
      {
         *dst++ = (uint16_t) x;
         continue;
      }

      dst = _mulle_utf32_convert_to_utf16_as_surrogatepair( x, dst);
   }
   return( dst);
}

//
// this also does not do any error checking, the UTF8 string must be perfect
// the destination buffer must be large enough.
mulle_utf32_t   *_mulle_utf8_convert_to_utf32( mulle_utf8_t *src,
                                               size_t len,
                                               mulle_utf32_t *dst)
{
   mulle_utf8_t   *next;
   mulle_utf8_t   *sentinel;
   mulle_utf8_t   _c;
   size_t         extra_len;

   assert( len != (size_t) -1);

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( mulle_utf8_get_startcharactertype( _c) != mulle_utf8_invalid_start_character);

      if( (char) _c >= 0)
      {
         *dst++ = _c;
         continue;
      }

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      next      = &src[ extra_len];
      assert( next <= sentinel);

      *dst++ = mulle_utf8_extracharactersvalue( src - 1, extra_len);
      src    = next;
   }
   return( dst);
}


//
// this also does not do any error checking, the UTF8 string must be perfect
//
void   mulle_utf8_bufferconvert_to_utf16( mulle_utf8_t *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t addbytes)
{
   mulle_utf16_t   _w;
   mulle_utf8_t    *next;
   mulle_utf8_t    *sentinel;
   mulle_utf8_t    _c;
   size_t          extra_len;
   uint32_t        x;

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);
   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( mulle_utf8_get_startcharactertype( _c) != mulle_utf8_invalid_start_character);

      if( (char) _c >= 0)
      {
         _w = (uint16_t) _c;
         (*addbytes)( buffer, &_w, sizeof( _w));
         continue;
      }

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      next      = &src[ extra_len];
      assert( next <= sentinel);

      x   = mulle_utf8_extracharactersvalue( src - 1, extra_len);
      src = next;
      if( x < 0x10000)
      {
         _w = (uint16_t) x;
         (*addbytes)( buffer, &_w, sizeof( _w));
         continue;
      }

      mulle_utf32_bufferconvert_to_utf16_as_surrogatepair( x, buffer, addbytes);
   }
}

//
// this also does not do any error checking, the UTF8 string must be perfect
//
void  mulle_utf8_bufferconvert_to_utf32( mulle_utf8_t *src,
                                         size_t len,
                                         void *buffer,
                                         mulle_utf_add_bytes_function_t addbytes)
{
   mulle_utf8_t   *next;
   mulle_utf8_t   *sentinel;
   mulle_utf8_t   _c;
   size_t         extra_len;
   mulle_utf32_t   x;

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);
   // if dst_len == -1, then sentinel - 1 = dst_sentinel (OK!)

   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      assert( mulle_utf8_get_startcharactertype( _c) != mulle_utf8_invalid_start_character);

      if( (char) _c >= 0)
      {
         x = _c;
         (*addbytes)( buffer, &x, sizeof( x));
         continue;
      }

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      next      = &src[ extra_len];
      assert( next <= sentinel);

      x   = mulle_utf8_extracharactersvalue( src - 1, extra_len);
      src = next;
      (*addbytes)( buffer, &x, sizeof( x));
   }
}


mulle_utf8_t  *mulle_utf8_validate( mulle_utf8_t *src, size_t len)
{
   mulle_utf8_t    *sentinel;
   mulle_utf8_t    *end;
   mulle_utf8_t    _c;
   size_t          extra_len;
   mulle_utf32_t   _x;

   if( ! src)
      return( NULL);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);

   if( mulle_utf8_has_leading_bomcharacter( src, len))
   {
      src += 3;
      len -= 3;
   }

   sentinel = &src[ len];
   for( ; src < sentinel; src++)
   {
      if( ! (_c = *src))
         return( src);

      if( mulle_utf8_is_asciicharacter( _c))
         continue;

      if( mulle_utf8_is_invalidstartcharacter( _c))
         return( src);

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      end       = &src[ extra_len];
      if( end >= sentinel)
         return( src);

      if( ! mulle_utf8_are_valid_extracharacters( src, extra_len, &_x))
         return( src);
      src = end;
   }
   return( NULL);
}


//
// make it more optimal, by (a) checking that pointer can be accessed with
// a long or long long
// (b) masking value with 0x80808080 to figure out if all are "ASCII"
//
int  mulle_utf8_information( mulle_utf8_t *src, size_t len, struct mulle_utf_information *info)
{
   mulle_utf8_t                   *end;
   mulle_utf8_t                   *sentinel;
   mulle_utf8_t                   _c;
   size_t                         dst_len;
   size_t                         extra_len;
   struct mulle_utf_information   dummy;
   mulle_utf32_t                  _x;

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

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);

   //
   // remove leading BOM
   //
   info->has_bom = mulle_utf8_has_leading_bomcharacter( src, len);
   if( info->has_bom)
   {
      src += 3;
      len -= 3;
   }

   info->start = src;
   sentinel    = &src[ len];
   dst_len     = len;

#ifdef USE_SMART_CODE
   /*
    * first loop checks for everything, we reduce the loop code
    * very slightly as soon as we determine that we are
    * not info->is_char5 , info->is_ascii and info->is_utf15 in that
    * order. This should pay off (slightly) for large strings
    */
   for( ; src < sentinel; src++)
   {
      _c = *src;

      if( ! _c)
      {
         info->has_terminating_zero = 1;
         goto done;
      }

      if( ! mulle_utf8_is_char5character( _c))
      {
         info->is_char5 = 0;
         break;
      }
   }

   /* known not to be not is_char5 */
   for( ; src < sentinel; _c = *++src)
   {
      if( ! _c)
      {
         info->has_terminating_zero = 1;
         goto done;
      }

      if( ! mulle_utf8_is_asciicharacter( _c))
      {
         info->is_ascii = 0;
         break;
      }
   }

   /* known not to be not is_ascii */
   for( ; src < sentinel; _c = *++src)
   {
      if( mulle_utf8_is_asciicharacter( _c))
      {
         if( ! _c)
         {
            info->has_terminating_zero = 1;
            goto done;
         }
         continue;
      }

      if( mulle_utf8_is_invalidstartcharacter( _c))
         goto fail;

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      dst_len  -= extra_len;  // reduce character count

      end = &src[ extra_len];
      if( end >= sentinel)
         goto fail;

      if( ! mulle_utf8_are_valid_extracharacters( src, extra_len, &_x))
         goto fail;

      if( _x >= 0x08000)
         info->is_utf15 = 0;  // could optimize this away as well, but...
      if( _x >= 0x10000)
         info->utf16len += 1;
      src = end;
   }
#else
   for( ; src < sentinel; src++)
   {
      _c = *src;

      if( mulle_utf8_is_asciicharacter( _c))
      {
         if( ! _c)
         {
            info->has_terminating_zero = 1;
            break;
         }

         if( info->is_char5 && ! mulle_utf8_is_char5character( _c))
            info->is_char5 = 0;
         continue;
      }

      info->is_ascii = 0;
      if( mulle_utf8_is_invalidstartcharacter( _c))
         goto fail;

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      dst_len  -= extra_len;  // reduce character count

      end = &src[ extra_len];
      if( end >= sentinel)
         goto fail;

      if( ! mulle_utf8_are_valid_extracharacters( src, extra_len, &_x))
         goto fail;
      if( _x >= 0x08000)
         info->is_utf15 = 0;
      if( _x >= 0x10000)
         info->utf16len += 1;

//
//      if( ! mulle_utf8_are_valid_extra_chars( src, extra_len))
//      {
//         info->invalid_utf8 = src;
//         return( -1);
//      }
      src = end;
   }
#endif

//done:
   info->utf8len   = src - (mulle_utf8_t *) info->start;                // actual UTF8 strlen
   info->utf32len  = dst_len - (len - info->utf8len);  // number of characters
   info->utf16len += info->utf32len;                   // size in utf16 with escapes
   info->is_char5 &= info->is_ascii;

   return( 0);

fail:
   memset( info, 0, sizeof( *info));
   info->invalid = src;
   return( -1);
}



int   mulle_utf8_is_ascii( mulle_utf8_t *src, size_t len)
{
   mulle_utf8_t   *sentinel;

   if( ! src)
      return( 0);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);

   sentinel = &src[ len];

   while( src < sentinel)
      if( ! mulle_utf8_is_asciicharacter( *src++))
         return( 0);

   return( 1);
}

//
// this routine does not validate...
//
size_t  mulle_utf8_utf16length( mulle_utf8_t *src, size_t len)
{
   mulle_utf8_t   _c;
   mulle_utf8_t   *sentinel;
   mulle_utf8_t   *end;
   size_t          extra_len;
   size_t          dst_len;

   if( ! src)
      return( 0);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( src);

   sentinel = &src[ len];
   dst_len  = len;

   for( ; src < sentinel; src++)
   {
      _c = *src;
      assert( mulle_utf8_get_startcharactertype( _c) != mulle_utf8_invalid_start_character);

      if( (char) _c >= 0)
         continue;

      // 32 bit ?
      if( _c >= 0xF0)
         dst_len++;

      extra_len = mulle_utf8_get_extracharacterslength( _c);
      dst_len  -= extra_len;

      end       = &src[ extra_len];
      if( end >= sentinel)
         return( -1);
#ifndef NDEBUG
      do
      {
         _c = *++src;
         assert( mulle_utf8_is_validcontinuationcharacter( _c));
      }
      while( src < end);
#else
      src = end;
#endif
   }
   return( dst_len);
}




//
// different API to strnstr, and on linux strnstr is only available
// with BSD...
// You can't search for '\0' with this function.
//
mulle_utf8_t   *mulle_utf8_strnstr( mulle_utf8_t *s, size_t len, mulle_utf8_t *search)
{
   mulle_utf8_t      *sentinel;
   mulle_utf8_t      *p;
   size_t            offset;

   if( ! s || ! search)
      return( NULL);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s);

   offset   = mulle_utf8_strlen( search);
   if( ! offset)
      return( NULL);

   sentinel = &s[ len];
   p        = search;

   // fprintf( stderr, "# s=\"%s\" search=\"%s\" len=%ld, offset=%ld, sentinel=\"%s\"\n",
   //            s, search, (long) len, (long) offset, sentinel);

   for(;;)
   {
      if( s >= sentinel)
         return( NULL);

      if( *s++ != *p)
      {
         p = search;
         continue;
      }

      if( *++p)
         continue;

      return( &s[ -(long) offset]);
   }
}


// 0 is no terminator in this case
mulle_utf8_t  *mulle_utf8_strnchr( mulle_utf8_t *s, size_t len, mulle_utf8_t c)
{
   mulle_utf8_t   *sentinel;

   if( ! s)
      return( NULL);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s);

   sentinel = &s[ len];

   while( s < sentinel)
   {
      if( *s == c)
         return( s);
      ++s;
   }
   return( NULL);
}


size_t   mulle_utf8_strnspn( mulle_utf8_t *s, size_t len, mulle_utf8_t *search)
{
   mulle_utf8_t   *start;
   mulle_utf8_t   *sentinel;
   size_t         search_len;

   if( ! s)
      return( 0);

   if( ! search)
      return( 0);
   search_len = mulle_utf8_strlen( search);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s);

   start    = s;
   sentinel = &s[ len];

   while( s < sentinel)
   {
      if( ! mulle_utf8_strnchr( search, search_len, *s))
         break;
      ++s;
   }
   return( s - start);
}


size_t   mulle_utf8_strncspn( mulle_utf8_t *s, size_t len, mulle_utf8_t *search)
{
   mulle_utf8_t   *start;
   mulle_utf8_t   *sentinel;
   size_t         search_len;

   if( ! s)
      return( 0);

   if( len == (size_t) -1)
      len = mulle_utf8_strlen( s);

   if( search)
   {
      search_len = mulle_utf8_strlen( search);

      start    = s;
      sentinel = &s[ len];

      while( s < sentinel)
      {
         if( mulle_utf8_strnchr( search, search_len, *s))
            return( s - start);
         ++s;
      }
   }
   return( len);
}


struct mulle_utf8data  mulle_utf8data_range_of_utf32_range( struct mulle_utf8data data,
                                                            struct mulle_range range)
{
   mulle_utf8_t            *s;
   mulle_utf8_t            *sentinel;
   uintptr_t               i;
   uintptr_t               end;
   struct mulle_utf8data   rval;

   assert( mulle_range_is_valid( range));

   range = mulle_range_validate_against_length( range, data.length);

   // is this needed ?
//   if( mulle_utf8_has_leading_bomcharacter( data.characters, data.length))
//   {
//      data.characters += 3;
//      data.length     -= 3;
//   }

   if( ! range.length)
      return( mulle_utf8data_make( NULL, 0));

   s        = data.characters;
   sentinel = &data.characters[ data.length];
   end      = range.location + range.length;
   i        = 0;

   rval.characters = s;   // for the analyzer
   while( s < sentinel)
   {
      if( i == range.location)
         rval.characters = s;
      if( *s++ & 0x80)
         continue;
      if( ++i == end)
      {
         rval.length = s - rval.characters;
         return( rval);
      }
   }

   return( mulle_utf8data_make( NULL, 0));
}




//
// dst should be 2 * len
// http://www.alanwood.net/demos/macroman.html
//

static mulle_utf8_t   *_mulle_table_convert_to_utf8( char *macroman,
                                                     size_t len,
                                                     uint16_t table[],
                                                     mulle_utf8_t *dst)
{
   unsigned char   *src;
   unsigned char   *sentinel;
   unsigned char   _c;
   mulle_utf32_t   utf32;

   assert( len != (size_t) -1);

   src      = (unsigned char *) macroman;
   sentinel = &src[ len];

   while( src < sentinel)
   {
      _c = *src++;
      if( _c < 0x80)
      {
         *dst++ = _c;
         continue;
      }

      utf32 = table[ _c - 0x80];
      dst   = _mulle_utf32_as_utf8( utf32, dst);
   }
   return( dst);
}


mulle_utf8_t   *_mulle_macroman_convert_to_utf8( char *macroman,
                                                 size_t len,
                                                 mulle_utf8_t *dst)
{
   static uint16_t   macroman_0x80_0xFF[] =
   {
      0x00C4, 0x00C5, 0x00C7, 0x00C9,  0x00D1, 0x00D6, 0x00DC, 0x00E1,
      0x00E0, 0x00E2, 0x00E4, 0x00E3,  0x00E5, 0x00E7, 0x00E9, 0x00E8,
      0x00EA, 0x00EB, 0x00ED, 0x00EC,  0x00EE, 0x00EF, 0x00F1, 0x00F3,
      0x00F2, 0x00F4, 0x00F6, 0x00F5,  0x00FA, 0x00F9, 0x00FB, 0x00FC,
      0x2020, 0x00B0, 0x00A2, 0x00A3,  0x00A7, 0x2022, 0x00B6, 0x00DF,
      0x00AE, 0x00A9, 0x2122, 0x00B4,  0x00A8, 0x2260, 0x00C6, 0x00D8,
      0x221E, 0x00B1, 0x2264, 0x2265,  0x00A5, 0x00B5, 0x2202, 0x2211,
      0x220F, 0x03C0, 0x222B, 0x00AA,  0x00BA, 0x03A9, 0x00E6, 0x00F8,

      0x00BF, 0x00A1, 0x00AC, 0x221A,  0x0192, 0x2248, 0x2206, 0x00AB,
      0x00BB, 0x2026, 0x00A0, 0x00C0,  0x00C3, 0x00D5, 0x0152, 0x0153,
      0x2013, 0x2014, 0x201C, 0x201D,  0x2018, 0x2019, 0x00F7, 0x25CA,
      0x00FF, 0x0178, 0x2044, 0x20AC,  0x2039, 0x203A, 0xFB01, 0xFB02,
      0x2021, 0x00B7, 0x201A, 0x201E,  0x2030, 0x00C2, 0x00CA, 0x00C1,
      0x00CB, 0x00C8, 0x00CD, 0x00CE,  0x00CF, 0x00CC, 0x00D3, 0x00D4,
      0xF8FF, 0x00D2, 0x00DA, 0x00DB,  0x00D9, 0x0131, 0x02C6, 0x02DC,
      0x00AF, 0x02D8, 0x02D9, 0x02DA,  0x00B8, 0x02DD, 0x02DB, 0x02C7
   };

   return( _mulle_table_convert_to_utf8( macroman, len, macroman_0x80_0xFF, dst));
}


mulle_utf8_t   *_mulle_nextstep_convert_to_utf8( char *nextstep,
                                                 size_t len,
                                                 mulle_utf8_t *dst)
{
   // http://www.kostis.net/charsets/nextstep.htm
   // https://en.wikipedia.org/wiki/NeXT_character_set
   static uint16_t   nextstep_0x80_0xFF[] =
   {
      160, 192, 193, 194,     195, 196, 197, 199,
      200, 201, 202, 203,     204, 205, 206, 207,

      208, 209, 210, 211,     212, 213, 214, 217,
      218, 219, 220, 221,     222, 181, 215, 247,

      169, 161, 162, 163,     8260, 165, 402, 167,
      164, 0x27, 8220, 171,    8249,  8250, 64257, 64258,

      174, 8211, 8224, 8225,  183, 166, 182, 8729,
      8218, 8222, 8221, 187,  8943,  8240, 172, 191,

      185, 768, 180, 770,     771, 175, 728, 729,
      168, 178, 176, 184,     179, 733, 731, 711,

      821, 177, 188, 189,     190, 224, 225, 226,
      227, 228, 229, 231,     232, 233, 234, 235,

      236, 198, 237, 170,     238, 239, 240, 241,
      321, 216, 338, 186,     242, 243, 244, 245,

      246, 230, 249, 250,     251, 305, 252, 253,
      322, 248, 339, 223,     254, 255, 0, 0
   };
   return( _mulle_table_convert_to_utf8( nextstep, len, nextstep_0x80_0xFF, dst));
}
