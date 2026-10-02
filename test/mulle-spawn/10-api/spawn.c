/*
 * spawn.c -- mulle-spawn's API test.
 *
 * Every case runs *this* program again with an argument that makes it behave a
 * certain way, rather than reaching for /bin/sh and /bin/echo. That is the point
 * of the API, and it is also what makes the test mean the same thing on Linux,
 * macOS and Windows -- where /bin/sh is not a path that exists.
 *
 * stdout is the result as well as the record: mulle-test compares it against
 * spawn.stdout, or spawn.stdout.<platform> where the platforms genuinely differ
 * (the signal case is one). So nothing that varies run to run or machine to
 * machine may be printed by default -- those lines are behind --verbose, which
 * is what to pass when a check fails and the numbers are what you want:
 *
 *    ./spawn              # exactly the recorded output
 *    ./spawn --verbose    # that, plus timings and the self path
 *
 * To re-record after an intended change, build and run it, read the output, and
 * only then save it. On Windows the tree needs -I<mulle-core>/dlfcn-win32:
 *
 *   cc -std=gnu11 -isystem <mulle-core>/src -o spawn spawn.c \
 *      -L<build>/mulle-core -lmulle-core
 *   ./spawn > spawn.stdout
 */
#include <mulle-spawn/mulle-spawn.h>
#include <mulle-fprintf/mulle-fprintf.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
# include <signal.h>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <sys/stat.h>
# include <sys/wait.h>
# define smoke_read(fd, buf, n)    read( fd, buf, n)
# define smoke_write(fd, buf, n)   write( fd, buf, n)
# define smoke_close(fd)           close( fd)
# define smoke_mkdir(path)         mkdir( path, 0755)
# define smoke_rmdir(path)         rmdir( path)
# define smoke_getcwd(buf, n)      getcwd( buf, n)
# define smoke_pipe(fds)           pipe( fds)
# define smoke_fd_open(fd)         (fcntl( fd, F_GETFD) >= 0)
#else
# include <windows.h>
# include <direct.h>
# include <fcntl.h>
# include <io.h>
# define smoke_read(fd, buf, n)    _read( fd, buf, n)
# define smoke_write(fd, buf, n)   _write( fd, buf, n)
# define smoke_close(fd)           _close( fd)
# define smoke_mkdir(path)         _mkdir( path)
# define smoke_rmdir(path)         _rmdir( path)
# define smoke_getcwd(buf, n)      _getcwd( buf, n)
# define smoke_pipe(fds)           _pipe( fds, 4096, _O_BINARY)
# define smoke_fd_open(fd)         (_get_osfhandle( fd) != (intptr_t) -1)
#endif


/*
 * The signal case makes this program die by SIGSEGV on purpose. The sanitizers
 * install a SIGSEGV handler of their own and turn that into an ordinary exit,
 * so the parent would see `exited` where a plain build sees `signalled` -- the
 * assertion would fail for a reason that has nothing to do with mulle-spawn.
 * Tell them to keep their hands off; this crash is the test.
 */
#if defined( __SANITIZE_ADDRESS__)
# define SMOKE_SANITIZER   1
#elif defined( __has_feature)
# if __has_feature( address_sanitizer)
#  define SMOKE_SANITIZER   1
# endif
#endif

#ifdef SMOKE_SANITIZER
const char   *__asan_default_options( void)
{
   return( "handle_segv=0");
}
#endif


static int    failures;
static int    verbose;
static char   self_path[ 4096];
static int    self_found;


/*
 * The tail of a path, for both separators. Used so the self path assertions
 * describe *this* program whatever it happens to be called on disk, instead of
 * hardcoding the name this file was compiled under.
 */
static char   *smoke_basename( char *path)
{
   char   *p;
   char   *slash;

   slash = NULL;
   for( p = path; *p; p++)
      if( *p == '/' || *p == '\\')
         slash = p;

   return( slash ? slash + 1 : path);
}


static void   check( const char *what, int ok)
{
   mulle_printf( "%-4s %s\n", ok ? "ok" : "FAIL", what);
   if( ! ok)
      failures++;
}


static void   show( const char *label, struct mulle_spawn_result *r)
{
   mulle_printf( "     %s: status=%d exit=%d signal=%d length=%zu output=<%s>\n",
                 label, r->status, r->exit_code, r->signal_number, r->length,
                 r->output ? r->output : "");
}


/*
 * `dst` must have room for want + 1. A pipe can hand back a short read, so this
 * loops until it has the bytes or the writer is gone.
 */
static int   smoke_read_up_to( int fd, char *dst, size_t want)
{
   size_t   got;
   int      n;

   got = 0;
   while( got < want)
   {
      n = (int) smoke_read( fd, &dst[ got], want - got);
      if( n <= 0)
         break;
      got += (size_t) n;
   }
   dst[ got] = 0;
   return( (int) got);
}


// mulle-core's own monotonic clock, so this runs where there is no
// clock_gettime (mingw) exactly as it does where there is.
//
static double   now_seconds( void)
{
   return( mulle_relativetime_now());
}


/*
 * Unset and empty are two different things an environment can be, and the first
 * is what a replaced environment produces -- so they are told apart here rather
 * than both printed as nothing.
 */
static void   print_variable( const char *name)
{
   char   *value;

   value = getenv( name);
   mulle_printf( "%s=%s\n", name, value ? value : "<unset>");
}


/*
 * The marker goes into the parent's own environment, so that what the child says
 * about it is a statement about the launch and not about whatever environment
 * this test was started in.
 */
static void   set_marker( void)
{
#ifdef _WIN32
   (void) _putenv_s( "MULLE_SPAWN_MARKER", "inherited");
#else
   (void) setenv( "MULLE_SPAWN_MARKER", "inherited", 1);
#endif
}


