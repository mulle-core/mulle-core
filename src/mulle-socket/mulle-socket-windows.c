/*
 *  mulle-socket-windows.c
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
//
// inet_addr is used deliberately (see mulle_socket_listen): it is declared by
// every winsock SDK, while InetPton needs a newer _WIN32_WINNT. Silence the
// MSVC deprecation warning instead of rewriting the call.
//
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
# define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#ifdef _WIN32

#include "include-private.h"

#include "mulle-socket.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>


static int   _mulle_socket_started;


/*
 * Winsock has to be started, and starting it twice is harmless: it is reference
 * counted and this library never calls WSACleanup, so the count goes up and the
 * process goes down with sockets still open, which is what the platform expects.
 * Two threads racing here can therefore both call WSAStartup and both succeed,
 * which is why this is a plain flag and not a lock -- the worst case is a
 * redundant call, not a broken state.
 */
int   mulle_socket_init( void)
{
   WSADATA   data;

   if( _mulle_socket_started)
      return( 0);

   if( WSAStartup( MAKEWORD( 2, 2), &data))
      return( -1);

   _mulle_socket_started = 1;
   return( 0);
}


/*
 * Winsock reports errors as WSAE codes; POSIX reports them as errno values, and
 * callers of this library write one test that has to work on both. Every code
 * with a POSIX twin is translated to it; the rest become EIO, which is the
 * honest answer for "this failed in a way POSIX has no name for".
 */
static int   _mulle_socket_map_error( int code)
{
   switch( code)
   {
   case 0                  : return( 0);

   case WSAEINTR           : return( EINTR);
   case WSAEWOULDBLOCK     : return( EAGAIN);
   case WSAEINPROGRESS     : return( EINPROGRESS);
   case WSAEALREADY        : return( EALREADY);
   case WSAEISCONN         : return( EISCONN);
   case WSAENOTCONN        : return( ENOTCONN);
   case WSAECONNRESET      : return( ECONNRESET);
   case WSAECONNABORTED    : return( ECONNABORTED);
   case WSAECONNREFUSED    : return( ECONNREFUSED);
   case WSAESHUTDOWN       : return( EPIPE);
   case WSAEADDRINUSE      : return( EADDRINUSE);
   case WSAEADDRNOTAVAIL   : return( EADDRNOTAVAIL);
   case WSAEAFNOSUPPORT    : return( EAFNOSUPPORT);
   case WSAENETDOWN        : return( ENETDOWN);
   case WSAENETUNREACH     : return( ENETUNREACH);
   case WSAENETRESET       : return( ENETRESET);
   case WSAEHOSTUNREACH    : return( EHOSTUNREACH);
   case WSAETIMEDOUT       : return( ETIMEDOUT);
   case WSAEMSGSIZE        : return( EMSGSIZE);
   case WSAENOBUFS         : return( ENOBUFS);
   case WSAENOTSOCK        : return( ENOTSOCK);
   case WSAEOPNOTSUPP      : return( EOPNOTSUPP);
   case WSAEACCES          : return( EACCES);
   case WSAEINVAL          : return( EINVAL);
   case WSAEMFILE          : return( EMFILE);
   case WSAEPROTOTYPE      : return( EPROTOTYPE);
   case WSAEPROTONOSUPPORT : return( EPROTONOSUPPORT);

   default                 : return( EIO);
   }
}


int   mulle_socket_error( void)
{
   return( _mulle_socket_map_error( WSAGetLastError()));
}


/*
 * Deliberately not SO_REUSEADDR.
 *
 * On POSIX that option means "let me rebind a port that is still in TIME_WAIT",
 * which is what every restartable server wants. On Windows the same option
 * means "let *anyone* bind this port, including a process that is not me" --
 * it hands the port to whoever asks next, which is the opposite of what a
 * server wants and a known way to lose a listen socket to a hostile program.
 *
 * Nothing is lost by declining: Windows does not put a listening port into
 * TIME_WAIT the way POSIX does, so a server that closes and immediately rebinds
 * already works there without asking. So this is a no-op that returns success,
 * rather than an option that is set to a different meaning on one platform.
 */
