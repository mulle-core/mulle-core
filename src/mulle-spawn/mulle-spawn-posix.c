/*
 *  mulle-spawn-posix.c
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
#ifndef _WIN32

#include "mulle-spawn.h"
#include "include-private.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#if defined( __APPLE__)
# include <crt_externs.h>
# include <mach-o/dyld.h>

/*
 * Darwin has no `environ` to assign to: the environment lives behind the dyld
 * accessor, which is also what the `environ` every caller sees expands to.
 */
# define mulle_spawn_environ   (*_NSGetEnviron())
#else
extern char **environ;
# define mulle_spawn_environ   environ
#endif


#define MULLE_SPAWN_CHUNK   4096

/*
 * How long `mulle_spawn_open` waits to hear that the program could not be run.
 * Both answers arrive as soon as the child reaches its exec -- EOF if it got
 * there, an errno if it could not -- so this is only a cap on a child that is
 * somehow stuck before exec. It is running, and staying silent, in that case.
 */
#define MULLE_SPAWN_EXEC_TIMEOUT_MS   5000


/*
 * A child that has let go of its stdout is on its way out, but has not
 * necessarily been reaped yet. Waiting a millisecond at a time for that to
 * happen is what keeps a child that *did* finish from being reported as
 * terminated.
 */
static void   mulle_spawn_millisleep( void)
{
   struct timespec   ts = { 0, 1000 * 1000 };

   nanosleep( &ts, NULL);
}


int   mulle_spawn_run( char * const *argv,
                       mulle_relativetime_t timeout,
                       struct mulle_spawn_result *p)
{
   char                  chunk[ MULLE_SPAWN_CHUNK];
   struct pollfd         pfds[ 2];
   mulle_relativetime_t  deadline;
   mulle_relativetime_t  remaining;
   size_t                capacity;
   int                   errfds[ 2];
   int                   fds[ 2];
   int                   failed;
   int                   flags;
   int                   has_deadline;
   int                   killed;
   int                   poll_ms;
   int                   reaped;
   int                   rval;
   int                   spawn_errno;
   int                   status;
   pid_t                 pid;
   pid_t                 waited;
   ssize_t               n;

   if( ! p)
      return( -1);

   capacity = 0;
   failed   = 0;
   killed   = 0;
   reaped   = 0;
   status   = 0;

   _mulle_spawn_result_init( p, p->allocator);

   if( ! argv || ! argv[ 0])
   {
      p->status = mulle_spawn_failed;
      return( -1);
   }

   if( pipe( fds) < 0)
   {
      p->status = mulle_spawn_failed;
      return( -1);
   }

   /*
    * A second pipe carries the errno back if exec fails. Its write end is
    * close-on-exec, so the parent sees EOF the moment the child does start
    * the program -- and four bytes of errno if it never gets that far. That
    * is the only way to tell "could not run it" from "it ran and exited 127",
    * which is the difference between a -1 and a result.
    */
   errfds[ 0] = -1;
   errfds[ 1] = -1;
   if( pipe( errfds) == 0)
   {
      flags = fcntl( errfds[ 1], F_GETFD);
      (void) fcntl( errfds[ 1], F_SETFD, flags | FD_CLOEXEC);
   }

   /* a half written line of ours must not end up in the child's capture */
   fflush( NULL);

   pid = fork();
   if( pid < 0)
   {
      close( fds[ 0]);
      close( fds[ 1]);
      if( errfds[ 0] >= 0)
      {
         close( errfds[ 0]);
         close( errfds[ 1]);
      }
      p->status = mulle_spawn_failed;
      return( -1);
   }

   if( ! pid)
   {
      /* stdout becomes the pipe, stdin and stderr stay as they were */
      close( fds[ 0]);
      if( fds[ 1] != STDOUT_FILENO)
      {
         if( dup2( fds[ 1], STDOUT_FILENO) < 0)
            _exit( 127);
         close( fds[ 1]);
      }

      if( errfds[ 0] >= 0)
      {
         close( errfds[ 0]);
         errfds[ 0] = -1;
      }

      execvp( argv[ 0], argv);

      spawn_errno = errno;
      if( errfds[ 1] >= 0)
      {
         ssize_t   written;

         /* the parent waits for this to learn *why* exec failed. If the
          * write fails, the pipe just closes and the parent falls back to
          * the exit status, so the result is consumed but not used. */
         written = write( errfds[ 1], &spawn_errno, sizeof( spawn_errno));
         (void) written;
      }

      /* and for anyone reading the output by hand, say it out loud */
      fprintf( stderr, "mulle-spawn: cannot run \"%s\": %s\n",
               argv[ 0], strerror( spawn_errno));
      _exit( 127);
   }

