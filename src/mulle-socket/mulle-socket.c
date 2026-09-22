/*
 *  mulle-socket.c
 *  mulle-socket
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

#include "mulle-socket.h"


uint32_t   mulle_socket_get_version( void)
{
   return( MULLE__SOCKET_VERSION);
}


/*
 * Shared by both platform backends, which is why it lives here and not in one
 * of them: a helper in a platform file is a helper the other platform does not
 * have.
 *
 * The loop is the whole point of it -- a blocking write may take only part of
 * the buffer, and code that assumes otherwise works until the day the peer is
 * slow, then silently loses the tail of a message.
 */
int   mulle_socket_write_all( mulle_socket_t socket,
                              void const *bytes,
                              size_t length)
{
   char const   *p;
   intptr_t     n;

   p = bytes;
   while( length)
   {
      n = mulle_socket_write( socket, p, length);
      if( n < 0)
         return( -1);

      /*
       * Neither platform's send returns 0 for a non-empty buffer, so this
       * branch is unreachable in practice. It exists because the alternative
       * when it is reached -- a loop that spins forever on a peer that will
       * never take another byte -- is worse than an error. There is no platform
       * error to report here, so the error value stays whatever it was.
       */
      if( n == 0)
         return( -1);

      p      += n;
      length -= (size_t) n;
   }

   return( 0);
}
