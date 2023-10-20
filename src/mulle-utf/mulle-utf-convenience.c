//
//  mulle_utf_convenience.c
//  mulle-utf
//
//  Created by Nat! on 30.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#include "mulle-utf-convenience.h"

#include "mulle-utf8.h"
#include "mulle-utf16.h"
#include "mulle-utf32.h"
#include "mulle-utf-noncharacter.h"

#include <errno.h>



void   mulle_utf8_conversion_context_add_bytes( void *_p,
                                                void *bytes,
                                                size_t len)
{
   struct mulle_utf8_conversion_context *p = _p;
   char                                 *end;
   char                                 *s_end;
   char                                 *s;
   char                                 *t;
   char                                 c;

   /* EOB reached ? */
   end = &p->buf[ len];
   if( end > p->sentinel)
   {
      if( ! p->sentinel)
         return;

      // figure out what we can safely copy and then stop
      // copy over what can be copied safely ?
      s     = bytes;
      s_end = &s[ p->sentinel - p->buf];
      for(;;)
      {
         t = s;
         c = *s++;
         if( ! mulle_utf8_is_asciicharacter( c))
            s = &s[ mulle_utf8_get_extracharacterslength( c)];
         if( s > s_end)
            break;
      }

      len         = (t - (char *) bytes);
      end         = &p->buf[ len];
      p->sentinel = NULL; // ensure we don't add a late smaller character
   }

   memcpy( p->buf, bytes, len);

   // dial forwards for next memcpy
   p->buf = end;
}


void   mulle_utf16_conversion_context_add_bytes( void *_p,
                                                 void *bytes,
                                                 size_t len)
{
   struct mulle_utf16_conversion_context *p = _p;
   mulle_utf16_t                         *end;
   mulle_utf16_t                         *s;
   mulle_utf16_t                         *t;
   mulle_utf16_t                         *s_end;
   mulle_utf16_t                         c;

   end = &p->buf[ len / sizeof( mulle_utf16_t)];
   if( end > p->sentinel)
   {
      if( ! p->sentinel)
         return;

      // figure out what we can safely copy and then stop
      // copy over what can be copied safely ?
      s     = (mulle_utf16_t *) bytes;
      s_end = &s[ p->sentinel - p->buf];
      for(;;)
      {
         t = s;
         c = *s++;
         if( mulle_utf32_is_surrogatecharacter( c))
            s++;
         if( s > s_end)
            break;
      }

      len         = (t - (mulle_utf16_t *) bytes) * sizeof( mulle_utf16_t);
      end         = &p->buf[ len];
      p->sentinel = NULL; // ensure we don't add a late smaller character
   }

   memcpy( p->buf, bytes, len);

   // dial forwards for next memcpy
   p->buf = end;
}


void   mulle_utf32_conversion_context_add_bytes( void *_p,
                                                 void *bytes,
                                                 size_t len)
{
   struct mulle_utf32_conversion_context *p = _p;
   mulle_utf32_t                         *end;

   end = &p->buf[ len / sizeof( mulle_utf32_t)];
   if( end > p->sentinel)
   {
      if( ! p->sentinel)
         return;

      len         = (p->sentinel - p->buf) * sizeof( mulle_utf32_t);
      p->sentinel = NULL; // ensure we don't add a late smaller character
      // fall thru
   }

   memcpy( p->buf, bytes, len);

   // dial forwards for next memcpy
   p->buf = end;
}



# pragma mark - utf8

// rename to utf16_string
mulle_utf16_t  *mulle_utf8_convert_to_utf16_string( char *src,
                                                    size_t len,
                                                    struct mulle_allocator *allocator)
{
   struct mulle_utf_information            info;
   mulle_utf16_t                           *memo;
   struct mulle_utf16_conversion_context   ctxt;