   close( fds[ 1]);
   if( errfds[ 1] >= 0)
      close( errfds[ 1]);

   has_deadline = (timeout >= 0.0);
   deadline     = has_deadline ? mulle_relativetime_now() + timeout : 0.0;

   pfds[ 0].fd     = fds[ 0];
   pfds[ 0].events = POLLIN;
   pfds[ 1].fd     = errfds[ 0];
   pfds[ 1].events = POLLIN;

   for(;;)
   {
      poll_ms = -1;

      if( has_deadline)
      {
         remaining = deadline - mulle_relativetime_now();
         if( remaining <= 0.0)
         {
            killed = 1;
            break;
         }
         poll_ms = (int) (remaining * 1000.0);
         if( poll_ms < 1)
            poll_ms = 1;
      }

      pfds[ 0].revents = 0;
      pfds[ 1].revents = 0;
      rval             = poll( pfds, 2, poll_ms);
      if( rval < 0)
      {
         if( errno == EINTR)
            continue;
         break;
      }

      if( ! rval)
      {
         killed = 1;      /* only reachable when there is a deadline */
         break;
      }

      if( pfds[ 1].revents)
      {
         n = read( pfds[ 1].fd, &spawn_errno, sizeof( spawn_errno));
         if( n == (ssize_t) sizeof( spawn_errno))
         {
            errno  = spawn_errno;
            failed = 1;
            break;
         }
         /* EOF: the exec happened, and this pipe is done */
         close( errfds[ 0]);
         pfds[ 1].fd = -1;
      }

      if( ! pfds[ 0].revents)
         continue;

      n = read( fds[ 0], chunk, sizeof( chunk));
      if( n < 0)
      {
         if( errno == EINTR)
            continue;
         break;
      }
      if( ! n)
         break;           /* EOF: the child let go of its stdout */

      if( _mulle_spawn_append( p, &capacity, chunk, (size_t) n))
      {
         failed = 1;      /* out of memory, stop it rather than buffer on */
         break;
      }
   }

   close( fds[ 0]);
   if( pfds[ 1].fd >= 0)
      close( pfds[ 1].fd);

   if( ! killed && ! failed)
   {
      for(;;)
      {
         waited = waitpid( pid, &status, WNOHANG);
         if( waited == pid)
         {
            reaped = 1;
            break;
         }
         if( waited < 0 && errno != EINTR)
            break;
         if( has_deadline && mulle_relativetime_now() >= deadline)
            break;
         mulle_spawn_millisleep();
      }
   }

   /*
    * Whatever the child was doing, it is not doing it any more. A child that
    * did not come back is not an error to report, it is a process to kill.
    */
   if( ! reaped)
   {
      killed = 1;
      (void) kill( pid, SIGKILL);

      /* SIGKILL cannot be blocked, so this drains, and leaves no zombie */
      for(;;)
      {
         if( waitpid( pid, &status, 0) >= 0)
            break;
         if( errno != EINTR)
            break;
      }
   }

   if( failed)
   {
      p->status = mulle_spawn_failed;
      return( -1);
   }

   if( killed)
      p->status = mulle_spawn_terminated;
   else
      if( WIFEXITED( status))
      {
         p->status    = mulle_spawn_exited;
         p->exit_code = WEXITSTATUS( status);
      }
      else
         if( WIFSIGNALED( status))
         {
            p->status        = mulle_spawn_signalled;
            p->signal_number = WTERMSIG( status);
         }
         else
            p->status = mulle_spawn_terminated;

   return( 0);
}


