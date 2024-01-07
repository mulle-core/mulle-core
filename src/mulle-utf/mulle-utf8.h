//
//  mulle_utf8.h
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
#ifndef mulle_utf8_h__
#define mulle_utf8_h__

#include "mulle-utf-type.h"

#include <string.h>


static inline int   mulle_utf8_is_asciicharacter( char c)
{
   return( (unsigned char) c < 0x80);
}


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
static inline int   mulle_utf8_is_invalidstartcharacter( char c)
{
   return( ((unsigned char) c >= 0x80 && (unsigned char) c < 0xC2) || (unsigned char) c >= 0xF5);
}


//static inline int   mulle_utf8_is_start_character( char c)
//{
//   return( !  mulle_utf8_is_invalidstartcharacter( c));
//}



static inline int   mulle_utf8_get_startcharactertype( char c)
{
   if( mulle_utf8_is_asciicharacter( c))
      return( mulle_utf8_ascii_start_character);

   if( mulle_utf8_is_invalidstartcharacter( c))
      return( mulle_utf8_invalid_start_character);

   return( mulle_utf8_multiple_start_character);
}


static inline int   mulle_utf8_is_validcontinuationcharacter( char c)
{
   return( (unsigned char) c >= 0x80 && (unsigned char) c < 0xC0);
}


// length excluding 'c'
static inline unsigned int  mulle_utf8_get_extracharacterslength( char c)
{
   assert( mulle_utf8_get_startcharactertype( c) == mulle_utf8_multiple_start_character);

   if( (unsigned char) c < 0xE0)
      return( 1);  // 11 bits

   if( (unsigned char) c < 0xF0)
      return( 2); //  16 bits

   return( 3);  // 21 bits -> UTF32
}


// Check: http://tools.ietf.org/html/rfc3629
// General considerations.
//    On the initial procurement of your UTF8 string, you must validate it once
//    then remember it as validated and don't change it (NSString :))
//    If this is the case, you can run all the other routines "quickly" without
//    having to validate again.

//
// if len is -1, assume that *s is '\0' terminated
// function is not ware of UTF8 BOM
// returned length does not include BOM
//
MULLE__UTF_GLOBAL
unsigned int  mulle_utf8_utf16length( char *src, size_t len);

MULLE__UTF_GLOBAL
unsigned int  mulle_utf8_utf32length( char *src, size_t len);


static inline size_t  mulle_utf8_utf16maxlength( size_t len)
{
   return( len * 4);
}


static inline int  mulle_utf8_has_leading_bomcharacter( char *src, size_t len)
{
   if( len < 3)
      return( 0);

   return( ((unsigned char *) src)[ 0] == 0xEF &&
           ((unsigned char *) src)[ 1] == 0xBB &&
           ((unsigned char *) src)[ 2] == 0xBF);
}

// 0: no 1: yes
// if yes, p_c will contain char value
//
MULLE__UTF_GLOBAL
int   mulle_utf8_are_valid_extracharacters( char *s, size_t len, mulle_utf32_t *p_c);

//
// if len is -1, assume that *s is '\0' terminated
// returns length in characters
// returns size in bytes
// returns pointer to character, that is invalid (first of compound chars)
// TODO: find encoding where nothing needs to be composed

MULLE__UTF_GLOBAL
int  mulle_utf8_information( char *s, size_t len, struct mulle_utf_information *info);

MULLE__UTF_GLOBAL
int  mulle_utf8_is_ascii( char *s, size_t len);

// returns NULL if OK, otherwise the offending character address
MULLE__UTF_GLOBAL
char  *mulle_utf8_validate( char *src, size_t len);




// extremely primitive! (Not as primitive anymore...)
// iterate back and forth over a  buffer. the utf8 must be valid, and
// this doesn't check for zero or buffer overflow
//
MULLE__UTF_GLOBAL
mulle_utf32_t   _mulle_utf8_next_utf32character( char **s_p);

MULLE__UTF_GLOBAL
mulle_utf32_t   _mulle_utf8_previous_utf32character( char **s_p);


// use this to walk through a utf8 string
// returns -2 if malformed
static inline mulle_utf32_t   mulle_utf8_next_utf32character( char **s_p)
{
   if( mulle_utf8_is_asciicharacter( **s_p))
      return( *(*s_p)++);
   return( _mulle_utf8_next_utf32character( s_p));
}



// used in Foundation and maybe here in the future
struct mulle_utf8data
{
   char    *characters;
   size_t   length;
};


static inline size_t  mulle_utf8_strlen( char *s)
{
   char   *p;

   if( ! s)
      return( 0);

   /* this produced the nicest looking i386 code :P */
   for( p = s - 1; *++p;);

   return( p - s);
}


static inline struct mulle_utf8data
   mulle_utf8data_make( char *s, size_t length)
{
   struct mulle_utf8data   data;

   data.length     = (length == (size_t) -1) ? strlen( s) : length;
   data.characters = data.length ? s : "";
   return( data);
}


// keep this compatible with mulle-data
static inline struct mulle_utf8data   mulle_utf8data_make_empty( void)
{
   struct mulle_utf8data   data;

   data.characters = "";
   data.length     = 0;
   return( data);
}

