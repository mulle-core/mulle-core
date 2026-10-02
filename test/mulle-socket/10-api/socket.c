/*
 * socket.c -- mulle-socket's API test.
 *
 * The listener, the client and the server are all this process, so it needs no
 * child and no fixture: connect() completes against the backlog, and accept()
 * picks it up afterwards. In order it covers init, find_free_port, listen,
 * connect, accept, a 4 KB binary round trip, shutdown, the promise that a write
 * to a dead peer fails with EPIPE *without* a signal, a refused connection,
 * rebinding a port, and the timeouts.
 *
 * stdout is the result as well as the record: mulle-test compares it against
 * socket.stdout, or socket.stdout.<platform> where the platforms genuinely
 * differ -- and here they do, twice over: Linux bounds accept() with
 * SO_RCVTIMEO and macOS and Winsock do not, and the error numbers are the
 * platform's own (a refused connect is 111, 61 and 107). So the goldens are per
 * platform, and anything that varies run to run -- ports, elapsed times, byte
 * counts -- is behind --verbose instead:
 *
 *    ./socket              # exactly the recorded output
 *    ./socket --verbose    # that, plus ports, timings and byte counts
 *
 * Build and re-record, adding -lws2_32 on Windows:
 *
 *   cc -std=gnu11 -isystem <mulle-core>/src -o socket socket.c \
 *      -L<build>/mulle-core -lmulle-core -lpthread
 *   ./socket > socket.stdout
 *
 * The timeout probe next door in probe/ is what established which platform
 * bounds what; read it before doubting the header on that point.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the envelope, the way a downstream project sees it */
#include <mulle-socket/mulle-socket.h>

#include <errno.h>

#if defined( __APPLE__)
# define SMOKE_PLATFORM_NAME   "macOS"
#elif defined( _WIN32)
# define SMOKE_PLATFORM_NAME   "Windows"
#else
# define SMOKE_PLATFORM_NAME   "this platform"
#endif

#ifdef _WIN32
# include <windows.h>
# include <fcntl.h>
# include <io.h>
static double   now_seconds( void)   { return( (double) GetTickCount64() / 1000.0); }
#else
# include <time.h>
static double   now_seconds( void)
{
   struct timespec   ts;

   clock_gettime( CLOCK_MONOTONIC, &ts);
   return( (double) ts.tv_sec + (double) ts.tv_nsec / 1e9);
}
#endif


static int   failures;
static int   checks;
static int   verbose;


static void   check( char const *what, int ok)
{
   checks++;
   if( ! ok)
      failures++;

   printf( "%-4s %s\n", ok ? "ok" : "FAIL", what);
}


/*
 * Read exactly `length` bytes, the way a protocol reader has to. Anything less
 * is a short read and a bug somewhere, so it is reported rather than retried
 * into a timeout.
 */
static int   read_exactly( mulle_socket_t fd, void *buffer, size_t length)
{
   char     *p;
   intptr_t  n;
   size_t    got;

   p   = buffer;
   got = 0;

   while( got < length)
   {
      n = mulle_socket_read( fd, p + got, length - got);
      if( n <= 0)
         return( -1);
      got += (size_t) n;
   }

   return( 0);
}


/*
 * A client and a server that talk to each other, which is the case the redis
 * harness puts this library to.
 */
