/*
 *  mulle_utf32-string.h
 *  MulleRegularExpressions
 *
 *  Created by Nat! on 09.11.11.
//  Copyright (C) 2011 Nat!, Mulle kybernetiK.
//  Copyright (c) 2011 Codeon GmbH.
//  All rights reserved.
 *
 */
#ifndef mulle_utf32_string_h__
#define mulle_utf32_string_h__

#include "mulle-utf-type.h"

#include "mulle-utf32.h"

#include <stddef.h>
#include <assert.h>
#include <string.h>


MULLE__UTF_GLOBAL
unsigned int  mulle_utf32_strnlen( mulle_utf32_t *src, unsigned int len);

MULLE__UTF_GLOBAL
mulle_utf32_t  *mulle_utf32_strdup( mulle_utf32_t *s);

MULLE__UTF_GLOBAL
mulle_utf32_t  *mulle_utf32_strncpy( mulle_utf32_t *dst, unsigned int len, mulle_utf32_t *src);

MULLE__UTF_GLOBAL
mulle_utf32_t  *mulle_utf32_strchr( mulle_utf32_t *s, mulle_utf32_t c);

MULLE__UTF_GLOBAL
int            _mulle_utf32_atoi( mulle_utf32_t **s);

MULLE__UTF_GLOBAL
mulle_utf32_t  *mulle_utf32_strstr( mulle_utf32_t *s1, mulle_utf32_t *s2);

MULLE__UTF_GLOBAL
int             mulle_utf32_strncmp( mulle_utf32_t *s1, mulle_utf32_t *s2, unsigned int len);

MULLE__UTF_GLOBAL
unsigned int          mulle_utf32_strspn( mulle_utf32_t *s1, mulle_utf32_t *s2);

MULLE__UTF_GLOBAL
unsigned int          mulle_utf32_strcspn( mulle_utf32_t *s1, mulle_utf32_t *s2);


static inline int      mulle_utf32_strcmp( mulle_utf32_t *s1, mulle_utf32_t *s2)
{
   return( mulle_utf32_strncmp( s1, s2, -1));
}

static inline int   mulle_utf32_atoi( mulle_utf32_t *s)
{
   return( _mulle_utf32_atoi( &s));
}


static inline void   mulle_utf32_memcpy( mulle_utf32_t *dst, mulle_utf32_t *src, unsigned int len)
{
   memcpy( dst, src, sizeof( mulle_utf32_t) * len);
}


static inline void   mulle_utf32_memmove( mulle_utf32_t *dst, mulle_utf32_t *src, unsigned int len)
{
   memmove( dst, src, sizeof( mulle_utf32_t) * len);
}


#endif
