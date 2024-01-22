/*
 *  regexp+private.h
 *  MulleRegularExpressions
 *
 *  Created by Nat! on 09.11.11.
 *  Copyright 2011 Mulle kybernetiK. All rights reserved.
 *
 */

/*
 * The first mulle_utf32_t of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MULLE_UTF32REGEX_MAGIC	0x4818  /* carefully chosen */


/* test 122 will fail if this isn't exactly 10 */
#define MULLE_UTF32REGEX_NSUBEXP   10  /* ((CLOSE - OPEN) / 2)  max: 122 */

/* obvious problem:  index 0 is never used */
typedef struct mulle_utf32regex
{
   mulle_utf32_t   *startp[ MULLE_UTF32REGEX_NSUBEXP];
   mulle_utf32_t   *endp[ MULLE_UTF32REGEX_NSUBEXP];
   mulle_utf32_t   *start;
   mulle_utf32_t   regstart;
   mulle_utf32_t   *regmust;
   int             regmlen;
   short           reganch;
   char            program[ 1];
} regexp;


void   mulle_utf32regex_dump( struct mulle_utf32regex *r);