/*
 * A capture printed line by line, so that a multi line answer reads as lines in
 * the record instead of as one string with newlines in the middle of it.
 */
static void   smoke_show_lines( const char *label, const char *text)
{
   const char   *nl;
   const char   *p;

   for( p = text; *p; p = nl + 1)
   {
      nl = strchr( p, '\n');
      if( ! nl)
      {
         mulle_printf( "     %s: %s\n", label, p);
         break;
      }
      mulle_printf( "     %s: %.*s\n", label, (int) (nl - p), p);
   }
}


/*
 * Runs the child with `options` and leaves what it printed in `buffer`.
 * Returns the number of bytes captured, or -1 if it could not be started.
 * The caller's options are borrowed; the handle is opened, drained and closed
 * here, which is all these cases need of one.
 */
static int   smoke_capture( char * const *argv,
                            struct mulle_spawn_options const *options,
                            char *buffer,
                            size_t capacity)
{
   struct mulle_spawn_handle   h;
   int                         n;

   if( mulle_spawn_open_with_options( &h, argv, options))
      return( -1);

   if( h.out < 0)
   {
      (void) mulle_spawn_close( &h, NULL);
      return( -1);
   }

   n = smoke_read_up_to( h.out, buffer, capacity - 1);

   (void) mulle_spawn_wait( &h, 5.0, NULL);
   (void) mulle_spawn_close( &h, NULL);
   return( n);
}


/*
 * Asking after a process this program no longer has a handle for, which is what
 * the detach case leaves behind: a signal that only asks, or a kernel object
 * opened on the pid. Both are a snapshot -- "it is running" is not a promise
 * about a moment later -- and that is enough for a child asleep for thirty
 * seconds with nothing to exit for.
 *
 * `smoke_end_process` is the other half: a test that detaches a child still has
 * to clean it up, or it leaves a sleeper behind for the next run to find.
 */
#ifdef _WIN32
static int   smoke_is_running( unsigned long pid)
{
   HANDLE   process;
   DWORD    waited;

   process = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE,
                          FALSE, (DWORD) pid);
   if( ! process)
      return( 0);

   waited = WaitForSingleObject( process, 0);
   CloseHandle( process);
   return( waited == WAIT_TIMEOUT);
}


static void   smoke_end_process( unsigned long pid)
{
   HANDLE   process;

   process = OpenProcess( PROCESS_TERMINATE, FALSE, (DWORD) pid);
   if( ! process)
      return;

   (void) TerminateProcess( process, 1);
   CloseHandle( process);
}
#else
static int   smoke_is_running( unsigned long pid)
{
   return( kill( (pid_t) pid, 0) == 0);
}


static void   smoke_end_process( unsigned long pid)
{
   int   status;

   (void) kill( (pid_t) pid, SIGKILL);

   /* it is still this process's child, detached or not, so it is ours to reap */
   while( waitpid( (pid_t) pid, &status, 0) < 0)
   {
      if( errno != EINTR)
         break;
   }
}
#endif


/*
 * The child modes. Each one exists so the parent has a real second process to
 * observe, and each exercises a different status: a clean exit, a non zero exit,
 * and one that outlives its deadline after having said something.
 */
