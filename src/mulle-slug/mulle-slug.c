//
//  mulle-slug.c
//  mulle-slug
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
#include "include-private.h"

#include "mulle-slug.h"

#include <ctype.h>
#include <string.h>


int   __MULLE_SLUG_ranlib__;


uint32_t   mulle_slug_get_version( void)
{
   return( MULLE__SLUG_VERSION);
}


static struct map_entry
{
   mulle_utf32_t   utf32;
   char            *ascii;
} map[] =
{
#include "map.inc"
};


// auto-generated from UnicodeData.txt canonical/compat decompositions
static struct map_entry   decompose_map[] =
{
#include "decompose-map.inc"
};



static char  *search_map( struct map_entry *table, int n, mulle_utf32_t c)
{
   int   first;
   int   last;
   int   middle;

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      if( table[ middle].utf32 <= c)
      {
         if( table[ middle].utf32 == c)
            return( table[ middle].ascii);
         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }
   return( NULL);
}



static void  _mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                                    struct mulle_utf8data data,
                                                    char delimiter,
                                                    int passthru)
{
   mulle_utf32_t      c;
   mulle_utf32_t      prev;
   char               *walk;
   char               *sentinel;
   size_t             length;
   size_t             prevlen;

   if( ! data.length)
      return;

   prev     = 0;
   walk     = data.characters;
   sentinel = &data.characters[ data.length];
   prevlen  = mulle_buffer_get_length( buffer);

   while( walk < sentinel)
   {
      c = mulle_utf8_next_utf32character( &walk);
      if( c < 127)
      {
         switch( c)
         {
         case '\0' : goto stop;
         case ' '  :
         case '\f' :
         case '\n' :
         case '\r' :
         case '\t' :
         case '\v' :
         case '-'  : if( prev && prev != (mulle_utf32_t) delimiter)
                     {
                        mulle_buffer_add_byte( buffer, delimiter);
                        prev = (mulle_utf32_t) delimiter;
                     }
                     continue;

//            case '&'  : mulle_buffer_add_string( buffer, "and"); prev = c; continue;
         case '<'  : mulle_buffer_add_string( buffer, "less"); prev = c; continue;
         case '>'  : mulle_buffer_add_string( buffer, "greater"); prev = c; continue;
//            case '|'  : mulle_buffer_add_string( buffer, "or"); prev = c; continue;
         case '$'  : mulle_buffer_add_string( buffer, "dollar"); prev = c; continue;
         case '#'  : mulle_buffer_add_string( buffer, "hash"); prev = c; continue;
         }

         if( ! isprint( c))
            continue;

         if( ispunct( c))
         {
            if( prev && prev != (mulle_utf32_t) delimiter)
            {
               mulle_buffer_add_byte( buffer, delimiter);
               prev = (mulle_utf32_t) delimiter;
            }
            continue;
         }

         mulle_buffer_add_byte( buffer, c);
         prev = c;
         continue;
      }

      // non-ASCII: transliterate via lookup tables
      {
         char   *ascii;

         // skip combining marks (nonbase characters like U+0300 combining grave)
         // this handles decomposed forms: e + U+0301 -> just "e"
         if( mulle_unicode_is_nonbase( c))
            continue;

         // search hand-curated table first (has semantic mappings)
         ascii = search_map( map,
                             (int) (sizeof( map) / sizeof( map[ 0])),
                             c);
         // fall back to Unicode decomposition table
         if( ! ascii)
            ascii = search_map( decompose_map,
                                (int) (sizeof( decompose_map) / sizeof( decompose_map[ 0])),
                                c);
         if( ascii)
         {
            mulle_buffer_add_string( buffer, ascii);
            prev = c;
         }
         else if( passthru && ! mulle_unicode_is_whitespace( c)
                           && ! mulle_unicode_is_punctuation( c)
                           && ! mulle_unicode_is_control( c))
         {
            // pass through non-transliterable characters (CJK, Arabic, etc.) as UTF-8
            mulle_utf32_bufferconvert_to_utf8( &c,
                                               1,
                                               buffer,
                                               (mulle_utf_add_bytes_function_t *) mulle_buffer_add_bytes);
            prev = c;
         }
      }
   }

stop:
   length = mulle_buffer_get_length( buffer);
   while( length > prevlen + 1)
   {
      c = mulle_buffer_get_last_byte( buffer);
      if( c != (mulle_utf32_t) delimiter)
         break;

      mulle_buffer_remove_last_byte( buffer);
      --length;
   }
}


