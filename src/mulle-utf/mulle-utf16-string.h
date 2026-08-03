//
//  mulle-utf16-string.h
//  mulle-utf
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2011 Codeon GmbH.
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
#ifndef mulle_utf16_string_h__
#define mulle_utf16_string_h__

#include "mulle-utf-type.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "mulle-utf16.h"


MULLE__UTF_GLOBAL
size_t  mulle_utf16_strnlen( mulle_utf16_t *src, size_t len);

MULLE__UTF_GLOBAL
mulle_utf16_t  *mulle_utf16_strdup( mulle_utf16_t *s);

MULLE__UTF_GLOBAL
mulle_utf16_t  *mulle_utf16_strncpy( mulle_utf16_t *dst, size_t len, mulle_utf16_t *src);

MULLE__UTF_GLOBAL
int             mulle_utf16_strncmp( mulle_utf16_t *s1, mulle_utf16_t *s2, size_t len);

MULLE__UTF_GLOBAL
mulle_utf16_t  *mulle_utf16_strchr( mulle_utf16_t *s, mulle_utf32_t c);  // sic

MULLE__UTF_GLOBAL
int            _mulle_utf16_atoi( mulle_utf16_t **s);

MULLE__UTF_GLOBAL
mulle_utf16_t  *mulle_utf16_strstr( mulle_utf16_t *s1, mulle_utf16_t *s2);

MULLE__UTF_GLOBAL
size_t    mulle_utf16_strspn( mulle_utf16_t *s1, mulle_utf16_t *s2);

MULLE__UTF_GLOBAL
size_t    mulle_utf16_strcspn( mulle_utf16_t *s1, mulle_utf16_t *s2);


static inline int   mulle_utf16_strcmp( mulle_utf16_t *s1, mulle_utf16_t *s2)

{
   return( mulle_utf16_strncmp( s1, s2, mulle_utf16_strlen( s2)));
}


static inline int   mulle_utf16_atoi( mulle_utf16_t *s)
{
   return( _mulle_utf16_atoi( &s));
}



static inline void   mulle_utf16_memcpy( mulle_utf16_t *dst, mulle_utf16_t *src, size_t len)
{
   memcpy( dst, src, sizeof( mulle_utf16_t) * len);
}


static inline void   mulle_utf16_memmove( mulle_utf16_t *dst, mulle_utf16_t *src, size_t len)
{
   memmove( dst, src, sizeof( mulle_utf16_t) * len);
}

#endif