static int   child_main( int argc, char **argv)
{
#ifdef _WIN32
   /*
    * A CRT writes \r\n for every \n in a text mode stream, and that translation
    * happens inside the child -- mulle-spawn cannot reach it, which is why
    * mulle_spawn_run undoes the doubling for what it captures. A child that
    * speaks a protocol over a pipe has to say so itself, and it has to be the
    * child: without these three lines the same child reads differently here
    * than it does on POSIX.
    */
   (void) _setmode( _fileno( stdin), _O_BINARY);
   (void) _setmode( _fileno( stdout), _O_BINARY);
   (void) _setmode( _fileno( stderr), _O_BINARY);
#endif

   if( argc >= 2 && ! strcmp( argv[ 1], "--filter"))
   {
      char   line[ 256];

      /* answers what it is given, line by line, until its stdin closes */
      while( fgets( line, sizeof( line), stdin))
      {
         mulle_printf( "got %s", line);
         fflush( stdout);
      }
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--banner"))
   {
      /* says where it is, then keeps running: the server pattern */
      mulle_fprintf( stderr, "listening on 127.0.0.1:%s\n", argv[ 2]);
      mulle_fflush( stderr);
      mulle_relativetime_sleep( 30);
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--child"))
   {
      mulle_printf( "child says %s", argv[ 2]);
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--echo"))
   {
      mulle_printf( "%s\n", argv[ 2]);
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--exit"))
      return( atoi( argv[ 2]));

   if( argc >= 3 && ! strcmp( argv[ 1], "--environment"))
   {
      int   i;

      /* the names are the question; the answers are what it was launched with */
      for( i = 2; i < argc; i++)
         print_variable( argv[ i]);
      return( 0);
   }

   if( ! strcmp( argv[ 1], "--cwd"))
   {
      char   buffer[ 1024];

      if( ! smoke_getcwd( buffer, sizeof( buffer)))
         return( 1);

      /* the tail, not the path: a path is a machine's layout, not a fact */
      mulle_printf( "cwd=%s\n", smoke_basename( buffer));
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--stall"))
   {
      mulle_printf( "out\n");
      fflush( stdout);
      mulle_relativetime_sleep( atoi( argv[ 2]));
      return( 0);
   }

   if( argc >= 3 && ! strcmp( argv[ 1], "--tick"))
   {
      /*
       * Says the same thing on a clock, so that "is it still doing anything"
       * is a question with an answer: a suspended child answers it by going
       * quiet, and an interrupted one by never answering again. Flushed every
       * time, because buffered output would hide both.
       *
       * The count is deliberately far larger than any case needs: a child that
       * finishes on its own while its parent is between two checks makes those
       * checks meaningless -- and a stopped child that has already exited is
       * still a pid that `kill` happily accepts.
       */
      int   i;
      int   count;

      count = atoi( argv[ 2]);

#ifndef _WIN32
      /*
       * A disposition of SIG_IGN survives `exec`, and a test harness that
       * ignores SIGINT -- so that Ctrl+C in a terminal does not stop a whole
       * suite -- hands that on to every child it starts. This child wants the
       * signal to do what the signal does, and only the child can say so.
       */
      signal( SIGINT, SIG_DFL);
#endif

      for( i = 0; i < count; i++)
      {
         mulle_printf( "tick\n");
         fflush( stdout);
         mulle_relativetime_sleep( 0.1);
      }
      return( 0);
   }

#ifndef _WIN32
   if( ! strcmp( argv[ 1], "--segv"))
   {
      kill( getpid(), SIGSEGV);
      return( 0);
   }
#endif

   return( -1);
}


static void   test_echo( void)
{
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--echo";
   argv[ 2] = "hello";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);
   rval = mulle_spawn_run( argv, 5.0, &r);

   check( "echo: run returns 0", rval == 0);
   check( "echo: exited", r.status == mulle_spawn_exited);
   check( "echo: exit code 0", r.exit_code == 0);
   check( "echo: captured stdout", r.output && ! strcmp( r.output, "hello\n"));
   show( "echo", &r);
   mulle_spawn_result_done( &r);
}


static void   test_timeout( void)
{
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   double                      elapsed;
   double                      start;
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--stall";
   argv[ 2] = "30";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);
   start  = now_seconds();
   rval   = mulle_spawn_run( argv, 0.4, &r);
   elapsed = now_seconds() - start;

   check( "timeout: run returns 0", rval == 0);
   check( "timeout: terminated", r.status == mulle_spawn_terminated);
   check( "timeout: the deadline is enforced", elapsed < 3.0);
   check( "timeout: output survives the kill", r.output && ! strcmp( r.output, "out\n"));
   if( verbose)
      mulle_printf( "     deadline fired after %.2fs\n", elapsed);
   show( "timeout", &r);
   mulle_spawn_result_done( &r);
}


static void   test_cannot_run( void)
{
   struct mulle_spawn_result   r;
   char                        *argv[ 2];
   int                         rval;

   argv[ 0] = "/nonexistent/mulle-spawn-smoke-program";
   argv[ 1] = NULL;

   mulle_spawn_result_init( &r, NULL);
   rval = mulle_spawn_run( argv, 5.0, &r);

   check( "cannot run: run returns -1", rval == -1);
   check( "cannot run: status failed", r.status == mulle_spawn_failed);
   show( "cannot run", &r);
   mulle_spawn_result_done( &r);
}


static void   test_exit_code( void)
{
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--exit";
   argv[ 2] = "3";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);
   rval = mulle_spawn_run( argv, 5.0, &r);

   check( "exit 3: run returns 0", rval == 0);
   check( "exit 3: exited", r.status == mulle_spawn_exited);
   check( "exit 3: exit code 3", r.exit_code == 3);
   check( "exit 3: no output", r.length == 0 && r.output == NULL);
   show( "exit 3", &r);
   mulle_spawn_result_done( &r);
}


/*
 * signal_number is POSIX by definition -- mulle-spawn.h says so -- so there is
 * nothing to assert on Windows, where a crashing child is just a non zero exit
 * code and the process did come back.
 */
static void   test_signal( void)
{
#ifndef _WIN32
   struct mulle_spawn_result   r;
   char                        *argv[ 3];
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--segv";
   argv[ 2] = NULL;

   mulle_spawn_result_init( &r, NULL);
   rval = mulle_spawn_run( argv, 5.0, &r);

   check( "signal: run returns 0", rval == 0);
   check( "signal: signalled", r.status == mulle_spawn_signalled);
   check( "signal: SIGSEGV recorded", r.signal_number == 11);
   show( "signal", &r);
   mulle_spawn_result_done( &r);
#else
   mulle_printf( "skip no POSIX signal_number on this platform\n");
#endif
}


/*
 * The pattern hashspace needs: find yourself, and run yourself again with
 * arguments. Nothing is inherited, so this is a genuine second process.
 */
static void   test_reexec( void)
{
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--child";
   argv[ 2] = "smoke";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);
   rval = mulle_spawn_run( argv, 5.0, &r);

   check( "re-exec: run returns 0", rval == 0);
   check( "re-exec: exited", r.status == mulle_spawn_exited);
   check( "re-exec: the child's report came back",
          r.output && ! strcmp( r.output, "child says smoke"));
   show( "re-exec", &r);
   mulle_spawn_result_done( &r);
}


/*
 * The two way case, which is what a stdio protocol needs: the child's stdin and
 * stdout are both pipes, and the parent holds both ends.
 */
static void   test_handle_filter( void)
{
   struct mulle_spawn_handle   h;
   char                        buffer[ 64];
   char                        *argv[ 3];
   int                         n;
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--filter";
   argv[ 2] = NULL;

   rval = mulle_spawn_open( &h, argv, MULLE_SPAWN_PIPE, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT);
   check( "handle: open returns 0", rval == 0);
   if( rval)
      return;

   check( "handle: both ends came back", h.in >= 0 && h.out >= 0);
   check( "handle: stderr was left inherited", h.err == -1);
   check( "handle: it has a pid", h.pid != 0);

   check( "handle: write reaches it", smoke_write( h.in, "ping\n", 5) == 5);

   n = smoke_read_up_to( h.out, buffer, 9);
   check( "handle: and the answer comes back",
          n == 9 && ! strcmp( buffer, "got ping\n"));
   mulle_printf( "     handle: the child said <%s>\n", buffer);

   /*
    * Closing our end of its stdin is how a filter is told there is no more.
    * The handle is told by setting the field to -1, so that close does not do
    * it again -- descriptor numbers get reused.
    */
   smoke_close( h.in);
   h.in = -1;

   check( "handle: wait sees it stop", mulle_spawn_wait( &h, 5.0, NULL) == 1);
   check( "handle: close returns 0", mulle_spawn_close( &h, NULL) == 0);
   check( "handle: close resets it",
          h.in == -1 && h.out == -1 && h.err == -1 && h.pid == 0);
}


/*
 * The banner pattern: learn something from the child's stderr while it keeps
 * running, then stop it. `mredis_spawn_server` is exactly this.
 */
static void   test_handle_banner( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_result   r;
   char                        buffer[ 128];
   char                        *argv[ 4];
   int                         n;
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--banner";
   argv[ 2] = "6379";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);

   rval = mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_DISCARD,
                            MULLE_SPAWN_PIPE);
   check( "banner: open returns 0", rval == 0);
   if( rval)
      return;

   check( "banner: stdout was discarded", h.out == -1);
   check( "banner: stderr came back", h.err >= 0);

   n = smoke_read_up_to( h.err, buffer, 28);
   check( "banner: it announced itself",
          n == 28 && ! strcmp( buffer, "listening on 127.0.0.1:6379\n"));
   mulle_printf( "     banner: <%s>\n", buffer);

   /* the whole point: the banner arrived while the child is still there */
   check( "banner: it is still running", mulle_spawn_wait( &h, 0.0, NULL) == 0);
   check( "banner: terminate asks it to stop", mulle_spawn_terminate( &h) == 0);
   check( "banner: and it goes", mulle_spawn_wait( &h, 5.0, &r) == 1);