int   mulle_spawn_get_executable_path( char *buf, size_t length)
{
   char    raw[ PATH_MAX];
   char    resolved[ PATH_MAX];
   size_t  len;

   if( ! buf || ! length)
      return( -1);

   raw[ 0] = 0;

#if defined( __APPLE__)
   {
      uint32_t   size;

      size = (uint32_t) sizeof( raw);
      if( _NSGetExecutablePath( raw, &size) != 0)
         return( -1);
   }
#elif defined( __linux__) || defined( __ANDROID__)
   {
      ssize_t   n;

      n = readlink( "/proc/self/exe", raw, sizeof( raw) - 1);
      if( n <= 0)
         return( -1);
      raw[ n] = 0;
   }
#elif defined( __FreeBSD__)
   {
      ssize_t   n;

      n = readlink( "/proc/curproc/file", raw, sizeof( raw) - 1);
      if( n <= 0)
         return( -1);
      raw[ n] = 0;
   }
#else
   return( -1);
#endif

   /* what the OS reports can still be a symlink, which is not re-executable */
   if( ! realpath( raw, resolved))
      return( -1);

   len = strlen( resolved);
   if( len + 1 > length)
      return( -1);

   memcpy( buf, resolved, len + 1);
   return( (int) len);
}

/*
 * -------------------------------------------------------------------- handles
 *
 * The same fork and exec, minus the loop that reads until the child is gone:
 * the caller has the descriptors now, and does its own reading.
 */
struct mulle_spawn_platform
{
   int   status;      /* the raw wait status, once there is one */
   int   reaped;      /* 1 once the child has been waited for */
};


/*
 * Writes the fate into a result, leaving `output` alone: a handle does not
 * capture, so the stream was the caller's and there is nothing here to own or
 * to release.
 */
static void   mulle_spawn_record_fate( int status,
                                       int killed,
                                       struct mulle_spawn_result *p)
{
   if( ! p)
      return;

   p->exit_code     = 0;
   p->signal_number = 0;

   if( killed)
   {
      p->status = mulle_spawn_terminated;
      return;
   }

   if( WIFEXITED( status))
   {
      p->status    = mulle_spawn_exited;
      p->exit_code = WEXITSTATUS( status);
      return;
   }

   if( WIFSIGNALED( status))
   {
      p->status        = mulle_spawn_signalled;
      p->signal_number = WTERMSIG( status);
      return;
   }

   p->status = mulle_spawn_terminated;
}


/*
 * Done in the parent, before anything is created. The child could choke on the
 * directory itself, but its complaint would arrive as an exit code and its own
 * copy of an errno -- and the launch would have "succeeded", which is the
 * wrong answer for a caller who named a directory. `errno` is set, so the
 * reason survives: ENOENT for one that is not there, ENOTDIR for one that is
 * not a directory, EACCES for one that cannot be entered.
 */
static int   mulle_spawn_check_directory( char const *path)
{
   struct stat   st;

   if( ! path)
      return( 0);

   if( stat( path, &st))
      return( -1);

   if( ! S_ISDIR( st.st_mode))
   {
      errno = ENOTDIR;
      return( -1);
   }
   return( 0);
}


/*
 * Checked in the parent, like the directory and for the same reason: a
 * descriptor that is not open would come back as a child that started and died
 * on its first write, which reads as the child's fault and not as the caller's
 * mistake. `fcntl` is the question -- it fails with EBADF, which is what
 * `errno` then says. A negative descriptor is refused without asking, since
 * -1 is the "nothing here" this header uses everywhere.
 */
static int   mulle_spawn_check_descriptor( int fd)
{
   if( fd < 0)
   {
      errno = EBADF;
      return( -1);
   }

   return( fcntl( fd, F_GETFD) < 0 ? -1 : 0);
}


int   mulle_spawn_open_with_options( struct mulle_spawn_handle *h,
                                     char * const *argv,
                                     struct mulle_spawn_options const *options)
{
   /* a zeroed one is the default: inherited environment, inherited streams */
   static struct mulle_spawn_options const   default_options;

   struct mulle_spawn_platform   *platform;
   struct pollfd                 pfd;
   int                           errfds[ 2];
   int                           execfds[ 2];
   int                           infds[ 2];
   int                           outfds[ 2];
   int                           child_err;
   int                           child_in;
   int                           child_out;
   int                           flags;
   int                           n;
   int                           spawn_errno;
   pid_t                         pid;

   MULLE_C_ASSERT( sizeof( struct mulle_spawn_platform) <= sizeof( h->platform));

   if( ! h || ! argv || ! argv[ 0])
      return( -1);

   if( ! options)
      options = &default_options;

   memset( h, 0, sizeof( *h));
   h->in  = -1;
   h->out = -1;
   h->err = -1;

   platform         = (struct mulle_spawn_platform *) h->platform;
   platform->status = 0;
   platform->reaped = 0;

