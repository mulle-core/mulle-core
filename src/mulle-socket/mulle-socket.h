/*
 *  mulle-socket.h
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
#ifndef mulle__socket_h__
#define mulle__socket_h__

#include "include.h"

#include <stddef.h>
#include <stdint.h>


#define MULLE__SOCKET_VERSION  ((1UL << 20) | (0 << 8) | 0)


static inline unsigned int   mulle_socket_get_version_major( void)
{
   return( MULLE__SOCKET_VERSION >> 20);
}


static inline unsigned int   mulle_socket_get_version_minor( void)
{
   return( (MULLE__SOCKET_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_socket_get_version_patch( void)
{
   return( MULLE__SOCKET_VERSION & 0xFF);
}


MULLE__SOCKET_GLOBAL
uint32_t   mulle_socket_get_version( void);



/*
 * TCP and unix domain sockets, from the same three calls on every platform.
 *
 * Sockets are the one API where POSIX and Windows do not merely differ in
 * details: they disagree about what a socket *is*. On POSIX it is a file
 * descriptor, so it reads and writes with read(2) and write(2). On Windows it
 * is a kernel handle of its own kind, and the CRT's _read()/_write() do not
 * know it -- a program that keeps one abstraction over "a descriptor" ends up
 * unable to send a byte on Windows, and the failure is not a compile error.
 *
 * So this library never pretends a socket is a descriptor. mulle_socket_read
 * and mulle_socket_write are the way in on both platforms, and the differences
 * that cannot be hidden away are settled here, once:
 *
 *   - WSAStartup. Windows needs starting up and POSIX does not; mulle-socket
 *     starts it on first use, so no caller has to know. mulle_socket_init() is
 *     available for a caller who wants it done before anything else, and it is
 *     idempotent.
 *
 *   - Errors. Windows reports them from WSAGetLastError(), POSIX from errno.
 *     mulle_socket_error() translates the Windows codes that have a POSIX
 *     equivalent into it, so a caller can test EAGAIN, ECONNRESET, EADDRINUSE
 *     and friends on both platforms with one piece of code.
 *
 *   - SIGPIPE. On POSIX a write to a socket the peer has closed raises SIGPIPE,
 *     which by default kills the process -- so every program has to remember
 *     signal( SIGPIPE, SIG_IGN) before it does any I/O. mulle_socket_write
 *     sends with MSG_NOSIGNAL, so the signal is never raised and no caller has
 *     to disarm it globally. Windows has no SIGPIPE to raise.
 *
 *   - EINTR. POSIX system calls can be interrupted by a signal; Windows calls
 *     cannot. mulle_socket_read, mulle_socket_write, mulle_socket_accept and
 *     mulle_socket_connect all retry it themselves, so callers stop writing the
 *     loop.
 *
 * What is deliberately *not* here: name resolution beyond what the platform
 * gives getaddrinfo(), any framing, and any timeout -- a blocking socket that
 * needs a deadline belongs in a thread, which is what this library is for.
 *
 * Every call that returns a mulle_socket_t returns MULLE_SOCKET_INVALID on
 * failure, with the reason in mulle_socket_error().
 *
 * The handles are blocking and connected-or-listening; nothing is closed twice
 * and nothing is closed for you. mulle_socket_close() is the one way out, and
 * it accepts MULLE_SOCKET_INVALID so cleanup paths can be unconditional.
 */

#ifdef _WIN32

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif

/*
 * Order matters and is not negotiable: winsock2.h before windows.h, or
 * windows.h pulls in the older winsock.h and the two sets of declarations
 * collide. ws2tcpip.h carries getaddrinfo() and inet_pton(), which are the
 * parts of the API that only exist there.
 */
# include <winsock2.h>
# include <ws2tcpip.h>
# include <windows.h>

typedef SOCKET   mulle_socket_t;

# define MULLE_SOCKET_INVALID   INVALID_SOCKET

/* No unix domain sockets here. Windows has had AF_UNIX since 10 1803, but
   whether <afunix.h> and struct sockaddr_un are there depends on the SDK in a
   way this library cannot paper over, and nothing in this tree needs one. A
   caller who does can open AF_UNIX itself; what is offered is offered on
   every platform, which is the whole point of the type above being a SOCKET
   and not an int. */

#else

# include <errno.h>

typedef int   mulle_socket_t;

# define MULLE_SOCKET_INVALID   (-1)
# define MULLE_SOCKET_HAVE_UNIX   1

#endif