#ifndef _WIN32
   /* there is no SIGTERM to send here, so on POSIX that is what it is */
   check( "banner: stopped by SIGTERM",
          r.status == mulle_spawn_signalled && r.signal_number == 15);
   mulle_printf( "     banner: status=%d signal=%d\n", r.status, r.signal_number);
#else
   /*
    * TerminateProcess, which used to be reported as "exited, 1" -- the same
    * answer as a child that chose to exit 1. It is remembered as a termination
    * now, so that a caller can tell the two apart.
    */
   check( "banner: stopped by TerminateProcess",
          r.status == mulle_spawn_terminated);
   mulle_printf( "     banner: status=%d exit=%d\n", r.status, r.exit_code);
#endif

   check( "banner: close returns 0", mulle_spawn_close( &h, NULL) == 0);
   mulle_spawn_result_done( &r);
}


/*
 * A child that is still going when the handle is closed has to go with it, and
 * close is where that is decided -- not wait, which is the polite half.
 */
static void   test_handle_close_kills( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_result   r;
   char                        buffer[ 64];
   char                        *argv[ 4];
   int                         n;
   int                         rval;

   argv[ 0] = self_path;
   argv[ 1] = "--stall";
   argv[ 2] = "30";
   argv[ 3] = NULL;

   mulle_spawn_result_init( &r, NULL);

   rval = mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT);
   check( "close: open returns 0", rval == 0);
   if( rval)
      return;

   n = smoke_read_up_to( h.out, buffer, 4);
   check( "close: it said its piece", n == 4 && ! strcmp( buffer, "out\n"));
   check( "close: and it is still there", mulle_spawn_wait( &h, 0.0, NULL) == 0);

   check( "close: close returns 0", mulle_spawn_close( &h, &r) == 0);
   check( "close: and calls it terminated", r.status == mulle_spawn_terminated);

   mulle_spawn_result_done( &r);
}


/*
 * A program that cannot be run is not a handle to a program that cannot be run.
 */
static void   test_handle_cannot_open( void)
{
   struct mulle_spawn_handle   h;
   char                        *argv[ 2];

   argv[ 0] = "/nonexistent/mulle-spawn-smoke-program";
   argv[ 1] = NULL;

   h.in  = 7;      /* so that a reset is visible as a reset */
   h.out = 7;
   h.err = 7;
   h.pid = 7;

   check( "handle: a program that cannot run is not opened",
          mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT) == -1);
   check( "handle: and leaves nothing open",
          h.in == -1 && h.out == -1 && h.err == -1 && h.pid == 0);
}


/*
 * `<the directory this program is in>/<name>`, so a case can make something of
 * its own without knowing where it was put -- which is what keeps the same
 * assertion true under `mulle-sde test`, under a hand build in /tmp, and on a
 * machine that lays the tree out differently. Note the small `smoke_basename`
 * above: what goes into the record is the tail of the path, never the path.
 *
 * Returns 0, or -1 if it does not fit or this program has no known location.
 */
static int   smoke_beside_self( char *dst, size_t capacity, const char *name)
{
   size_t   length;
   size_t   wanted;

   if( ! self_found)
      return( -1);

   length = strlen( self_path);
   while( length && self_path[ length - 1] != '/' && self_path[ length - 1] != '\\')
      length--;

   wanted = length + strlen( name) + 1;
   if( wanted > capacity)
      return( -1);

   memcpy( dst, self_path, length);
   memcpy( dst + length, name, strlen( name) + 1);
   return( 0);
}


/*
 * The directory a child ran in, given back.
 *
 * On Windows the removal is not immediate: the platform keeps answering EACCES
 * for a short while after the child has been waited for and its process handle
 * closed, and releases the directory on its own within a tenth of a second or
 * so. Measured there (under wine, which is the only Windows this has run on),
 * the third attempt at 25 ms apart succeeds. Nothing in mulle-spawn is failing
 * here -- every assertion of the case has run by the time this is called -- so
 * a bounded retry is the honest way to give the directory back, rather than a
 * case that leaves litter behind on one platform.
 *
 * An errno that will not change for the better is not retried, and a directory
 * that could not be removed is reported on stderr, which is not part of the
 * record.
 */