static void   test_exchange( void)
{
   mulle_socket_t   listener;
   mulle_socket_t   client;
   mulle_socket_t   server;
   unsigned int     port;
   char             buffer[ 64];
   char             pattern[ 4096];
   char             echoed[ 4096];
   size_t           i;
   intptr_t         n;

   port = mulle_socket_find_free_port();
   check( "find_free_port: got one", port != 0);
   if( verbose)
      printf( "     find_free_port: %u\n", port);

   port = 0;
   listener = mulle_socket_listen( "127.0.0.1", 0, &port);
   check( "listen: returns a socket", listener != MULLE_SOCKET_INVALID);
   if( listener == MULLE_SOCKET_INVALID)
   {
      printf( "     errno %d\n", mulle_socket_error());
      return;
   }
   check( "listen: port 0 got a port", port != 0);
   if( verbose)
      printf( "     listen: 127.0.0.1:%u\n", port);

   client = mulle_socket_connect( "127.0.0.1", port);
   check( "connect: to the listener", client != MULLE_SOCKET_INVALID);
   if( client == MULLE_SOCKET_INVALID)
   {
      printf( "     errno %d\n", mulle_socket_error());
      mulle_socket_close( listener);
      return;
   }

   server = mulle_socket_accept( listener);
   check( "accept: returns the connection", server != MULLE_SOCKET_INVALID);
   if( server == MULLE_SOCKET_INVALID)
   {
      printf( "     errno %d\n", mulle_socket_error());
      mulle_socket_close( client);
      mulle_socket_close( listener);
      return;
   }

   check( "write_all: a small request", 
          mulle_socket_write_all( client, "PING\r\n", 6) == 0);

   memset( buffer, 0, sizeof( buffer));
   check( "read: the request arrives",
          read_exactly( server, buffer, 6) == 0 && ! strcmp( buffer, "PING\r\n"));

   check( "write_all: the answer", 
          mulle_socket_write_all( server, "+PONG\r\n", 7) == 0);
   memset( buffer, 0, sizeof( buffer));
   check( "read: the answer comes back",
          read_exactly( client, buffer, 7) == 0 && ! strcmp( buffer, "+PONG\r\n"));

   /*
    * A pattern no text file would contain, round-tripped: what this proves is
    * byte fidelity, including the 0x0a and 0x0d in it. That is the difference
    * between this path and the demo's --stdio mode, which has to put the Windows
    * CRT into binary mode because a pipe is otherwise line-ending territory.
    *
    * It does not prove that write_all loops across several sends. Nobody is
    * reading while it writes, so the largest buffer that can be tested here is
    * one the socket takes in a single send; a bigger one would block partway
    * with the reader behind it. The redis suite is what covers that -- it sends
    * a fixture from one process while another reads it.
    */
   for( i = 0; i < sizeof( pattern); i++)
      pattern[ i ] = (char) (i * 7 + 1);

   check( "write_all: 4K of binary",
          mulle_socket_write_all( client, pattern, sizeof( pattern)) == 0);

   memset( echoed, 0, sizeof( echoed));
   check( "read: all 4K came back, byte for byte",
          read_exactly( server, echoed, sizeof( echoed)) == 0 &&
          ! memcmp( echoed, pattern, sizeof( pattern)));

   /*
    * The one direction of shutdown that means anything here: the client says it
    * is done sending, and the server sees EOF rather than blocking.
    */
   check( "shutdown: returns 0", mulle_socket_shutdown( client) == 0);
   memset( buffer, 0, sizeof( buffer));
   n = mulle_socket_read( server, buffer, sizeof( buffer));
   check( "shutdown: the peer sees EOF", n == 0);

   /*
    * And the promise that matters most: writing to a peer that is gone fails
    * with EPIPE *without* raising SIGPIPE. If this fails, the whole library is
    * pointless -- the process dies here.
    */
   n = mulle_socket_write( client, "x", 1);
   check( "write after shutdown: fails", n == -1);
   check( "write after shutdown: EPIPE, not a signal",
          mulle_socket_error() == EPIPE);
   printf( "     write after shutdown: error %d\n", mulle_socket_error());

   check( "close: the connection", mulle_socket_close( server) == 0);
   check( "close: the client", mulle_socket_close( client) == 0);
   check( "close: the listener", mulle_socket_close( listener) == 0);
   check( "close: MULLE_SOCKET_INVALID is allowed",
          mulle_socket_close( MULLE_SOCKET_INVALID) == 0);
}


/*
 * A port with nothing behind it. This is what the redis suite's "the suite
 * connects to a port it did not start" failure would look like, and it is the
 * check that makes mulle_socket_error's translation worth having.
 */