   infds[ 0]  = infds[ 1]  = -1;
   outfds[ 0] = outfds[ 1] = -1;
   errfds[ 0] = errfds[ 1] = -1;
   execfds[ 0] = execfds[ 1] = -1;

   child_in  = -1;
   child_out = -1;
   child_err = -1;

   /*
    * `child_X` is the fd the child should end up with on standard descriptor
    * X, or -1 to leave that one alone. Only a pipe also leaves the parent
    * something to hold on to.
    */
   switch( options->in_mode)
   {
   case MULLE_SPAWN_DISCARD :
      child_in = open( "/dev/null", O_RDWR);
      break;

   case MULLE_SPAWN_PIPE :
      if( pipe( infds))
         goto fail;
      child_in = infds[ 0];
      break;

   case MULLE_SPAWN_DESCRIPTOR :
      child_in = options->in_descriptor;
      if( mulle_spawn_check_descriptor( child_in))
         goto fail;
      break;

   default :
      break;
   }

   if( child_in < 0 && options->in_mode != MULLE_SPAWN_INHERIT)
      goto fail;

   switch( options->out_mode)
   {
   case MULLE_SPAWN_DISCARD :
      child_out = open( "/dev/null", O_RDWR);
      break;

   case MULLE_SPAWN_PIPE :
      if( pipe( outfds))
         goto fail;
      child_out = outfds[ 1];
      break;

   case MULLE_SPAWN_DESCRIPTOR :
      child_out = options->out_descriptor;
      if( mulle_spawn_check_descriptor( child_out))
         goto fail;
      break;

   default :
      break;
   }

   if( child_out < 0 && options->out_mode != MULLE_SPAWN_INHERIT)
      goto fail;

   switch( options->err_mode)
   {
   case MULLE_SPAWN_DISCARD :
      child_err = open( "/dev/null", O_RDWR);
      break;

   case MULLE_SPAWN_PIPE :
      if( pipe( errfds))
         goto fail;
      child_err = errfds[ 1];
      break;

   case MULLE_SPAWN_DESCRIPTOR :
      child_err = options->err_descriptor;
      if( mulle_spawn_check_descriptor( child_err))
         goto fail;
      break;

   default :
      break;
   }

   if( child_err < 0 && options->err_mode != MULLE_SPAWN_INHERIT)
      goto fail;

   if( mulle_spawn_check_directory( options->directory))
      goto fail;

   /*
    * A third pipe carries the errno back if the exec fails, its write end
    * close-on-exec. That is what lets `open` tell "no such program" from
    * "started and still running": the parent reads EOF the moment the child
    * reaches its exec, and four bytes of errno if it never gets that far.
    * Without it the caller would hold a handle to a child that was never
    * anything, which on Windows CreateProcess refuses to do at all.
    */
   if( pipe( execfds) == 0)
   {
      flags = fcntl( execfds[ 1], F_GETFD);
      (void) fcntl( execfds[ 1], F_SETFD, flags | FD_CLOEXEC);
   }

   /* a half written line of ours must not end up in the child's stream */
   fflush( NULL);

   pid = fork();
   if( pid < 0)
      goto fail;