static void   smoke_rmdir_when_released( const char *path)
{
   int   i;

   for( i = 0; i < 40; i++)
   {
      errno = 0;
      if( ! smoke_rmdir( path))
         return;
      if( errno != EACCES && errno != EBUSY)
         break;
      mulle_relativetime_sleep( 0.025);
   }

   mulle_fprintf( stderr, "smoke: could not remove <%s>, errno %d -- a leftover\n",
                  path, errno);
}


/*
 * The directory a child runs in, which is not a nicety: a launch that names a
 * directory that is not there used to run the child wherever the parent was,
 * and that is indistinguishable from a child that worked.
 *
 * The directory is made here, beside this program, and given back however the
 * case ends.
 */
static void   test_directory( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_options  options;
   char                        buffer[ 256];
   char                        *argv[ 3];
   char                        work[ 4096];
   int                         n;

   argv[ 0] = self_path;
   argv[ 1] = "--cwd";
   argv[ 2] = NULL;

   mulle_spawn_options_init( &options);
   options.out_mode = MULLE_SPAWN_PIPE;

   if( smoke_beside_self( work, sizeof( work), "spawn-cwd"))
   {
      check( "directory: beside itself is a place it knows", 0);
      return;
   }

   /* a leftover from a run that died is not a reason to skip the case */
   if( smoke_mkdir( work) && errno != EEXIST)
   {
      check( "directory: made somewhere to run", 0);
      return;
   }

   options.directory = work;

   memset( buffer, 0, sizeof( buffer));
   n = smoke_capture( argv, &options, buffer, sizeof( buffer));
   check( "directory: open returns 0", n >= 0);
   check( "directory: it ran there", ! strcmp( buffer, "cwd=spawn-cwd\n"));
   smoke_show_lines( "directory", buffer);

   /* and one that is not there fails the launch, rather than being ignored */
   options.directory = "/nonexistent/mulle-spawn-no-such-directory";

   errno = 0;
   check( "directory: one that is absent is refused",
          mulle_spawn_open_with_options( &h, argv, &options) == -1);
#ifndef _WIN32
   check( "directory: and errno says ENOENT", errno == ENOENT);
#endif

   smoke_rmdir_when_released( work);
}


/*
 * A descriptor of the caller's own per stream -- what `MULLE_SPAWN_PIPE`
 * cannot be, because a pipe is something this call makes and hands back an end
 * of. A file, a socket, or a pipe somebody else made is what `setStandardOutput:`
 * with an `NSFileHandle` means, and this is the mode under it.
 *
 * Pipes are used rather than files, for two reasons that both matter here: the
 * code path is the same one a file would take (a descriptor the caller already
 * holds, handed over as it is), and a test that writes files has to remove them
 * again -- on one platform reaping a directory a child ran in already needed a
 * retry, and there is no reason to go looking for the same thing twice.
 */
static void   test_descriptor( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_options  options;
   char                        *argv[ 4];
   char                        buffer[ 64];
   int                         infds[ 2];
   int                         outfds[ 2];
   int                         n;

   /* four slots: the widest argv here is program, mode, argument, NULL */
   argv[ 0] = self_path;
   argv[ 1] = NULL;
   argv[ 2] = NULL;
   argv[ 3] = NULL;

   mulle_spawn_options_init( &options);

   /*
    * What the child reads. The write end is ours and is closed before the
    * launch, so the child's `--filter` sees the end of its input and stops.
    */
   if( smoke_pipe( infds))
   {
      check( "descriptor: made a pipe to hand over", 0);
      return;
   }

   check( "descriptor: wrote the line it will read",
          smoke_write( infds[ 1], "hello\n", 6) == 6);
   smoke_close( infds[ 1]);

   argv[ 1] = "--filter";

   options.in_descriptor = infds[ 0];
   options.in_mode       = MULLE_SPAWN_DESCRIPTOR;
   options.out_mode      = MULLE_SPAWN_PIPE;

   memset( buffer, 0, sizeof( buffer));
   n = smoke_capture( argv, &options, buffer, sizeof( buffer));
   check( "descriptor: open returns 0", n >= 0);
   check( "descriptor: the child got it as its stdin",
          ! strcmp( buffer, "got hello\n"));
   smoke_show_lines( "descriptor", buffer);

   /* the point of the mode: the descriptor is still the caller's afterwards */
   check( "descriptor: close left it open", smoke_fd_open( infds[ 0]));
   smoke_close( infds[ 0]);

   /*
    * And where the child writes. `--echo` says one line and is done, so the
    * order is: launch it, give the handle back, and only then read -- the read
    * end sees the line and then the end of the pipe, because by now the child
    * and our own copy of the write end are both gone.
    */
   if( smoke_pipe( outfds))
   {
      check( "descriptor: made a pipe to write into", 0);
      return;
   }

   argv[ 1] = "--echo";
   argv[ 2] = "through-a-pipe";

   options.in_descriptor  = -1;
   options.in_mode        = MULLE_SPAWN_INHERIT;
   options.out_mode       = MULLE_SPAWN_DESCRIPTOR;
   options.out_descriptor = outfds[ 1];

   check( "descriptor: open with it returns 0",
          mulle_spawn_open_with_options( &h, argv, &options) == 0);
   check( "descriptor: the handle made nothing of its own",
          h.in == -1 && h.out == -1 && h.err == -1);

   /* waited for before closing: close is allowed to end a child that is still
    * running, and a killed child never gets to flush what it printed */
   check( "descriptor: it stopped by itself",
          mulle_spawn_wait( &h, 5.0, NULL) == 1);
   check( "descriptor: close returns 0", mulle_spawn_close( &h, NULL) == 0);
   check( "descriptor: close left that one open too",
          smoke_fd_open( outfds[ 1]));

   smoke_close( outfds[ 1]);

   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( outfds[ 0], buffer, sizeof( buffer) - 1);
   check( "descriptor: it wrote where it was told",
          n == 15 && ! strcmp( buffer, "through-a-pipe\n"));
   smoke_show_lines( "descriptor", buffer);

   smoke_close( outfds[ 0]);

   /* and one that is not open fails the launch, rather than the child */
   options.out_descriptor = -1;

   errno = 0;
   check( "descriptor: one that is not open is refused",
          mulle_spawn_open_with_options( &h, argv, &options) == -1);
#ifndef _WIN32
   check( "descriptor: and errno says EBADF", errno == EBADF);
#endif
   check( "descriptor: and leaves nothing open",
          h.in == -1 && h.out == -1 && h.err == -1 && h.pid == 0);
}