static void   test_refused( void)
{
   mulle_socket_t   fd;
   unsigned int     port;
   int              error;

   port = mulle_socket_find_free_port();
   check( "refused: a free port to aim at", port != 0);
   if( ! port)
      return;

   fd = mulle_socket_connect( "127.0.0.1", port);
   check( "refused: connect fails", fd == MULLE_SOCKET_INVALID);

   error = mulle_socket_error();
   check( "refused: named as ECONNREFUSED", error == ECONNREFUSED);
   printf( "     refused: error %d\n", error);

   check( "refused: close is safe on failure",
          mulle_socket_close( fd) == 0);
}


/*
 * A server that stops and starts again on the same port. The connection above
 * leaves the port in TIME_WAIT on POSIX, and rebinding it is exactly what
 * SO_REUSEADDR exists for -- so this is the one check that says the option is
 * actually set where it can be.
 */
static void   test_rebind( void)
{
   mulle_socket_t   listener;
   mulle_socket_t   client;
   unsigned int     port;
   unsigned int     again;

   port     = 0;
   listener = mulle_socket_listen( "127.0.0.1", 0, &port);
   check( "rebind: first listen", listener != MULLE_SOCKET_INVALID);
   if( listener == MULLE_SOCKET_INVALID)
      return;

   /* take a connection, so the port really does go through TIME_WAIT */
   client = mulle_socket_connect( "127.0.0.1", port);
   check( "rebind: a client arrives", client != MULLE_SOCKET_INVALID);
   mulle_socket_close( client);
   mulle_socket_close( listener);

   again    = 0;
   listener = mulle_socket_listen( "127.0.0.1", port, &again);
   check( "rebind: the same port comes back",
          listener != MULLE_SOCKET_INVALID && again == port);
   if( listener == MULLE_SOCKET_INVALID)
      printf( "     rebind: errno %d\n", mulle_socket_error());

   mulle_socket_close( listener);
}


/*
 * A bounded read, a bounded write, and -- where the platform allows it -- a
 * bounded accept.
 *
 * The first two are the portability claim: POSIX reports an expired receive
 * timeout as EAGAIN and Windows as WSAETIMEDOUT, and both arrive at a caller as
 * ETIMEDOUT. The accept half is not that, and is measured instead of promised:
 * Winsock ignores the option for accept, so on Windows the listener there is
 * *not* bounded and the check is replaced by a line saying so.
 */