   if( ! pid)
   {
      if( execfds[ 0] >= 0)
         close( execfds[ 0]);

      /* each one moves onto its standard descriptor and then lets the
       * original go, whether it was a pipe end or the null device */
      if( child_in >= 0 && child_in != STDIN_FILENO)
      {
         if( dup2( child_in, STDIN_FILENO) < 0)
            _exit( 127);
         close( child_in);
      }

      if( child_out >= 0 && child_out != STDOUT_FILENO)
      {
         if( dup2( child_out, STDOUT_FILENO) < 0)
            _exit( 127);
         close( child_out);
      }

      if( child_err >= 0 && child_err != STDERR_FILENO)
      {
         if( dup2( child_err, STDERR_FILENO) < 0)
            _exit( 127);
         close( child_err);
      }

      /* the parent's ends are not the child's business */
      if( infds[ 1] >= 0)
         close( infds[ 1]);
      if( outfds[ 0] >= 0)
         close( outfds[ 0]);
      if( errfds[ 0] >= 0)
         close( errfds[ 0]);

      /*
       * `execvp` resolves `argv[ 0]` against the PATH of the *current*
       * environment, and on every platform that is this pointer -- so
       * replacing it is what makes a replaced PATH the one that gets searched,
       * which is what a caller who passes an environment means. It is the
       * child's own copy of the global, taken after the fork, so the parent's
       * cannot be affected.
       *
       * Guarded on the pointer and not on what it points at: `{ NULL }` is an
       * environment of nothing, and NULL is the parent's.
       */
      if( options->env)
         mulle_spawn_environ = (char **) options->env;

      /*
       * Checked in the parent, so this should not fail -- but "should not" is
       * not a reason to let a child run in the wrong directory if it does. The
       * errno goes back over the same pipe the exec failure uses, so the parent
       * refuses the launch exactly as it would for a program that is not there.
       */
      if( options->directory && chdir( options->directory))
      {
         spawn_errno = errno;
         fprintf( stderr, "mulle-spawn: cannot change to \"%s\": %s\n",
                  options->directory, strerror( spawn_errno));
         goto child_cannot_run;
      }

      execvp( argv[ 0], argv);

      spawn_errno = errno;
      fprintf( stderr, "mulle-spawn: cannot run \"%s\": %s\n",
               argv[ 0], strerror( spawn_errno));

child_cannot_run:
      if( execfds[ 1] >= 0)
      {
         ssize_t   written;

         /* the parent waits for this to learn *why*. If the write fails, the
          * pipe just closes and the parent falls back to the exit status, so
          * the result is consumed but not used. */
         written = write( execfds[ 1], &spawn_errno, sizeof( spawn_errno));
         (void) written;
      }

      _exit( 127);
   }

   /* the child's ends, and whatever the child already took over. Setting the
    * variables to -1 as they go keeps the failure path from closing twice */
   if( infds[ 0] >= 0)
      close( infds[ 0]);
   if( outfds[ 1] >= 0)
      close( outfds[ 1]);
   if( errfds[ 1] >= 0)
      close( errfds[ 1]);
   if( execfds[ 1] >= 0)
      close( execfds[ 1]);

   infds[ 0]   = -1;
   outfds[ 1]  = -1;
   errfds[ 1]  = -1;
   execfds[ 1] = -1;

   /*
    * Did it get to its exec? EOF says yes, four bytes say no. Either answer is
    * immediate; only a child stuck before exec reaches the cap, and that one
    * is running and saying nothing, which is what it will go on doing.
    */
   if( execfds[ 0] >= 0)
   {
      pfd.fd      = execfds[ 0];
      pfd.events  = POLLIN;
      pfd.revents = 0;

      n = poll( &pfd, 1, MULLE_SPAWN_EXEC_TIMEOUT_MS);
      if( n > 0)
      {
         n = (int) read( execfds[ 0], &spawn_errno, sizeof( spawn_errno));
         if( n == (int) sizeof( spawn_errno))
         {
            /* it is on its way out by itself, but it must not be left a zombie */
            (void) kill( pid, SIGKILL);
            for(;;)
            {
               if( waitpid( pid, NULL, 0) >= 0)
                  break;
               if( errno != EINTR)
                  break;
            }

            close( execfds[ 0]);
            execfds[ 0] = -1;

            errno = spawn_errno;
            goto fail;
         }
      }

      close( execfds[ 0]);
      execfds[ 0] = -1;
   }

   /* a null device opened for the child is not the parent's to keep */
   if( options->in_mode == MULLE_SPAWN_DISCARD)
      close( child_in);
   if( options->out_mode == MULLE_SPAWN_DISCARD)
      close( child_out);
   if( options->err_mode == MULLE_SPAWN_DISCARD)
      close( child_err);

   h->in     = infds[ 1];
   h->out    = outfds[ 0];
   h->err    = errfds[ 0];
   h->pid    = (unsigned long) pid;
   h->opened = 1;
   return( 0);

fail:
   if( infds[ 0] >= 0)
      close( infds[ 0]);
   if( infds[ 1] >= 0)
      close( infds[ 1]);
   if( outfds[ 0] >= 0)
      close( outfds[ 0]);
   if( outfds[ 1] >= 0)
      close( outfds[ 1]);
   if( errfds[ 0] >= 0)
      close( errfds[ 0]);
   if( errfds[ 1] >= 0)
      close( errfds[ 1]);
   if( execfds[ 0] >= 0)
      close( execfds[ 0]);
   if( execfds[ 1] >= 0)
      close( execfds[ 1]);