int   mulle_socket_set_reuseaddr( mulle_socket_t socket)
{
   (void) socket;
   return( 0);
}


int   mulle_socket_set_nodelay( mulle_socket_t socket)
{
   int   on;

   on = 1;
   return( setsockopt( socket, IPPROTO_TCP, TCP_NODELAY,
                       (char const *) &on, sizeof( on)));
}


/*
 * The same option as on POSIX, with two differences that are the reason the
 * header exists: the value is a DWORD of milliseconds rather than a struct
 * timeval, and Windows reports the expiry as WSAETIMEDOUT rather than EAGAIN,
 * which already arrives here as ETIMEDOUT. Both are settled behind this call.
 *
 * A positive timeout that would round to 0 ms becomes 1 ms, because 0 ms on
 * Windows means "no timeout" exactly as a zero timeval does on POSIX.
 *
 * There is deliberately no accept() counterpart to the POSIX side's
 * _mulle_socket_name_timeout(), because the option does not reach accept here:
 * an accept on a listener carrying SO_RCVTIMEO blocks until a client arrives,
 * however small the value. That was measured rather than assumed -- the probe
 * is in probe/timeout-probe.c, and it hangs on Windows -- and it is why the
 * header says a bounded accept on Windows is a select() loop.
 * macOS behaves the same way, Linux does not; the difference is in the header
 * rather than papered over here.
 */
static int   _mulle_socket_set_timeout( SOCKET socket, int option, double seconds)
{
   DWORD   value;

   if( seconds <= 0.0)
      value = 0;
   else
   {
      value = (DWORD) (seconds * 1000.0 + 0.5);
      if( ! value)
         value = 1;
   }

   return( setsockopt( socket, SOL_SOCKET, option,
                       (char const *) &value, sizeof( value)));
}


int   mulle_socket_set_read_timeout( mulle_socket_t socket, double seconds)
{
   return( _mulle_socket_set_timeout( socket, SO_RCVTIMEO, seconds));
}


int   mulle_socket_set_write_timeout( mulle_socket_t socket, double seconds)
{
   return( _mulle_socket_set_timeout( socket, SO_SNDTIMEO, seconds));
}


mulle_socket_t   mulle_socket_listen( char const *address,
                                      unsigned int port,
                                      unsigned int *p_port)
{
   struct sockaddr_in   addr;
   struct sockaddr_in   bound;
   int                  length;
   SOCKET               fd;
   unsigned long        ip;

   if( mulle_socket_init() < 0)
      return( MULLE_SOCKET_INVALID);

   memset( &addr, 0, sizeof( addr));
   addr.sin_family = AF_INET;
   addr.sin_port   = htons( (u_short) port);

   if( address)
   {
      /*
       * inet_addr rather than the modern InetPton: it is the one conversion
       * every winsock SDK declares unconditionally, and the addresses this
       * takes are the literal dotted quads a server binds to. Names are the
       * client's problem, and getaddrinfo handles those properly.
       */
      ip = inet_addr( address);
      if( ip == INADDR_NONE)
      {
         WSASetLastError( WSAEINVAL);
         return( MULLE_SOCKET_INVALID);
      }
      addr.sin_addr.s_addr = ip;
   }
   else
      addr.sin_addr.s_addr = htonl( INADDR_ANY);

   fd = socket( AF_INET, SOCK_STREAM, 0);
   if( fd == INVALID_SOCKET)
      return( MULLE_SOCKET_INVALID);

   if( bind( fd, (struct sockaddr *) &addr, sizeof( addr)) == SOCKET_ERROR ||
       listen( fd, SOMAXCONN) == SOCKET_ERROR)
   {
      closesocket( fd);
      return( MULLE_SOCKET_INVALID);
   }

   if( p_port)
   {
      length = sizeof( bound);
      if( getsockname( fd, (struct sockaddr *) &bound, &length) == SOCKET_ERROR)
      {
         closesocket( fd);
         return( MULLE_SOCKET_INVALID);
      }
      *p_port = ntohs( bound.sin_port);
   }

   return( fd);
}


