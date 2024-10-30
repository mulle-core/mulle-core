//
//  mulle-stacktrace.h
//  mulle-core
//
//  Created by Nat! on 28.10.18
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2018 Codeon GmbH.
//  All rights reserved.
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

#ifndef mulle_stacktrace_h__
#define mulle_stacktrace_h__


#include "include.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>


/*
 *  (c) 2018 nat
 *
 *  version:  major, minor, patch
 */
#define MULLE__STACKTRACE_VERSION  ((0UL << 20) | (2 << 8) | 9)


static inline unsigned int   mulle_stacktrace_get_version_major( void)
{
   return( MULLE__STACKTRACE_VERSION >> 20);
}


static inline unsigned int   mulle_stacktrace_get_version_minor( void)
{
   return( (MULLE__STACKTRACE_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_stacktrace_get_version_patch( void)
{
   return( MULLE__STACKTRACE_VERSION & 0xFF);
}

MULLE__STACKTRACE_GLOBAL
uint32_t   mulle_stacktrace_get_version( void);

/*
   Add other library headers here like so, for exposure to library
   consumers.

   # include "foo.h"
*/
typedef char   *(mulle_stacktrace_symbolizer_t)( void *s,
                                                 size_t max,
                                                 char *buf,
                                                 size_t len,
                                                 void **userinfo);

struct mulle_stacktrace
{
   mulle_stacktrace_symbolizer_t  *symbolize;
   char                           *(*trim_belly_fat)( char *s);
   int                            (*trim_arse_fat)( char *s);
   int                            (*is_boring)( char *s, int size);
};


MULLE__STACKTRACE_GLOBAL
void   _mulle_stacktrace_init( struct mulle_stacktrace *stacktrace,
                               mulle_stacktrace_symbolizer_t *symbolize,
                               char *(*trim_belly_fat)( char *),
                               int (*trim_arse_fat)( char *),
                               int (*is_boring)( char *, int size));

// what is used if NULL is passed for stacktrace
MULLE__STACKTRACE_GLOBAL
void   _mulle_stacktrace_init_default( struct mulle_stacktrace *stacktrace);

enum mulle_stacktrace_format
{
   mulle_stacktrace_normal   = 0,
   mulle_stacktrace_trimmed  = 1, 
   mulle_stacktrace_linefeed = 2
};

// stacktrace may be NULL
MULLE__STACKTRACE_GLOBAL
void  _mulle_stacktrace( struct mulle_stacktrace *stacktrace,
                         int offset,
                         enum mulle_stacktrace_format format,
                         FILE *fp);


static inline void   mulle_stacktrace( struct mulle_stacktrace *stacktrace, FILE *fp)
{
   _mulle_stacktrace( stacktrace, 1, mulle_stacktrace_trimmed, fp);
}


static inline void   mulle_stacktrace_once( FILE *fp)
{
   _mulle_stacktrace( NULL, 1, mulle_stacktrace_trimmed, fp);
}


int   mulle_stacktrace_count_frames( void);


#ifdef __has_include
# if __has_include( "_mulle-stacktrace-versioncheck.h")
#  include "_mulle-stacktrace-versioncheck.h"
# endif
#endif

#endif