// keep this compatible with mulle-data
static inline struct mulle_utf8data   mulle_utf8data_make_invalid( void)
{
   struct mulle_utf8data   data;

   data.characters = NULL;
   data.length     = 0;
   return( data);
}


//
// use these two functions to allocate and free mulle_utf8data
// They know that if data.length == 0, that data.characters is a "" 
// static string
//
static inline void   mulle_utf8data_init( struct mulle_utf8data *data,
                                          char *characters,
                                          size_t length, 
                                          struct mulle_allocator *allocator)
{
   
   if( ! data)
      return;

   if( ! length)
   {
      *data = mulle_utf8data_make( NULL, 0);
      return;
   }

   data->characters                = mulle_allocator_malloc( allocator, length + 1);
   if( characters)
      memcpy( data->characters, characters, length);
   data->length                    = length;
   data->characters[ data->length] = 0;
}


static inline void
   mulle_utf8data_done( struct mulle_utf8data *data, struct mulle_allocator *allocator)
{
   if( data && data->length)
   {
      mulle_allocator_free( allocator, data->characters);
#ifdef DEBUG      
      data->characters = "<freed>";
      data->length     = UINTPTR_MAX;
#endif      
   }
}



static inline int   mulle_utf8data_is_empty( struct mulle_utf8data data)
{
   return( data.length == 0);
}


static inline int   mulle_utf8data_is_invalid( struct mulle_utf8data data)
{
   return( data.characters == NULL);
}


static inline void   mulle_utf8data_assert( struct mulle_utf8data data)
{
   assert( ! data.length || data.characters);
}


//
// returned data is actually one longer and terminated by '\0'
//
struct mulle_utf8data   mulle_utf8data_copy( struct mulle_utf8data data, 
                                             struct mulle_allocator *allocator);


//
// some conversions
//
static inline struct mulle_utf8data
   mulle_data_as_utf8data( struct mulle_data data)
{
   return( mulle_utf8data_make( data.bytes, data.length));
}


static inline struct mulle_data
   mulle_utf8data_as_data( struct mulle_utf8data data)
{
   return( mulle_data_make( data.characters, data.length));
}


// changes contents of rover
MULLE__UTF_GLOBAL
mulle_utf32_t   _mulle_utf8data_next_utf32character( struct mulle_utf8data *rover);

MULLE__UTF_GLOBAL
mulle_utf32_t   __mulle_utf8data_next_utf32character( struct mulle_utf8data *rover,
                                                      char c);

// use this to walk through a utf8 string
// changes contents of rover
// returns -1 at end (not 0)
// returns -2 if malformed
static inline mulle_utf32_t   mulle_utf8data_next_utf32character( struct mulle_utf8data *rover)
{
   char   c;

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



MULLE__UTF_GLOBAL
mulle_utf32_t   *_mulle_utf8_convert_to_utf32( char *src,
                                               size_t len,
                                               mulle_utf32_t *dst);


// low level conversion, no checks dst is assumed to be wide enough
// returns end of dst, len can't be -1
MULLE__UTF_GLOBAL
mulle_utf16_t   *_mulle_utf8_convert_to_utf16( char *src,
                                               size_t len,
                                               mulle_utf16_t *dst);

MULLE__UTF_GLOBAL
mulle_utf32_t   *_mulle_utf8_convert_to_utf32( char *src,
                                               size_t len,
                                               mulle_utf32_t *dst);
//
// You can supply a "mulle-buffer" here as "buffer" and mulle_buffer_add_bytes
// as the callback.
// These routines do not skip BOM characters. And don't check for validity.
// The input must be correct! These routines do not add a trailing zero.
//
MULLE__UTF_GLOBAL
void   mulle_utf8_bufferconvert_to_utf16( char *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t *addbytes);

// as above, but for utf32
MULLE__UTF_GLOBAL
void   mulle_utf8_bufferconvert_to_utf32( char *src,
                                          size_t len,
                                          void *buffer,
                                          mulle_utf_add_bytes_function_t *addbytes);


//
// dst should be 2 * len
// These routines do not add a trailing zero. (untested)
MULLE__UTF_GLOBAL
char   *_mulle_iso1_convert_to_utf8( char *src,
                                             size_t len,
                                             char *dst);

// as above but for macroman
MULLE__UTF_GLOBAL
char   *_mulle_macroman_convert_to_utf8( char *macroman,
                                                 size_t len,
                                                 char *dst);

// as above but for nextstep
MULLE__UTF_GLOBAL
char   *_mulle_nextstep_convert_to_utf8( char *nextstep,
                                                 size_t len,
                                                 char *dst);
//
// latin iso1 this len, will bail if it can't covert (return NULL) and unknown
// is -1. If unknown is 0, will just skip. Otherwise will replace with unknown.
// dst must be same len.
// These routines do not add a trailing zero. (untested)
MULLE__UTF_GLOBAL
char   *_mulle_utf8_convert_to_iso1( char *src,
                                     size_t len,
                                     char *dst,
                                     int unknown);

//
// data may not have a bom
//
MULLE__UTF_GLOBAL
struct mulle_utf8data
   mulle_utf8data_range_of_utf32_range( struct mulle_utf8data,
                                        struct mulle_range range);

#endif
