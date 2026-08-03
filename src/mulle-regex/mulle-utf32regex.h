//
//  mulle-utf32regex.h
//  mulle-regex
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
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
/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef mulle_utf32regex_h__
#define mulle_utf32regex_h__

#include "include.h"
#include <stdlib.h>


struct mulle_utf32regex;

// think of the parameters as
//
//   dst = `echo src | sed  's/pattern/replacement/'
//

/* may return NULL on failure (malformed regular expression) */
MULLE__REGEX_GLOBAL
struct mulle_utf32regex   *mulle_utf32regex_compile( mulle_utf32_t *pattern);

static inline void   mulle_utf32regex_free( struct mulle_utf32regex *regex)
{
   mulle_free( regex);
}


/* returns < 0 on failure, 1 on match, 0 on no match */
MULLE__REGEX_GLOBAL
int   mulle_utf32regex_execute( struct mulle_utf32regex *regex,
                                mulle_utf32_t *src);

/* returns < 0 on failure, 0 otherwise
   mulle_utf32regex_substitute, does not truncate. If the output buffer is too
   small, its an error.
   Does not append a trailing zero, unless specified by `zero`. `dst_len` must
   then be + 1
 */
MULLE__REGEX_GLOBAL
int   mulle_utf32regex_substitute( struct mulle_utf32regex *regex,
                                   mulle_utf32_t *replacement,
                                   mulle_utf32_t *dst,
                                   size_t dst_len,
                                   int zero);



// returns malloced buffer, or NULL
// convenience for doing compile/match in one step
//
MULLE__REGEX_GLOBAL
mulle_utf32_t   *mulle_utf32_match( mulle_utf32_t *pattern, mulle_utf32_t *src);

// returns malloced buffer, or NULL
// convenience for doing compile/substitute in one step
//
MULLE__REGEX_GLOBAL
mulle_utf32_t   *mulle_utf32_substitute( mulle_utf32_t *pattern,
                                         mulle_utf32_t *replacement,
                                         mulle_utf32_t *src);

// Length of the string that will be substituted in the matched part of the
// string. This does _not_ include the trailing zero, that
// mulle_utf32_substitute will add! See mulle_utf32regex_substitution_buffer_size
// for the space you should malloc. Will return (unsigned int) -1 on error.
//
// You need to add the front and back part yourself.
MULLE__REGEX_GLOBAL
size_t   mulle_utf32regex_substitution_length( struct mulle_utf32regex *regex,
                                               mulle_utf32_t *replacement);

// number of bytes(!) needed for the substitution buffer
static inline size_t
   mulle_utf32regex_substitution_buffer_size( struct mulle_utf32regex *regex,
                                              mulle_utf32_t *replacement)
{
   size_t   length;

   length = mulle_utf32regex_substitution_length( regex, replacement);
   return( (length + 1) * sizeof( mulle_utf32_t));
}


//
// use 0 to get range of matched string
// use 1-9 for \1 to \9
//
MULLE__REGEX_GLOBAL
struct mulle_range   mulle_utf32regex_range_for_index( struct mulle_utf32regex *regex, unsigned int i);

#ifdef __has_include
# if __has_include( "_mulle-regex-versioncheck.h")
#  include "_mulle-regex-versioncheck.h"
# endif
#endif


#endif

/* extern void regerror(char *message); */
