//
//  mulle-slug.h
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
#ifndef mulle_slug_h__
#define mulle_slug_h__

#include "include.h"

#include <stdint.h>

/*
 *  (c) 2023 nat <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE__SLUG_VERSION  ((0UL << 20) | (1 << 8) | 1)


static inline uint32_t   mulle_slug_get_version_major( void)
{
   return( MULLE__SLUG_VERSION >> 20);
}


static inline uint32_t   mulle_slug_get_version_minor( void)
{
   return( (MULLE__SLUG_VERSION >> 8) & 0xFFF);
}


static inline uint32_t   mulle_slug_get_version_patch( void)
{
   return( MULLE__SLUG_VERSION & 0xFF);
}

// mulle-c11 feature: MULLE_SLUG__GLOBAL
MULLE__SLUG_GLOBAL
uint32_t   mulle_slug_get_version( void);


//
// you get an allocated string back, that you need to mulle_free
//
MULLE__SLUG_GLOBAL
char   *mulle_utf8_slugify( char *s);

//
// you get a struct mulle_utf8data, whose characters you need to mulle_free
//
MULLE__SLUG_GLOBAL
struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                struct mulle_allocator *allocator);


// adds slugified string to buffer, useful for building up an html page
MULLE__SLUG_GLOBAL
void  mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                           struct mulle_utf8data data);
//
// You can slugify into an existing buffer. You can use an alloca buffer
// here and then get a) a max sized string and b) no mallocs
//
MULLE__SLUG_GLOBAL
void  mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer,
                                     struct mulle_utf8data data);
/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-slug-versioncheck.h")
#  include "_mulle-slug-versioncheck.h"
# endif
#endif

#endif
