/*
 *  mulle-socket-posix.c
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
#ifndef _WIN32

#include "include-private.h"

#include "mulle-socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>


/*
 * Linux suppresses SIGPIPE per send with MSG_NOSIGNAL; the BSDs and macOS have
 * no such flag and want SO_NOSIGPIPE on the socket instead. Between the two,
 * every platform this library targets can be covered -- but a platform with
 * neither cannot honour the promise mulle-socket_write makes, and finding that
 * out at build time beats finding it out when a peer hangs up.
 */
#if ! defined( MSG_NOSIGNAL) && ! defined( SO_NOSIGPIPE)
# error "this platform has neither MSG_NOSIGNAL nor SO_NOSIGPIPE, so mulle-socket cannot stop a write to a closed peer from raising SIGPIPE"
#endif


/*
 * Everything a socket needs set at the moment it is created, as opposed to at
 * the moment somebody thinks of it. The send flags in mulle_socket_write are
 * the Linux half of this; this is the macOS half.
 */
static int   _mulle_socket_configure( int fd)
{
#if defined( SO_NOSIGPIPE)
   int   on;

   on = 1;
   if( setsockopt( fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof( on)) < 0)
      return( -1);
#else
   (void) fd;   /* Linux: nothing to do, the send flag covers it */
#endif
   return( 0);
}


int   mulle_socket_init( void)
{
   return( 0);
}


int   mulle_socket_error( void)
{
   return( errno);
}


int   mulle_socket_set_reuseaddr( mulle_socket_t socket)
{
   int   on;

   on = 1;
   return( setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on)));
}


int   mulle_socket_set_nodelay( mulle_socket_t socket)
{
   int   on;

   on = 1;
   return( setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof( on)));
}


/*
 * POSIX wants a struct timeval in microseconds here, and spells "forever" as a
 * zero timeval. A timeout that rounds to nothing is nudged to a microsecond:
 * seconds >= 0 are never allowed to collide with the meaning of 0.
 */
static int   _mulle_socket_set_timeout( int fd, int option, double seconds)
{
   struct timeval   value;

   if( seconds <= 0.0)
   {
      value.tv_sec  = 0;
      value.tv_usec = 0;
   }
   else
   {
      value.tv_sec  = (time_t) seconds;
      value.tv_usec = (suseconds_t) ((seconds - (double) value.tv_sec) * 1e6 + 0.5);
      if( ! value.tv_sec && ! value.tv_usec)
         value.tv_usec = 1;
   }

   return( setsockopt( fd, SOL_SOCKET, option, &value, sizeof( value)));
}


int   mulle_socket_set_read_timeout( mulle_socket_t socket, double seconds)
{
   return( _mulle_socket_set_timeout( socket, SO_RCVTIMEO, seconds));
}


int   mulle_socket_set_write_timeout( mulle_socket_t socket, double seconds)
{
   return( _mulle_socket_set_timeout( socket, SO_SNDTIMEO, seconds));
}


/*
 * A blocking read on a socket with no timeout does not fail this way -- it
 * waits. So EAGAIN here is the expiry of SO_RCVTIMEO and nothing else, and it
 * is reported as the ETIMEDOUT that Windows would have said, because this
 * library opens no non-blocking sockets and a caller should get one answer.
 */
static void   _mulle_socket_name_timeout( void)
{
   if( errno == EAGAIN || errno == EWOULDBLOCK)
      errno = ETIMEDOUT;
}