/*
 * The one thing `close` will not do: let a child go on running.
 *
 * Everything here is about what detach gives *up* -- the descriptors, the pid,
 * the wait that would have collected the child -- so it is asserted by what is
 * gone and by the child still being there, not by anything the handle says.
 */
static void   test_detach( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   unsigned long               pid;
   int                         out;

   argv[ 0] = self_path;
   argv[ 1] = "--stall";
   argv[ 2] = "30";
   argv[ 3] = NULL;

   check( "detach: open returns 0",
          mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT) == 0);

   /* both have to be read out first: detach empties the handle */
   pid = h.pid;
   out = h.out;

   check( "detach: it is a real process", pid != 0 && out >= 0);
   check( "detach: returns 0", mulle_spawn_detach( &h) == 0);
   check( "detach: the handle is emptied",
          h.in == -1 && h.out == -1 && h.err == -1 &&
          h.pid == 0 && ! h.opened);
   check( "detach: the descriptor went with it", ! smoke_fd_open( out));
   check( "detach: there is nothing left to wait for",
          mulle_spawn_wait( &h, 0.0, &r) == -1);

   /*
    * The child is the thing that must *not* have changed: detach is only a
    * promise about this handle, and a detached child that has been killed is
    * exactly what a caller uses detach to avoid.
    */
   check( "detach: the child is still running", smoke_is_running( pid));

   /* a test cleans up after itself, so this one ends what it detached */
   smoke_end_process( pid);

   /* nothing to detach is not an error either way round */
   check( "detach: a handle that was never opened detaches",
          mulle_spawn_detach( &h) == 0);
   check( "detach: and NULL is refused", mulle_spawn_detach( NULL) == -1);
}


/*
 * Stopping a child and letting it go on again -- and interrupting one, which on
 * one platform is a signal and on the other one refuses to be.
 *
 * The child ticks: the same line every 100 ms, and many more of them than the
 * case could ever consume. So "is it still doing anything" has an answer -- a
 * suspended child goes quiet, an interrupted one never speaks again -- and
 * there is no count at which it might finish on its own between two checks.
 *
 * What can be asserted *exactly* differs, and that is worth spelling out:
 *
 *   POSIX    suspend is SIGSTOP, which cannot be caught or ignored, and the
 *            proof is silence: the child ticks every 100 ms, so after a stop
 *            the kernel confirms, four intervals with nothing to read is a
 *            child that is really not running -- and a tick arriving after the
 *            resume is the matching positive.
 *   Windows  suspend is NtSuspendProcess and there is no `poll` to ask with, so
 *            what is asserted there is the round trip and the tick that follows
 *            the resume.
 *
 * There is a tempting proof that is not used here: send SIGTERM to a stopped
 * child and watch it *not* die. Linux holds that signal pending until the
 * process is continued; Darwin kills a stopped process there and then. Both are
 * reasons enough not to build a test on it -- and worth knowing before an
 * `NSTask` port assumes the Linux answer.
 */
static void   test_interrupt_suspend( void)
{
   struct mulle_spawn_handle   h;
   struct mulle_spawn_result   r;
   char                        *argv[ 4];
   char                        buffer[ 32];
   int                         n;

   argv[ 0] = self_path;
   argv[ 1] = "--tick";
   argv[ 2] = "600";
   argv[ 3] = NULL;

   /* ---------------------------------------------------- suspend and resume */
   check( "suspend: open returns 0",
          mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT) == 0);

   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( h.out, buffer, 5);
   check( "suspend: the child is ticking", n == 5 && ! strcmp( buffer, "tick\n"));

   check( "suspend: returns 0", mulle_spawn_suspend( &h) == 0);
   check( "suspend: and it is still a running child",
          mulle_spawn_wait( &h, 0.1, NULL) == 0);
   check( "resume: returns 0", mulle_spawn_resume( &h) == 0);

   /* the only proof of a resume that is not the return value of the call */
   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( h.out, buffer, 5);
   check( "resume: and it is ticking again", n == 5 && ! strcmp( buffer, "tick\n"));

   check( "resume: terminate returns 0", mulle_spawn_terminate( &h) == 0);
   check( "resume: it stops on that", mulle_spawn_wait( &h, 5.0, &r) == 1);
#ifdef _WIN32
   /* the same thing, said the only way the platform can say it */
   check( "resume: and it was terminated", r.status == mulle_spawn_terminated);
#else
   check( "resume: as the signal it was sent",
          r.status == mulle_spawn_signalled && r.signal_number == SIGTERM);