mulle_socket_t   mulle_socket_accept( mulle_socket_t listener)
{
   SOCKET   fd;

   for(;;)
   {
      fd = accept( listener, NULL, NULL);
      if( fd != INVALID_SOCKET)
         break;
      if( WSAGetLastError() != WSAEINTR)
         return( MULLE_SOCKET_INVALID);
   }

   if( mulle_socket_set_nodelay( fd) < 0)
   {
      closesocket( fd);
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
   SOCKET            fd;
   int               rval;

   if( mulle_socket_init() < 0)
      return( MULLE_SOCKET_INVALID);

   memset( &hints, 0, sizeof( hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   snprintf( service, sizeof( service), "%u", port);

   addresses = NULL;
   if( getaddrinfo( host, service, &hints, &addresses))
   {
      WSASetLastError( WSAEHOSTUNREACH);
      return( MULLE_SOCKET_INVALID);
   }

   fd = INVALID_SOCKET;
   for( q = addresses; q; q = q->ai_next)
   {
      fd = socket( q->ai_family, q->ai_socktype, q->ai_protocol);
      if( fd == INVALID_SOCKET)
         continue;

      for(;;)
      {
         rval = connect( fd, q->ai_addr, (int) q->ai_addrlen);
         if( rval == 0 || WSAGetLastError() != WSAEINTR)
            break;
      }

      if( rval == 0)
      {
         if( mulle_socket_set_nodelay( fd) < 0)
            break;      /* connected, but unusable: report it as a failure */

         freeaddrinfo( addresses);
         return( fd);
      }

      closesocket( fd);
      fd = INVALID_SOCKET;
   }

   freeaddrinfo( addresses);
   return( MULLE_SOCKET_INVALID);
}


unsigned int   mulle_socket_find_free_port( void)
{
   struct sockaddr_in   addr;
   int                  length;
   unsigned int         port;
   SOCKET               fd;

   if( mulle_socket_init() < 0)
      return( 0);

   fd = socket( AF_INET, SOCK_STREAM, 0);
   if( fd == INVALID_SOCKET)
      return( 0);

   memset( &addr, 0, sizeof( addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK);
   addr.sin_port        = 0;

   if( bind( fd, (struct sockaddr *) &addr, sizeof( addr)) == SOCKET_ERROR)
   {
      closesocket( fd);
      return( 0);
   }

   length = sizeof( addr);
   if( getsockname( fd, (struct sockaddr *) &addr, &length) == SOCKET_ERROR)
   {
      closesocket( fd);
      return( 0);
   }

   port = ntohs( addr.sin_port);
   closesocket( fd);

   return( port);
}


/*
 * recv and send take an int, so a buffer larger than INT_MAX is truncated
 * rather than overflowed. It is a cap no caller should ever reach, and the one
 * that does gets a partial read or write -- which is what the return value says
 * happened -- instead of a negative length.
 */
intptr_t   mulle_socket_read( mulle_socket_t socket,
                              void *buffer,
                              size_t length)
{
   int   n;

   if( length > (size_t) INT_MAX)
      length = (size_t) INT_MAX;

   for(;;)
   {
      n = recv( socket, buffer, (int) length, 0);
      if( n != SOCKET_ERROR)
         return( (intptr_t) n);
      if( WSAGetLastError() != WSAEINTR)
         return( -1);
   }
}


intptr_t   mulle_socket_write( mulle_socket_t socket,
                               void const *bytes,
                               size_t length)
{
   int   n;

   if( length > (size_t) INT_MAX)
      length = (size_t) INT_MAX;

   for(;;)
   {
      n = send( socket, bytes, (int) length, 0);
      if( n != SOCKET_ERROR)
         return( (intptr_t) n);
      if( WSAGetLastError() != WSAEINTR)
         return( -1);
   }
}


int   mulle_socket_shutdown( mulle_socket_t socket)
{
   if( socket == MULLE_SOCKET_INVALID)
      return( 0);

   return( shutdown( socket, SD_SEND));
}


int   mulle_socket_close( mulle_socket_t socket)
{
   if( socket == MULLE_SOCKET_INVALID)
      return( 0);

   return( closesocket( socket));
}

#endif