mulle_socket_t   mulle_socket_listen( char const *address,
                                      unsigned int port,
                                      unsigned int *p_port)
{
   struct sockaddr_in   addr;
   struct sockaddr_in   bound;
   socklen_t            length;
   int                  fd;

   memset( &addr, 0, sizeof( addr));
   addr.sin_family = AF_INET;
   addr.sin_port   = htons( (uint16_t) port);

   if( address)
   {
      if( inet_pton( AF_INET, address, &addr.sin_addr) != 1)
      {
         errno = EINVAL;
         return( MULLE_SOCKET_INVALID);
      }
   }
   else
      addr.sin_addr.s_addr = htonl( INADDR_ANY);

   fd = socket( AF_INET, SOCK_STREAM, 0);
   if( fd < 0)
      return( MULLE_SOCKET_INVALID);

   if( _mulle_socket_configure( fd) < 0 ||
       mulle_socket_set_reuseaddr( fd) < 0 ||
       bind( fd, (struct sockaddr *) &addr, sizeof( addr)) < 0 ||
       listen( fd, SOMAXCONN) < 0)
   {
      close( fd);
      return( MULLE_SOCKET_INVALID);
   }

   if( p_port)
   {
      length = sizeof( bound);
      if( getsockname( fd, (struct sockaddr *) &bound, &length) < 0)
      {
         close( fd);
         return( MULLE_SOCKET_INVALID);
      }
      *p_port = ntohs( bound.sin_port);
   }

   return( fd);
}


mulle_socket_t   mulle_socket_accept( mulle_socket_t listener)
{
   int   fd;

   for(;;)
   {
      fd = accept( listener, NULL, NULL);
      if( fd >= 0)
         break;
      if( errno != EINTR)
      {
         /*
          * Linux applies a receive timeout to accept and reports it as EAGAIN,
          * which is named here as the ETIMEDOUT the read and write paths
          * report. On macOS and the BSDs this never fires, because their accept
          * ignores the option entirely and simply waits; see the header.
          */
         _mulle_socket_name_timeout();
         return( MULLE_SOCKET_INVALID);
      }
   }

   if( _mulle_socket_configure( fd) < 0 ||
       mulle_socket_set_nodelay( fd) < 0)
   {
      close( fd);
      return( MULLE_SOCKET_INVALID);
   }

   return( fd);
}


