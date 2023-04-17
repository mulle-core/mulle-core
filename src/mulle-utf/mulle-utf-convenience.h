//
//  mulle_utf_convenience.h
//  mulle-utf
//
//  Created by Nat! on 30.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_utf_convenience_h__
#define mulle_utf_convenience_h__

// pollute mulle_utf with mulle_allocator ? but it's just so.. convenient!

#include "include.h"
#include "mulle-utf-type.h"
#include "mulle-utf8.h"
#include "mulle-utf16.h"
#include "mulle-utf32.h"


struct mulle_utf8_conversion_context
{
   mulle_utf8_t   *buf;
   mulle_utf8_t   *sentinel;
};


struct mulle_utf16_conversion_context
{
   mulle_utf16_t   *buf;
   mulle_utf16_t   *sentinel;
};


struct mulle_utf32_conversion_context
{
   mulle_utf32_t   *buf;
   mulle_utf32_t   *sentinel;
};


// bytes will be properly aligned, len is in bytes not characters!
MULLE_UTF_GLOBAL
void  mulle_utf8_conversion_context_add_bytes( void *p,
                                               void *bytes,
                                               size_t len);
MULLE_UTF_GLOBAL
void  mulle_utf16_conversion_context_add_bytes( void *p,
                                                void *bytes,
                                                size_t len);
MULLE_UTF_GLOBAL
void  mulle_utf32_conversion_context_add_bytes( void *p,
                                                void *bytes,
                                                size_t len);
// these convenience routines always append a \0
// the returned string is allocated with the allocator, which may be NULL

MULLE_UTF_GLOBAL
mulle_utf16_t   *mulle_utf8_convert_to_utf16_string( mulle_utf8_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator);
MULLE_UTF_GLOBAL
mulle_utf32_t   *mulle_utf8_convert_to_utf32_string( mulle_utf8_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator);

MULLE_UTF_GLOBAL
mulle_utf8_t    *mulle_utf16_convert_to_utf8_string( mulle_utf16_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator);
MULLE_UTF_GLOBAL
mulle_utf32_t   *mulle_utf16_convert_to_utf32_string( mulle_utf16_t *src,
                                                      size_t len,
                                                      struct mulle_allocator *allocator);

MULLE_UTF_GLOBAL
mulle_utf8_t    *mulle_utf32_convert_to_utf8_string( mulle_utf32_t *src,
                                                     size_t len,
                                                     struct mulle_allocator *allocator);
MULLE_UTF_GLOBAL
mulle_utf16_t   *mulle_utf32_convert_to_utf16_string( mulle_utf32_t *src,
                                                      size_t len,
                                                      struct mulle_allocator *allocator);



struct mulle_utf_mogrification_info
{
     mulle_utf32_t (*f1_conversion)( mulle_utf32_t);
     mulle_utf32_t (*f2_conversion)( mulle_utf32_t);
     int           (*is_white)( mulle_utf32_t);
};

typedef int   mulle_utf8_mogrifier_function_t( struct mulle_utf8data *dst,
                                               struct mulle_utf8data *src,
                                               struct mulle_utf_mogrification_info *info);
typedef int   mulle_utf16_mogrifier_function_t( struct mulle_utf32data *dst,
                                                struct mulle_utf16data *src,
                                                struct mulle_utf_mogrification_info *info);
typedef int   mulle_utf32_mogrifier_function_t( struct mulle_utf32data *dst,
                                                struct mulle_utf32data *src,
                                                struct mulle_utf_mogrification_info *info);

//
// This will return 0, if no actual conversion took place, will return a
// -1 if dst was exhausted. src can't be same as dst.
// The value of dst->length will be adjusted to the actual length.
//  The incoming data must be sane.
//
MULLE_UTF_GLOBAL
int   _mulle_utf8_character_mogrify( struct mulle_utf8data *dst,
                                     struct mulle_utf8data *src,
                                     struct mulle_utf_mogrification_info *info);
MULLE_UTF_GLOBAL
int   _mulle_utf16_character_mogrify( struct mulle_utf32data *dst,
                                      struct mulle_utf16data *src,
                                      struct mulle_utf_mogrification_info *info);

MULLE_UTF_GLOBAL
int   _mulle_utf16_character_mogrify_unsafe( struct mulle_utf16data *dst,
                                             struct mulle_utf16data *src,
                                             struct mulle_utf_mogrification_info *info);
//
// This will return 0, if no actual conversion took place, will return a
// -1 if dst is too small. Here src can be the same as dst.
// The value of dst->length will be adjusted to the actual length, which
// is the same as src->length. The incoming data must be sane.
//
MULLE_UTF_GLOBAL
int   _mulle_utf32_character_mogrify( struct mulle_utf32data *dst,
                                      struct mulle_utf32data *src,
                                      struct mulle_utf_mogrification_info *info);

MULLE_UTF_GLOBAL
int   _mulle_utf8_word_mogrify( struct mulle_utf8data *dst,
                                struct mulle_utf8data *src,
                                struct mulle_utf_mogrification_info *info);

// different! converts utf16 data to utf32
MULLE_UTF_GLOBAL
int   _mulle_utf16_word_mogrify( struct mulle_utf32data *dst,
                                 struct mulle_utf16data *src,
                                 struct mulle_utf_mogrification_info *info);

MULLE_UTF_GLOBAL
int   _mulle_utf32_word_mogrify( struct mulle_utf32data *dst,
                                 struct mulle_utf32data *src,
                                 struct mulle_utf_mogrification_info *info);


#if MULLE_ALLOCATOR_VERSION  < ((1 << 20) | (5 << 8) | 0)
# error "mulle-allocator is too old"
#endif

#endif /* mulle_utf_convenience_h */