#endif
   check( "resume: close returns 0", mulle_spawn_close( &h, NULL) == 0);

   /* ------------------------------------------------------------ interrupt */
   check( "interrupt: open returns 0",
          mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT) == 0);

   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( h.out, buffer, 5);
   check( "interrupt: the child is ticking", n == 5 && ! strcmp( buffer, "tick\n"));

#ifndef _WIN32
   check( "interrupt: returns 0", mulle_spawn_interrupt( &h) == 0);
   check( "interrupt: it stopped", mulle_spawn_wait( &h, 5.0, &r) == 1);
   check( "interrupt: of the signal it was sent",
          r.status == mulle_spawn_signalled && r.signal_number == SIGINT);
#else
   /*
    * Nothing to aim at: Ctrl+Break goes to a console process *group*, and this
    * child sits in this process's group because `new_process_group` was not
    * asked for. Refusing is the point -- the alternative is signalling the
    * console this test is running on, which includes the test.
    */
   errno = 0;
   check( "interrupt: refused without a group of its own",
          mulle_spawn_interrupt( &h) == -1 && errno == ENOTSUP);
   check( "interrupt: and the refusal leaves the child alone",
          mulle_spawn_wait( &h, 0.3, NULL) == 0);
   check( "interrupt: terminate is still the way to end it",
          mulle_spawn_terminate( &h) == 0);
   check( "interrupt: and that does stop it", mulle_spawn_wait( &h, 5.0, &r) == 1);
   check( "interrupt: reported as terminated, not as an exit",
          r.status == mulle_spawn_terminated);
#endif
   check( "interrupt: close returns 0", mulle_spawn_close( &h, NULL) == 0);

#ifndef _WIN32
   /*
    * The proof of a stop, which is silence measured.
    */
   check( "stop: open returns 0",
          mulle_spawn_open( &h, argv, MULLE_SPAWN_INHERIT, MULLE_SPAWN_PIPE,
                            MULLE_SPAWN_INHERIT) == 0);

   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( h.out, buffer, 5);
   check( "stop: the child is ticking", n == 5 && ! strcmp( buffer, "tick\n"));

   check( "stop: suspend returns 0", mulle_spawn_suspend( &h) == 0);

   /*
    * Wait until the *kernel* agrees that it stopped. Asking for a stop and then
    * asking whether it took effect are two different questions, and only the
    * second one makes the rest of this case mean anything. `WUNTRACED` is the
    * portable way to be told, and reporting a stop does not consume anything
    * the library wants: the child is still this process's to reap.
    *
    * Bounded, so a stop that never took effect fails the assertion instead of
    * hanging the suite.
    */
   {
      int   status;
      int   i;
      int   stopped;

      stopped = 0;
      for( i = 0; i < 100; i++)
      {
         if( waitpid( (pid_t) h.pid, &status, WNOHANG | WUNTRACED) == (pid_t) h.pid)
         {
            stopped = WIFSTOPPED( status);
            break;
         }
         mulle_relativetime_sleep( 0.01);
      }
      check( "stop: and the kernel says it is stopped", stopped);
   }

   /*
    * Whatever it managed to write before the stop is still in the pipe, and a
    * full pipe would look like a child that is still talking -- so the pipe is
    * emptied first, non blocking, and only then is silence a question. Four
    * hundred milliseconds is four of its ticks.
    */
   {
      struct pollfd   pfd;
      int             flags;
      int             drained;

      flags = fcntl( h.out, F_GETFL);
      (void) fcntl( h.out, F_SETFL, flags | O_NONBLOCK);

      do
      {
         errno   = 0;
         drained = (int) smoke_read( h.out, buffer, sizeof( buffer));
      }
      while( drained > 0);

      pfd.fd      = h.out;
      pfd.events  = POLLIN;
      pfd.revents = 0;

      check( "stop: and it has gone quiet", poll( &pfd, 1, 400) == 0);

      (void) fcntl( h.out, F_SETFL, flags);
   }

   check( "stop: it did not exit either", mulle_spawn_wait( &h, 0.0, NULL) == 0);
   check( "stop: resume returns 0", mulle_spawn_resume( &h) == 0);

   memset( buffer, 0, sizeof( buffer));
   n = smoke_read_up_to( h.out, buffer, 5);
   check( "stop: and it is ticking again", n == 5 && ! strcmp( buffer, "tick\n"));

   check( "stop: terminate returns 0", mulle_spawn_terminate( &h) == 0);
   check( "stop: it stops on that", mulle_spawn_wait( &h, 5.0, &r) == 1);
   check( "stop: as the signal it was",
          r.status == mulle_spawn_signalled && r.signal_number == SIGTERM);
   check( "stop: close returns 0", mulle_spawn_close( &h, NULL) == 0);
#endif

   /* a handle that was never opened has nothing to aim a signal at */
   check( "suspend: a handle that is not open is refused",
          mulle_spawn_suspend( &h) == -1);
   check( "resume: and so is a resume", mulle_spawn_resume( &h) == -1);
   check( "interrupt: and so is an interrupt", mulle_spawn_interrupt( &h) == -1);
}


/*
 * What the child is given to start with, which `mulle_spawn_open` cannot say
 * anything about -- a child inherits or it inherits.
 *
 * The same child is launched three times and asked the same two questions, and
 * the answers are expected to move:
 *
 *   inherited   NULL          the marker is there, the other name is not
 *   replaced    { TEST }      the marker is gone, the other name is there
 *   emptied     { NULL }      neither, which is not the same as "no opinion"
 *
 * The child is named by its absolute path throughout, so that none of this
 * depends on `PATH` surviving into the environment it is given.
 *
 * All three are judged the same way on every platform, and on Windows that
 * takes one allowance: the child is this program, which links the library, and
 * a child inherits the loader's search path through its `PATH`. So the two
 * environments that are *given* carry the parent's `PATH`, or the child is
 * stopped by the loader before it can answer either question. That is the
 * platform's way of carrying a search path and not part of what is being asked
 * about. `research/env-block-probe.c` is the measurement that the block itself is
 * honoured, once it is announced as the UTF-16 block it is -- the earlier
 * reading that wine ignores `lpEnvironment` was that missing announcement.
 */