mulle_socket_t   mulle_socket_connect( char const *host, unsigned int port)
{
   struct addrinfo   hints;
   struct addrinfo   *addresses;
   struct addrinfo   *q;
   char              service[ 16];
   int               fd;
   int               rval;

   memset( &hints, 0, sizeof( hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   snprintf( service, sizeof( service), "%u", port);

   addresses = NULL;
   rval      = getaddrinfo( host, service, &hints, &addresses);
   if( rval != 0)
   {
      /*
       * getaddrinfo reports its own EAI_ codes rather than setting errno, so
       * the few that a caller might branch on are turned into the errno that
       * means the same thing. Anything else becomes EHOSTUNREACH, which is true
       * enough: the name did not lead to an address we could use.
       */
      if( rval == EAI_SYSTEM)
         return( MULLE_SOCKET_INVALID);

      errno = (rval == EAI_MEMORY) ? ENOMEM : EHOSTUNREACH;
      return( MULLE_SOCKET_INVALID);
   }

   fd = -1;
   for( q = addresses; q; q = q->ai_next)
   {
      fd = socket( q->ai_family, q->ai_socktype, q->ai_protocol);
      if( fd < 0)
         continue;

      if( _mulle_socket_configure( fd) < 0)
      {
         close( fd);
         fd = -1;
         continue;
      }

      for(;;)
      {
         rval = connect( fd, q->ai_addr, q->ai_addrlen);
         if( rval == 0 || errno != EINTR)
            break;
      }

      if( rval == 0)
      {
         if( mulle_socket_set_nodelay( fd) < 0)
            break;      /* connected, but unusable: report it as a failure */

         freeaddrinfo( addresses);
         return( fd);
      }

      close( fd);
      fd = -1;
   }

   freeaddrinfo( addresses);
   return( MULLE_SOCKET_INVALID);
}


unsigned int   mulle_socket_find_free_port( void)
{
   struct sockaddr_in   addr;
   socklen_t            length;
   unsigned int         port;
   int                  fd;

   fd = socket( AF_INET, SOCK_STREAM, 0);
   if( fd < 0)
      return( 0);

   memset( &addr, 0, sizeof( addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK);
   addr.sin_port        = 0;

   if( bind( fd, (struct sockaddr *) &addr, sizeof( addr)) < 0)
   {
      close( fd);
      return( 0);
   }

   length = sizeof( addr);
   if( getsockname( fd, (struct sockaddr *) &addr, &length) < 0)
   {
      close( fd);
      return( 0);
   }

   port = ntohs( addr.sin_port);
   close( fd);

   return( port);
}


intptr_t   mulle_socket_read( mulle_socket_t socket,
                              void *buffer,
                              size_t length)
{
   ssize_t   n;

   for(;;)
   {
      n = read( socket, buffer, length);
      if( n >= 0)
         return( (intptr_t) n);
      if( errno != EINTR)
      {
         _mulle_socket_name_timeout();
         return( -1);
      }
   }
}


intptr_t   mulle_socket_write( mulle_socket_t socket,
                               void const *bytes,
                               size_t length)
{
   ssize_t   n;
   int       flags;

#if defined( MSG_NOSIGNAL)
   flags = MSG_NOSIGNAL;
#else
   flags = 0;      /* _mulle_socket_configure() asked the socket for the same */
#endif

   for(;;)
   {
      n = send( socket, bytes, length, flags);
      if( n >= 0)
         return( (intptr_t) n);
      if( errno != EINTR)
      {
         _mulle_socket_name_timeout();
         return( -1);
      }
   }
}


int   mulle_socket_shutdown( mulle_socket_t socket)
{
   if( socket == MULLE_SOCKET_INVALID)
      return( 0);

   return( shutdown( socket, SHUT_WR));
}


int   mulle_socket_close( mulle_socket_t socket)
{
   if( socket == MULLE_SOCKET_INVALID)
      return( 0);

   return( close( socket));
}


mulle_socket_t   mulle_socket_listen_unix( char const *path)
{
   struct sockaddr_un   addr;
   size_t               length;
   int                  fd;

   length = strlen( path);
   if( length >= sizeof( addr.sun_path))
   {
      errno = ENAMETOOLONG;
      return( MULLE_SOCKET_INVALID);
   }

   fd = socket( AF_UNIX, SOCK_STREAM, 0);
   if( fd < 0)
      return( MULLE_SOCKET_INVALID);

   memset( &addr, 0, sizeof( addr));
   addr.sun_family = AF_UNIX;
   memcpy( addr.sun_path, path, length + 1);

   if( _mulle_socket_configure( fd) < 0 ||
       bind( fd, (struct sockaddr *) &addr, sizeof( addr)) < 0 ||
       listen( fd, SOMAXCONN) < 0)
   {
      close( fd);
      return( MULLE_SOCKET_INVALID);
   }

   return( fd);
}


mulle_socket_t   mulle_socket_connect_unix( char const *path)
{
   struct sockaddr_un   addr;
   size_t               length;
   int                  fd;
   int                  rval;

   length = strlen( path);
   if( length >= sizeof( addr.sun_path))
   {
      errno = ENAMETOOLONG;
      return( MULLE_SOCKET_INVALID);
   }

   fd = socket( AF_UNIX, SOCK_STREAM, 0);
   if( fd < 0)
      return( MULLE_SOCKET_INVALID);

   if( _mulle_socket_configure( fd) < 0)
   {
      close( fd);
      return( MULLE_SOCKET_INVALID);
   }

   memset( &addr, 0, sizeof( addr));
   addr.sun_family = AF_UNIX;
   memcpy( addr.sun_path, path, length + 1);

   for(;;)
   {
      rval = connect( fd, (struct sockaddr *) &addr, sizeof( addr));
      if( rval == 0 || errno != EINTR)
         break;
   }

   if( rval < 0)
   {
      close( fd);
      return( MULLE_SOCKET_INVALID);
   }

   return( fd);
}

#endif