/*
 * Starts whatever the platform needs started, and does nothing on the second
 * call. Returns 0, or -1 if sockets cannot be used at all -- on Windows a
 * machine without a usable winsock is the only way that happens, and every
 * other call in this header fails the same way afterwards.
 *
 * You do not have to call this: every entry point below calls it first. It
 * exists for the caller who wants the failure to show up at a place of their
 * choosing. There is no matching shutdown, deliberately: winsock is torn down
 * when the process ends, and doing it earlier only breaks threads that are
 * still using sockets.
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_init( void);


/*
 * The reason the last call failed, as an errno value. 0 when it did not.
 *
 * Windows errors that mean the same as a POSIX one are translated
 * (WSAEWOULDBLOCK to EAGAIN, WSAECONNRESET to ECONNRESET, WSAEADDRINUSE to
 * EADDRINUSE, and so on); anything with no POSIX equivalent arrives as EIO,
 * which is a faithful "something went wrong, and this API cannot name it".
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_error( void);


/*
 * A listening TCP socket on `address`, which is a literal IPv4 address, or NULL
 * for every interface the host has. A `port` of 0 asks the platform for a free
 * one, and *p_port -- which may be NULL -- receives the port that was actually
 * bound either way.
 *
 * SO_REUSEADDR is set, because every server that can be restarted wants it and
 * none of them wants to remember. If you need it off, that is a different call
 * on this socket, not a different default.
 *
 * On Windows it is deliberately left off, because it does not mean the same
 * thing there: see mulle_socket_set_reuseaddr(). Nothing is lost, because a
 * Windows server can already rebind its port -- including while connections are
 * still in TIME_WAIT -- without asking for it.
 *
 * This is not a resolver: mulle_socket_connect() takes a host and asks the
 * platform for addresses, but a *server* binds to what it was told.
 */
MULLE__SOCKET_GLOBAL
mulle_socket_t   mulle_socket_listen( char const *address,
                                      unsigned int port,
                                      unsigned int *p_port);


/*
 * The next connection on a listener, or MULLE_SOCKET_INVALID. TCP_NODELAY is
 * set on it, which for a request/response protocol is a correctness setting and
 * not a tuning knob.
 *
 * Nagle will not send a new segment while earlier bytes are unacknowledged
 * unless the segment is a full MSS, and the peer's delayed ACK adds up to 40 ms
 * to that. A reply batch ends in a partial segment by definition, so without
 * this the tail of every batch inherits that timer -- the mulle-core-demo redis
 * suite measured 16 pipelined 4 KB requests answering their last reply in
 * 34 ms instead of 0.15 ms. The batch size at which it bites moves with the
 * value size, so growing a buffer only relocates it. redis sets this flag on
 * every connection it accepts.
 *
 * EINTR is retried. Nothing else is: a listener that has been closed reports
 * its error and the caller decides.
 */
MULLE__SOCKET_GLOBAL
mulle_socket_t   mulle_socket_accept( mulle_socket_t listener);


/*
 * Connect to `host`:`port`, where `host` may be a name. Every address the
 * platform returns for it is tried in turn, so a name that resolves to both
 * IPv6 and IPv4 connects if either does.
 *
 * The first address that answers wins; if all of them refuse, the error from
 * the last one is mulle_socket_error(). EINTR is retried.
 *
 * TCP_NODELAY is set on the result for the same reason mulle_socket_accept()
 * sets it: a client's batch ends in a partial segment too, so the request half
 * of a round trip inherits the same timer.
 */
MULLE__SOCKET_GLOBAL
mulle_socket_t   mulle_socket_connect( char const *host, unsigned int port);


/*
 * A port nobody is listening on, for a server about to be started.
 *
 * It works by binding port 0, reading back what the platform chose, and closing
 * again -- which is why it is racy by nature and why the port is free when this
 * returns, not reserved. A caller that needs the port kept must bind it itself
 * and use that socket. 0 on failure.
 */
MULLE__SOCKET_GLOBAL
unsigned int   mulle_socket_find_free_port( void);