static void   test_environment( void)
{
   struct mulle_spawn_options  options;
   char                        buffer[ 256];
   char                        *argv[ 5];
   /*
    * Three and two rather than one and one, because on Windows the loader path
    * takes the second slot of each -- see below. The extra slot is NULL there
    * and everywhere else, which is what terminates the array anyway.
    */
   char                        *emptied[ 3];
   char                        *replacement[ 3];
#ifdef _WIN32
   char                        loader[ 2048];
#endif
   int                         n;

   set_marker();

   argv[ 0] = self_path;
   argv[ 1] = "--environment";
   argv[ 2] = "MULLE_SPAWN_MARKER";
   argv[ 3] = "MULLE_SPAWN_TEST";
   argv[ 4] = NULL;

   replacement[ 0] = "MULLE_SPAWN_TEST=replaced";
   replacement[ 1] = NULL;
   replacement[ 2] = NULL;

   emptied[ 0] = NULL;
   emptied[ 1] = NULL;
   emptied[ 2] = NULL;

#ifdef _WIN32
   /*
    * The one entry of the parent's that a given environment has to keep: the
    * loader finds an imported DLL through the child's `PATH` (that is why the
    * test framework prepends the dependency's `bin` to `PATH`, and to `WINEPATH`
    * for the exe it names itself), and this child links the library. Hand it an
    * environment without that and it never reaches its own `main` -- the loader
    * stops it, so there is nothing left to ask about the environment it was
    * handed. Which is why `emptied` is "empty except for the loader path" here
    * and empty outright on POSIX.
    *
    * Only when the parent has one: run by hand without a `PATH`, the loader
    * finds the DLLs another way and both environments are handed over exactly
    * as written above.
    */
   {
      char   *path;

      path = getenv( "PATH");
      if( path && strlen( path) + sizeof( "PATH=") < sizeof( loader))
      {
         strcpy( loader, "PATH=");
         strcat( loader, path);

         replacement[ 1] = loader;
         emptied[ 0]     = loader;
      }
   }
#endif

   mulle_spawn_options_init( &options);
   options.out_mode = MULLE_SPAWN_PIPE;

   /* NULL: the parent's environment, marker and all */
   memset( buffer, 0, sizeof( buffer));
   n = smoke_capture( argv, &options, buffer, sizeof( buffer));
   check( "environment: inherited, open returns 0", n >= 0);
   check( "environment: inherited",
          ! strcmp( buffer, "MULLE_SPAWN_MARKER=inherited\n"
                            "MULLE_SPAWN_TEST=<unset>\n"));
   smoke_show_lines( "environment: inherited", buffer);

   /* a given one replaces rather than adds to the parent's */
   options.env = replacement;

   memset( buffer, 0, sizeof( buffer));
   n = smoke_capture( argv, &options, buffer, sizeof( buffer));
   check( "environment: replaced, open returns 0", n >= 0);
   check( "environment: replaced",
          ! strcmp( buffer, "MULLE_SPAWN_MARKER=<unset>\n"
                            "MULLE_SPAWN_TEST=replaced\n"));
   smoke_show_lines( "environment: replaced", buffer);

   /* an empty environment is empty, and not the parent's */
   options.env = emptied;

   memset( buffer, 0, sizeof( buffer));
   n = smoke_capture( argv, &options, buffer, sizeof( buffer));
   check( "environment: emptied, open returns 0", n >= 0);
   check( "environment: emptied",
          ! strcmp( buffer, "MULLE_SPAWN_MARKER=<unset>\n"
                            "MULLE_SPAWN_TEST=<unset>\n"));
   smoke_show_lines( "environment: emptied", buffer);
}


int   main( int argc, char **argv)
{
   /* the child modes have to come first: they are this program too. --segv
    * takes no argument, so argc is 2 for it and child_main does the checking */
   if( argc >= 2)
   {
      int   rval;

      rval = child_main( argc, argv);
      if( rval != -1)
         return( rval);
   }

#ifdef _WIN32
   /*
    * The record this test is compared against is written with \n on every
    * platform: a CRT would turn every one of ours into \r\n here, and then the
    * Windows output would differ from the Linux and macOS one everywhere
    * instead of only where the platform really behaves differently.
    */
   (void) _setmode( _fileno( stdout), _O_BINARY);
#endif

   verbose = argc >= 2 &&
             ( ! strcmp( argv[ 1], "-v") || ! strcmp( argv[ 1], "--verbose"));

   memset( self_path, 0, sizeof( self_path));
   self_found = mulle_spawn_get_executable_path( self_path, sizeof( self_path)) > 0;

   check( "self path: found", self_found);
   check( "self path: names this program",
          self_found &&
          ! strcmp( smoke_basename( self_path), smoke_basename( argv[ 0])));
   if( verbose)
      mulle_printf( "     self path: %s\n", self_path);

   if( ! self_found)
      return( 1);

   test_echo();
   test_timeout();
   test_cannot_run();
   test_exit_code();
   test_signal();
   test_reexec();

   test_handle_filter();
   test_handle_banner();
   test_handle_close_kills();
   test_handle_cannot_open();
   test_environment();
   test_directory();
   test_descriptor();
   test_detach();
   test_interrupt_suspend();

   mulle_printf( "\n%s (%d failures)\n", failures ? "FAILED" : "all mulle-spawn tests passed",
                 failures);
   return( failures ? 1 : 0);
}
