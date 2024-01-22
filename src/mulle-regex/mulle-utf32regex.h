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
struct mulle_utf32regex   *mulle_utf32regex_compile( mulle_utf32_t *pattern);

static inline void   mulle_utf32regex_free( struct mulle_utf32regex *regex)
{
   mulle_free( regex);
}


/* returns < 0 on failure, 1 on match, 0 on no match */
int   mulle_utf32regex_execute( struct mulle_utf32regex *regex,
                                mulle_utf32_t *src);

/* returns < 0 on failure, 0 otherwise
   mulle_utf32regex_substitute, does not truncate. If the output buffer is too
   small, its an error.
   Does not append a trailing zero, unless specified by `zero`. `dst_len` must
   then be + 1
 */
int   mulle_utf32regex_substitute( struct mulle_utf32regex *regex,
                                   mulle_utf32_t *replacement,
                                   mulle_utf32_t *dst,
                                   unsigned int dst_len,
                                   int zero);



// returns malloced buffer, or NULL
// convenience for doing compile/match in one step
//
mulle_utf32_t   *mulle_utf32_match( mulle_utf32_t *pattern, mulle_utf32_t *src);

// returns malloced buffer, or NULL
// convenience for doing compile/substitute in one step
//
mulle_utf32_t   *mulle_utf32_substitute( mulle_utf32_t *pattern,
                                         mulle_utf32_t *replacement,
                                         mulle_utf32_t *src);

// Length of the string that will be substituted in the matched part of the
// string. This does _not_ include the trailing zero, that
// mulle_utf32_substitute will add! See mulle_utf32regex_substitution_buffer_size
// for the space you should malloc. Will return (unsigned int) -1 on error.
//
// You need to add the front and back part yourself.
unsigned int   mulle_utf32regex_substitution_length( struct mulle_utf32regex *regex,
                                                     mulle_utf32_t *replacement);

// number of bytes(!) needed for the substitution buffer
static inline size_t
   mulle_utf32regex_substitution_buffer_size( struct mulle_utf32regex *regex,
                                              mulle_utf32_t *replacement)
{
   unsigned int   length;

   length = mulle_utf32regex_substitution_length( regex, replacement);
   return( (length + 1) * sizeof( mulle_utf32_t));
}


//
// use 0 to get range of matched string
// use 1-9 for \1 to \9
//
struct mulle_range   mulle_utf32regex_range_for_index( struct mulle_utf32regex *regex, unsigned int i);

#ifdef __has_include
# if __has_include( "_mulle-regex-versioncheck.h")
#  include "_mulle-regex-versioncheck.h"
# endif
#endif


#endif

/* extern void regerror(char *message); */