   if( options->in_mode == MULLE_SPAWN_DISCARD && child_in >= 0)
      close( child_in);
   if( options->out_mode == MULLE_SPAWN_DISCARD && child_out >= 0)
      close( child_out);
   if( options->err_mode == MULLE_SPAWN_DISCARD && child_err >= 0)
      close( child_err);

   h->in  = -1;
   h->out = -1;
   h->err = -1;
   return( -1);
}


int   mulle_spawn_wait( struct mulle_spawn_handle *h,
                        mulle_relativetime_t timeout,
                        struct mulle_spawn_result *p)
{
   struct mulle_spawn_platform   *platform;
   mulle_relativetime_t          deadline;
   int                           has_deadline;
   int                           status;
   pid_t                         waited;

   if( ! h || ! h->opened)
      return( -1);

   platform = (struct mulle_spawn_platform *) h->platform;
   if( platform->reaped)
   {
      mulle_spawn_record_fate( platform->status, 0, p);
      return( 1);
   }

   has_deadline = (timeout >= 0.0);
   deadline     = has_deadline ? mulle_relativetime_now() + timeout : 0.0;

   for(;;)
   {
      waited = waitpid( (pid_t) h->pid, &status, WNOHANG);
      if( waited == (pid_t) h->pid)
      {
         platform->status = status;
         platform->reaped = 1;
         h->opened        = 0;
         h->pid           = 0;
         mulle_spawn_record_fate( status, 0, p);
         return( 1);
      }

      if( waited < 0 && errno != EINTR)
         return( -1);

      if( has_deadline && mulle_relativetime_now() >= deadline)
         return( 0);

      mulle_spawn_millisleep();
   }
}


int   mulle_spawn_terminate( struct mulle_spawn_handle *h)
{
   struct mulle_spawn_platform   *platform;

   if( ! h || ! h->opened)
      return( -1);

   platform = (struct mulle_spawn_platform *) h->platform;
   if( platform->reaped)
      return( 0);

   /* SIGTERM, so a child that means to tidy up first still can */
   if( kill( (pid_t) h->pid, SIGTERM) < 0)
      return( -1);

   return( 0);
}


int   mulle_spawn_close( struct mulle_spawn_handle *h,
                         struct mulle_spawn_result *p)
{
   struct mulle_spawn_platform   *platform;
   int                           killed;
   int                           status;

   if( ! h)
      return( -1);

   platform = (struct mulle_spawn_platform *) h->platform;
   killed   = 0;

   /*
    * Only a handle that was opened has a platform to look at. One that was
    * merely initialized closes its descriptors, of which it has none, and
    * nothing else -- that is the whole reason handle_init exists.
    */
   if( h->opened)
   {
      if( ! platform->reaped)
      {
         /*
          * A child that has not come back is not an error to report, it is a
          * process to kill. SIGKILL cannot be blocked, so this does not wait
          * for a cooperation that may never come.
          */
         (void) kill( (pid_t) h->pid, SIGKILL);
         killed = 1;

         for(;;)
         {
            if( waitpid( (pid_t) h->pid, &status, 0) >= 0)
            {
               platform->status = status;
               platform->reaped = 1;
               break;
            }
            if( errno != EINTR)
               break;
         }
      }

      if( p && platform->reaped)
         mulle_spawn_record_fate( platform->status, killed, p);
   }

   if( h->in >= 0)
      close( h->in);
   if( h->out >= 0)
      close( h->out);
   if( h->err >= 0)
      close( h->err);

   memset( h, 0, sizeof( *h));
   h->in  = -1;
   h->out = -1;
   h->err = -1;
   return( 0);
}


/*
 * The opposite of close: the child is not killed and not waited for, only let
 * go. There is nothing platform specific to do beyond dropping the descriptors
 * -- the process is not ours to reap or to hold, and the waiting that would
 * have been done here is what is being given up.
 *
 * Note that the pid is dropped with the rest. A caller that wants it has to
 * read `h->pid` before calling this, and after this the kernel is the only
 * thing that knows the child exists until it exits, when init collects it.
 */
int   mulle_spawn_detach( struct mulle_spawn_handle *h)
{
   if( ! h)
      return( -1);

   if( h->in >= 0)
      close( h->in);
   if( h->out >= 0)
      close( h->out);
   if( h->err >= 0)
      close( h->err);

   memset( h, 0, sizeof( *h));
   h->in  = -1;
   h->out = -1;
   h->err = -1;
   return( 0);
}

#endif // !_WIN32