static void   test_timeouts( void)
{
   mulle_socket_t   listener;
   mulle_socket_t   client;
   mulle_socket_t   server;
   unsigned int     port;
   double           started;
   double           elapsed;
   char             buffer[ 64];
   char             *chunk;
   size_t           chunk_size;
   intptr_t         n;
   int              i;

   port     = 0;
   listener = mulle_socket_listen( "127.0.0.1", 0, &port);
   check( "timeout: a listener to talk to", listener != MULLE_SOCKET_INVALID);
   if( listener == MULLE_SOCKET_INVALID)
      return;

   client = mulle_socket_connect( "127.0.0.1", port);
   server = mulle_socket_accept( listener);
   check( "timeout: a connection", client != MULLE_SOCKET_INVALID &&
                                    server != MULLE_SOCKET_INVALID);
   if( client == MULLE_SOCKET_INVALID || server == MULLE_SOCKET_INVALID)
   {
      mulle_socket_close( listener);
      return;
   }

   /* ------------------------------------------------------------ a slow read */
   check( "read timeout: setting it returns 0",
          mulle_socket_set_read_timeout( client, 0.25) == 0);

   started = now_seconds();
   n       = mulle_socket_read( client, buffer, sizeof( buffer));
   elapsed = now_seconds() - started;

   check( "read timeout: the read gives up", n == -1);
   check( "read timeout: named ETIMEDOUT, as on every platform",
          mulle_socket_error() == ETIMEDOUT);
   check( "read timeout: and it waited for it",
          elapsed >= 0.25 && elapsed < 5.0);
   if( verbose)
      printf( "     read timeout: waited %.3fs\n", elapsed);

   /* ----------------------------------------------------------- a slow write
    *
    * Nobody reads `server`, so writes stop making progress once the buffers are
    * full. Chunks in a loop rather than one big buffer, so this cannot pass by
    * accident on a machine whose socket buffers are larger than the guess.
    */
   check( "write timeout: setting it returns 0",
          mulle_socket_set_write_timeout( client, 0.25) == 0);

   chunk_size = 1024 * 1024;
   chunk      = malloc( chunk_size);
   check( "write timeout: a chunk to send", chunk != NULL);
   if( chunk)
   {
      memset( chunk, 'x', chunk_size);

      started = now_seconds();
      for( i = 0; i < 256; i++)
         if( mulle_socket_write_all( client, chunk, chunk_size))
            break;
      elapsed = now_seconds() - started;

      check( "write timeout: the write gives up", i < 256);
      check( "write timeout: named ETIMEDOUT, as on every platform",
             mulle_socket_error() == ETIMEDOUT);
      check( "write timeout: and it did not hang", elapsed < 60.0);
      if( verbose)
         printf( "     write timeout: gave up after %dB in %.3fs\n",
                 i * 1024 * 1024, elapsed);
      free( chunk);
   }

   mulle_socket_close( server);
   mulle_socket_close( client);
   mulle_socket_close( listener);

   /* ------------------------------------------------------- a slow accept */
   port     = 0;
   listener = mulle_socket_listen( "127.0.0.1", 0, &port);
   check( "accept timeout: a fresh listener", listener != MULLE_SOCKET_INVALID);
   if( listener == MULLE_SOCKET_INVALID)
      return;
   check( "accept timeout: setting it returns 0",
          mulle_socket_set_read_timeout( listener, 0.3) == 0);

#if ! defined( __linux__)
   /*
    * Of the three platforms here, only Linux applies a receive timeout to
    * accept() at all: BSD and macOS do not, and Winsock does not either. So
    * nothing is asserted -- the check would hang, not fail -- and what the
    * platform does instead is named.
    */
   printf( "skip accept timeout: %s does not apply SO_RCVTIMEO to accept,"
           " so a bounded accept here is a select() loop\n",
           SMOKE_PLATFORM_NAME);
   mulle_socket_close( listener);
#else
   started = now_seconds();
   server  = mulle_socket_accept( listener);
   elapsed = now_seconds() - started;

   check( "accept timeout: the accept gives up",
          server == MULLE_SOCKET_INVALID);
   check( "accept timeout: named ETIMEDOUT",
          mulle_socket_error() == ETIMEDOUT);
   check( "accept timeout: and it waited for it",
          elapsed >= 0.3 && elapsed < 5.0);
   if( verbose)
      printf( "     accept timeout: waited %.3fs\n", elapsed);

   mulle_socket_close( listener);
#endif
}


int   main( int argc, char **argv)
{
#ifdef _WIN32
   /*
    * The record this test is compared against is written with \n on every
    * platform, so the CRT must not turn every one of ours into \r\n here: the
    * Windows output should differ from the other two where it really differs
    * (the accept bound, the platform's error numbers) and nowhere else.
    */
   (void) _setmode( _fileno( stdout), _O_BINARY);
#endif

   verbose = argc >= 2 &&
             ( ! strcmp( argv[ 1], "-v") || ! strcmp( argv[ 1], "--verbose"));

   /*
    * Unbuffered, so that a test that hangs -- which is how a timeout that a
    * platform does not honour shows up -- still says how far it got before it
    * was killed. Redis's harness needed the same lesson the hard way.
    */
   setvbuf( stdout, NULL, _IONBF, 0);

   check( "init: returns 0", mulle_socket_init() == 0);
   check( "init: is idempotent", mulle_socket_init() == 0);

   test_exchange();
   test_refused();
   test_rebind();
   test_timeouts();

   printf( "\n%d checks, %d failed\n", checks, failures);
   return( failures ? 1 : 0);
}