void  mulle_buffer_add_slugified_utf8data_with_delimiter( struct mulle_buffer *buffer,
                                                          struct mulle_utf8data data,
                                                          char delimiter)
{
   _mulle_buffer_add_slugified_utf8data( buffer, data, delimiter, 0);
}


void  mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                           struct mulle_utf8data data)
{
   _mulle_buffer_add_slugified_utf8data( buffer, data, '-', 0);
}


void  mulle_buffer_add_utf8_slugified_utf8data( struct mulle_buffer *buffer,
                                                struct mulle_utf8data data)
{
   _mulle_buffer_add_slugified_utf8data( buffer, data, '-', 1);
}


void  mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer,
                                     struct mulle_utf8data data)
{
   mulle_buffer_add_slugified_utf8data( buffer, data);
   mulle_buffer_make_string( buffer);
}



struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                struct mulle_allocator *allocator)
{
   struct mulle_utf8data   slug;

   //
   // tries to avoid output of -- and trailing or leading -
   // tries to avoid output of leading '#'
   //
   mulle_buffer_do_allocator( buffer, allocator)
   {
      mulle_buffer_slugify_utf8data( buffer, data);
      slug = mulle_data_as_utf8data( mulle_buffer_extract_data( buffer));
   }

   return( slug);
}


char   *mulle_utf8_slugify( const char *s)
{
   struct mulle_utf8data   data;
   struct mulle_utf8data   slug;

   if( ! s)
      return( NULL);

   data = mulle_utf8data_make( (char *) s, -1);
   slug = mulle_utf8data_slugify( data, NULL);
   assert( slug.characters);
   assert( slug.length >= 1); // sic (the trailing 0)
   return( (char *) slug.characters);
}


char   *mulle_utf8_slugify_utf8( const char *s)
{
   struct mulle_utf8data   data;
   struct mulle_utf8data   slug;

   if( ! s)
      return( NULL);

   data = mulle_utf8data_make( (char *) s, -1);

   mulle_buffer_do( buffer)
   {
      _mulle_buffer_add_slugified_utf8data( buffer, data, '-', 1);
      mulle_buffer_make_string( buffer);
      slug = mulle_data_as_utf8data( mulle_buffer_extract_data( buffer));
   }

   assert( slug.characters);
   assert( slug.length >= 1);
   return( (char *) slug.characters);
}


char  *mulle_slugify_with_delimiter( char *dst, size_t dst_len,
                                     const char *src, size_t src_len,
                                     char delimiter)
{
   struct mulle_utf8data   data;

   if( ! dst_len)
      return( NULL);

   if( ! src)
   {
      dst[ 0] = '\0';
      return( dst);
   }

   if( src_len == (size_t) -1)
      src_len = strlen( src);

   data = mulle_utf8data_make( (char *) src, src_len);

   mulle_buffer_do_flexible( buffer, dst, dst_len)
   {
      mulle_buffer_add_slugified_utf8data_with_delimiter( buffer, data, delimiter);
      mulle_buffer_make_string( buffer);

      // if the buffer overflowed into malloc, copy back and truncate
      if( mulle_buffer_get_string( buffer) != dst)
      {
         strncpy( dst, mulle_buffer_get_string( buffer), dst_len - 1);
         dst[ dst_len - 1] = '\0';
      }
   }

   return( dst);
}


char  *mulle_slugify( char *dst, size_t dst_len, const char *src, size_t src_len)
{
   return( mulle_slugify_with_delimiter( dst, dst_len, src, src_len, '-'));
}

