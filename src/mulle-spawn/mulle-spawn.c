/*
 *  mulle-spawn.c
 *  mulle-spawn
 *
 *  Copyright (c) 2026 Nat! - Mulle kybernetiK.
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  Neither the name of Mulle kybernetiK nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "include-private.h"

#include "mulle-spawn.h"

#include <string.h>


uint32_t   mulle_spawn_get_version( void)
{
   return( MULLE__SPAWN_VERSION);
}


/*
 * Shared by both platform backends, which is why it lives here and not in one
 * of them: a helper in a platform file is a helper the other platform does not
 * have.
 *
 * `capacity` is the size of the `output` allocation and is carried by the
 * caller, because it is not part of the result. It doubles, so a capture of n
 * bytes costs O( n) copies and not O( n^2).
 */
int   _mulle_spawn_append( struct mulle_spawn_result *p,
                           size_t *capacity,
                           char *bytes,
                           size_t length)
{
   char     *q;
   size_t   size;

   if( p->length + length + 1 > *capacity)
   {
      size = *capacity ? *capacity : (p->length + length + 1);
      while( size < p->length + length + 1)
         size *= 2;

      q = mulle_allocator_realloc( p->allocator, p->output, size);
      if( ! q)
         return( -1);

      p->output = q;
      *capacity = size;
   }

   memcpy( p->output + p->length, bytes, length);
   p->length            += length;
   p->output[ p->length] = 0;
   return( 0);
}


void   mulle_spawn_result_done( struct mulle_spawn_result *p)
{
   struct mulle_allocator   *allocator;

   if( ! p)
      return;

   if( p->output)
   {
      allocator = p->allocator;
      mulle_allocator_free( allocator, p->output);
   }
   _mulle_spawn_result_init( p, p->allocator);
}