/*
 * The two sockets options anyone actually sets, without the socket option
 * vocabulary: no IPPROTO_ levels to remember, no size argument to get wrong.
 * Both return 0 or -1.
 *
 * On Windows, mulle_socket_set_reuseaddr() sets nothing and returns 0. There
 * SO_REUSEADDR does not mean "rebind a port that is still in TIME_WAIT", it
 * means "let a *different* process bind this port as well", so setting it is
 * how a listen socket gets taken away from its owner. Windows does not need it
 * to restart a server, so a no-op is the whole of what a caller wants and the
 * only thing that fails to surprise is refusing to set it. It is therefore a
 * call that can be made on every platform, not one that must be guarded.
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_set_reuseaddr( mulle_socket_t socket);

MULLE__SOCKET_GLOBAL
int   mulle_socket_set_nodelay( mulle_socket_t socket);


/*
 * Bound a blocking read or write, in seconds. 0 -- which is also what a socket
 * does by default -- means wait forever, on both platforms.
 *
 * A read or write that runs out of time returns -1 with
 * mulle_socket_error() == ETIMEDOUT, the same on every platform. That is the
 * whole reason these two exist rather than a documented raw setsockopt: POSIX
 * takes a struct timeval here and reports the expiry as EAGAIN, while Windows
 * takes a DWORD of milliseconds and reports it as WSAETIMEDOUT. Both are
 * settled behind these calls, so a caller writes one test.
 *
 * Sub-millisecond values round *up* to the platform's resolution rather than
 * down to zero, because 0 means forever: a 0.0004 s timeout becomes 1 ms on
 * Windows, never "no timeout at all".
 *
 * What a timeout does not bound, and this is measured rather than assumed:
 *
 * - **mulle_socket_accept(), except on Linux.** Linux is the only one of the
 *   three platforms here whose kernel applies a receive timeout to `accept`
 *   (and its expiry is reported as ETIMEDOUT like the rest). BSD and macOS do
 *   not, and neither does Winsock, so on those an accept on an idle listener
 *   waits however small the timeout is. That is measured, not assumed: the
 *   probe is probe/timeout-probe.c in the mulle-socket constituent, and it
 *   hangs on exactly those platforms. A caller who needs a bounded accept there wants
 *   `select()` -- a readiness wait, not a socket option -- and this library
 *   deliberately does not wrap that, because it would change what accept means
 *   without the socket option that is supposed to mean it.
 * - **mulle_socket_connect(), everywhere.** No socket option expresses it:
 *   bounding a connect means a non-blocking connect plus a readiness wait, and
 *   a half-open socket to clean up when it expires. Linux's `TCP_SYNCNT` and
 *   `TCP_USER_TIMEOUT` are neither portable nor a deadline.
 *
 * And mulle_socket_write_all() is bounded per send, not as a whole: on POSIX a
 * send that expires after moving some bytes returns the count rather than
 * failing, so a large write can take several multiples of the timeout before it
 * gives up. Windows fails the send outright instead.
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_set_read_timeout( mulle_socket_t socket, double seconds);

MULLE__SOCKET_GLOBAL
int   mulle_socket_set_write_timeout( mulle_socket_t socket, double seconds);


/*
 * Read and write, with the platform's differences settled: negative on error
 * (with mulle_socket_error()), and 0 from a read means the peer has closed.
 *
 * The return type is intptr_t rather than ssize_t because a public header has
 * to compile for a Windows compiler that has never heard of ssize_t.
 *
 * A blocking socket can return fewer bytes than asked for; that is normal and
 * not an error. mulle_socket_write_all is the loop for callers who want the
 * whole buffer gone, and it is the one to use unless you are doing your own
 * buffering.
 *
 * MSG_NOSIGNAL is used where it exists, so a write to a closed socket reports
 * EPIPE instead of killing the process. Read it as a normal error; the peer
 * is gone.
 */
MULLE__SOCKET_GLOBAL
intptr_t   mulle_socket_read( mulle_socket_t socket,
                              void *buffer,
                              size_t length);

MULLE__SOCKET_GLOBAL
intptr_t   mulle_socket_write( mulle_socket_t socket,
                              void const *bytes,
                              size_t length);

MULLE__SOCKET_GLOBAL
int   mulle_socket_write_all( mulle_socket_t socket,
                              void const *bytes,
                              size_t length);


/*
 * Half close: the peer sees end of file, this end can still be read from (or
 * the other way round, for the reading direction, which is not offered here
 * because nothing in this tree wants it). A socket that is merely closed can
 * lose writes that are still in flight to the peer.
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_shutdown( mulle_socket_t socket);


/*
 * The one way to release a socket. MULLE_SOCKET_INVALID is accepted and does
 * nothing, so cleanup paths do not need to ask whether the socket ever opened.
 */
MULLE__SOCKET_GLOBAL
int   mulle_socket_close( mulle_socket_t socket);


#ifdef MULLE_SOCKET_HAVE_UNIX

/*
 * Unix domain sockets: a path instead of an address, and no port.
 *
 * mulle_socket_listen_unix binds the path, leaving any existing socket file to
 * the caller to remove first -- unlinking a path this library did not create is
 * not a decision it gets to make for you, and a stale socket file is a
 * different problem from a stale file.
 */
MULLE__SOCKET_GLOBAL
mulle_socket_t   mulle_socket_listen_unix( char const *path);

MULLE__SOCKET_GLOBAL
mulle_socket_t   mulle_socket_connect_unix( char const *path);

#endif


/*
 * The following files are auto-generated.
 * The _mulle-socket-provide header is generated by
 * mulle-match-to-c during a reflect.
 */
#include "_mulle-socket-provide.h"

/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-socket-versioncheck.h")
#  include "_mulle-socket-versioncheck.h"
# endif
#endif

#endif