   if( mulle_utf8_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo  = mulle_allocator_malloc( allocator, sizeof( mulle_utf16_t) * (info.utf16len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf16len];

   mulle_utf8_bufferconvert_to_utf16( info.start,
                                      info.utf8len,
                                      &ctxt,
                                      mulle_utf16_conversion_context_add_bytes);

   // add trailing zero
   memo[ info.utf16len] = 0;

   return( memo);
}


mulle_utf32_t  *mulle_utf8_convert_to_utf32_string( char *src,
                                                    size_t len,
                                                    struct mulle_allocator *allocator)
{
   struct mulle_utf_information            info;
   mulle_utf32_t                           *memo;
   struct mulle_utf32_conversion_context   ctxt;

   if( mulle_utf8_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo = mulle_allocator_malloc( allocator, sizeof( mulle_utf32_t) * (info.utf32len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf32len];

   mulle_utf8_bufferconvert_to_utf32( info.start,
                                      info.utf8len,
                                      &ctxt,
                                      mulle_utf32_conversion_context_add_bytes);
   // add trailing zero
   memo[ info.utf32len] = 0;

   return( memo);
}


# pragma mark - utf16

char  *mulle_utf16_convert_to_utf8_string( mulle_utf16_t *src,
                                           size_t len,
                                           struct mulle_allocator *allocator)
{
   struct mulle_utf_information           info;
   char                                   *memo;
   struct mulle_utf8_conversion_context   ctxt;

   if( mulle_utf16_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo = mulle_allocator_malloc( allocator, sizeof( char) * (info.utf8len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf8len];

   mulle_utf16_bufferconvert_to_utf8( info.start,
                                      info.utf16len,
                                      &ctxt,
                                      mulle_utf8_conversion_context_add_bytes);
   // add trailing zero
   memo[ info.utf8len] = 0;

   return( memo);
}


mulle_utf32_t  *mulle_utf16_convert_to_utf32_string( mulle_utf16_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator)
{
   struct mulle_utf_information            info;
   mulle_utf32_t                           *memo;
   struct mulle_utf32_conversion_context   ctxt;

   if( mulle_utf16_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo = mulle_allocator_malloc( allocator, sizeof( mulle_utf32_t) * (info.utf32len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf32len];

   mulle_utf16_bufferconvert_to_utf32( info.start,
                                       info.utf16len,
                                       &ctxt,
                                       mulle_utf32_conversion_context_add_bytes);

   // add trailing zero
   memo[ info.utf32len] = 0;

   return( memo);
}


# pragma mark -
# pragma mark utf32

char  *mulle_utf32_convert_to_utf8_string( mulle_utf32_t *src,
                                           size_t len,
                                           struct mulle_allocator *allocator)
{
   struct mulle_utf_information           info;
   char                                   *memo;
   struct mulle_utf8_conversion_context   ctxt;

   if( mulle_utf32_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo  = mulle_allocator_malloc( allocator, sizeof( char) * (info.utf8len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf8len];

   mulle_utf32_bufferconvert_to_utf8( info.start,
                                      info.utf32len,
                                      &ctxt,
                                      mulle_utf8_conversion_context_add_bytes);
   memo[ info.utf8len] = 0;

   return( memo);
}



mulle_utf16_t  *mulle_utf32_convert_to_utf16_string( mulle_utf32_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator)
{
   struct mulle_utf_information            info;
   mulle_utf16_t                           *memo;
   struct mulle_utf16_conversion_context   ctxt;

   if( mulle_utf32_information( src, len, &info))
   {
      errno = EINVAL;
      return( NULL);
   }

   memo  = mulle_allocator_malloc( allocator, sizeof( mulle_utf16_t) * (info.utf16len + 1));

   ctxt.buf      = memo;
   ctxt.sentinel = &ctxt.buf[ info.utf16len];

   mulle_utf32_bufferconvert_to_utf16( info.start,
                                       info.utf32len,
                                       &ctxt,
                                       mulle_utf16_conversion_context_add_bytes);

   // add trailing zero
   memo[ info.utf16len] = 0;

   return( memo);
}



#pragma mark -  support for -toLower

int   _mulle_utf8_character_mogrify( struct mulle_utf8data *dst,
                                     struct mulle_utf8data *src,
                                     struct mulle_utf_mogrification_info *info)
{
   char            *p;
   char            *q;
   char            *p_sentinel;
   char            *q_sentinel;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   assert( dst != src);
   assert( info->f1_conversion);

   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;
   q_sentinel  = &q[ dst->length - 4];  // expands to 4

   while( p < p_sentinel)
   {
      if( q >= q_sentinel)
         return( -1);

      c            = mulle_utf8_next_utf32character( &p);
      d            = (*info->f1_conversion)( c);
      conversions += c != d;
      q            = mulle_utf32_as_utf8( d, q);

   }

   dst->length = q - dst->characters;
   return( conversions ? 1 : 0);
}


//
// Can be used for mulle_utf16data where its known that there are no
// surrogate pairs contained and the mogrification stays with 16 bit,
// which is tolower/toupper! This can be used inplace.
//
int   _mulle_utf16_character_mogrify_unsafe( struct mulle_utf16data *dst,
                                             struct mulle_utf16data *src,
                                             struct mulle_utf_mogrification_info *info)
{
   mulle_utf16_t   *p;
   mulle_utf16_t   *p_sentinel;
   mulle_utf16_t   *q;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   // assert( dst != src); // src can be same as dst
   assert( info->f1_conversion);

   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;

   if( dst->length < src->length)
      return( -1);

   while( p < p_sentinel)
   {
      c            = *p++;
      d            = (*info->f1_conversion)( c);
      assert( d <= 0xFFFF);
      conversions += c != d;
      *q++         = (mulle_utf16_t) d;
   }

   dst->length = q - dst->characters;
   return( conversions ? 1 : 0);
}


//
// the mogrification can expand into 32 bit, so we make the dst data as wide
//
int   _mulle_utf16_character_mogrify( struct mulle_utf32data *dst,
                                      struct mulle_utf16data *src,
                                      struct mulle_utf_mogrification_info *info)
{
   mulle_utf16_t   *p;
   mulle_utf16_t   *p_sentinel;
   mulle_utf32_t   *q;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   // assert( dst != src); // src can be same as dst
   assert( info->f1_conversion);

   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;

   if( dst->length < src->length)
      return( -1);

   while( p < p_sentinel)
   {
      c = *p++;
      if( mulle_utf32_is_highsurrogatecharacter( c))  // hi surrogate
      {
         // decode surrogate
         assert( p < p_sentinel);
         c = mulle_utf16_decode_surrogatepair( (mulle_utf16_t) c, *p++);
      }
      d            = (*info->f1_conversion)( c);
      conversions += c != d;
      *q++         = d;
   }

   dst->length = q - dst->characters;
   return( conversions ? 1 : 0);
}


int   _mulle_utf32_character_mogrify( struct mulle_utf32data *dst,
                                      struct mulle_utf32data *src,
                                      struct mulle_utf_mogrification_info *info)
{
   mulle_utf32_t   *p;
   mulle_utf32_t   *q;
   mulle_utf32_t   *p_sentinel;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   // assert( dst != src); // src can be same as dst
   assert( info->f1_conversion);

   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;

   if( dst->length < src->length)
      return( -1);

   while( p < p_sentinel)
   {
      c            = *p++;
      d            = (*info->f1_conversion)( c);
      conversions += c != d;
      *q++         = d;
   }

   dst->length = q - dst->characters;
   return( conversions ? 1 : 0);
}


/*
 *
 */
int   _mulle_utf8_word_mogrify( struct mulle_utf8data *dst,
                                struct mulle_utf8data *src,
                                struct mulle_utf_mogrification_info *info)
{
   int             is_start;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   char            *p;
   char            *q;
   char            *p_sentinel;
   char            *q_sentinel;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   assert( dst != src);
   assert( info->f1_conversion);
   assert( info->f2_conversion);
   assert( info->is_white);

   is_start    = 1;
   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;
   q_sentinel  = &q[ dst->length - 4];  // expands to 4 \0

   while( p < p_sentinel)
   {
      if( q >= q_sentinel)
         return( -1);

      c = mulle_utf8_next_utf32character( &p);
      if( (*info->is_white)( c))
      {
         is_start = 1;
         d        = c;
      }
      else
      {
         if( is_start)
         {
            d = (*info->f1_conversion)( c);
            is_start = 0;
         }
         else
            d = (*info->f2_conversion)( c);
      }
      conversions += c != d;
      q            = mulle_utf32_as_utf8( d, q);
   }

   dst->length = q - dst->characters;
   return( conversions ? 1 : 0);
}


int   _mulle_utf16_word_mogrify( struct mulle_utf32data *dst,
                                 struct mulle_utf16data *src,
                                 struct mulle_utf_mogrification_info *info)
{
   int             is_start;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   mulle_utf16_t   *p;
   mulle_utf16_t   *p_sentinel;
   mulle_utf32_t   *q;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   assert( dst != (struct mulle_utf32data *) src);
   assert( info->f1_conversion);
   assert( info->f2_conversion);
   assert( info->is_white);

   is_start    = 1;
   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;

   if( dst->length < src->length)
      return( -1);

   while( p < p_sentinel)
   {
      c = *p++;
      if( mulle_utf32_is_highsurrogatecharacter( c))  // hi surrogate
      {
         // decode surrogate
         assert( p < p_sentinel);
         c = mulle_utf16_decode_surrogatepair( (mulle_utf16_t) c, *p++);
      }

      if( (*info->is_white)( c))
      {
         is_start = 1;
         d        = c;
      }
      else
      {
         if( is_start)
         {
            d = (*info->f1_conversion)( c);
            is_start = 0;
         }
         else
            d = (*info->f2_conversion)( c);
      }
      conversions += c != d;
      *q++         = d;
   }

   dst->length = q - dst->characters;

   return( conversions ? 1 : 0);
}


int   _mulle_utf32_word_mogrify( struct mulle_utf32data *dst,
                                 struct mulle_utf32data *src,
                                 struct mulle_utf_mogrification_info *info)
{
   int             is_start;
   mulle_utf32_t   c;
   mulle_utf32_t   d;
   mulle_utf32_t   *p;
   mulle_utf32_t   *q;
   mulle_utf32_t   *p_sentinel;
   size_t          conversions;

   assert( info);
   assert( dst);
   assert( src);
   assert( dst != src);
   assert( info->f1_conversion);
   assert( info->f2_conversion);
   assert( info->is_white);

   is_start    = 1;
   conversions = 0;
   p           = src->characters;
   p_sentinel  = &p[ src->length];
   q           = dst->characters;

   if( dst->length < src->length)
      return( -1);

   while( p < p_sentinel)
   {
      c = *p++;
      if( (*info->is_white)( c))
      {
         is_start = 1;
         d        = c;
      }
      else
      {
         if( is_start)
         {
            d = (*info->f1_conversion)( c);
            is_start = 0;
         }
         else
            d = (*info->f2_conversion)( c);
      }
      conversions += c != d;
      *q++         = d;
   }

   dst->length = q - dst->characters;

   return( conversions ? 1 : 0);
}

