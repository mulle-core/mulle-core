/*
 *  mulle-spawn.h
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
#ifndef mulle_spawn_h__
#define mulle_spawn_h__

#include "include.h"

#include <stddef.h>
#include <stdint.h>


#define MULLE__SPAWN_VERSION  ((1UL << 20) | (0 << 8) | 0)


static inline unsigned int   mulle_spawn_get_version_major( void)
{
   return( MULLE__SPAWN_VERSION >> 20);
}


static inline unsigned int   mulle_spawn_get_version_minor( void)
{
   return( (MULLE__SPAWN_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_spawn_get_version_patch( void)
{
   return( MULLE__SPAWN_VERSION & 0xFF);
}


MULLE__SPAWN_GLOBAL
uint32_t   mulle_spawn_get_version( void);



/*
 * Run a program and collect what it prints, under a deadline.
 *
 * This is the portable "bounded child": a subprocess, its `stdout`, and a
 * wall-clock limit that is enforced rather than merely reported. It exists
 * because `fork()` is not a portable way to get a second process, and because
 * the interesting failure mode of a child is not a wrong answer but no answer
 * at all -- which no amount of error checking inside the child can cover.
 *
 * The contract, which is deliberately narrow so it can be the same code on
 * every platform:
 *
 *   - `argv[ 0]` is the program. It is resolved like the C library would
 *     resolve it (`execvp` semantics, so `PATH` is honored). Pass an absolute
 *     path if you mean one, and see `mulle_spawn_get_executable_path` for the
 *     "run me again" case.
 *   - The child's `stdout` is replaced by a pipe and captured. It is captured
 *     *only* as bytes; mulle-spawn does not know it is text. It is NUL
 *     terminated for convenience, but `length` is the truth. The one byte
 *     mulle-spawn does touch is the `\r` of a `\r\n` pair, which a Windows CRT
 *     inserts behind the child's back for every `\n` it writes: that is undone,
 *     so a child that printed `a\nb\n` reads as `a\nb\n` on every platform.
 *   - `stdin` and `stderr` are inherited unchanged. `stderr` is left alone on
 *     purpose: it is where a message about the child belongs, and where the
 *     caller's own diagnostics should be able to survive it.
 *   - The child runs to completion, or until `timeout` seconds have elapsed,
 *     whichever comes first. A negative `timeout` means "no deadline".
 *   - When the deadline passes the child is terminated unconditionally, and
 *     `mulle_spawn_terminated` is the verdict. There is no cooperative
 *     shutdown: the cases worth covering are the ones that cannot cooperate.
 *
 * Output and termination are independent. A child that printed something and
 * then outlived its deadline still leaves that output in `output`, with
 * `mulle_spawn_terminated` in `status`. Callers that asked a question usually
 * want the output; callers measuring liveness usually want the status.
 *
 * On POSIX a child that cannot be executed at all exits with 127, the
 * convention a shell uses, and explains itself on `stderr`.
 */
enum mulle_spawn_status
{
   mulle_spawn_unknown    = 0,   /* nothing was run (or the result was reset) */
   mulle_spawn_failed     = 1,   /* no child process was created at all */
   mulle_spawn_exited     = 2,   /* the child exited on its own, see exit_code */
   mulle_spawn_signalled  = 3,   /* POSIX: it died from signal_number */
   mulle_spawn_terminated = 4    /* it outlived the deadline and was killed */
};


/*
 * `output` is allocated with `allocator` (which may be NULL for the default
 * allocator) and is owned by this struct: hand it to `mulle_spawn_result_done`
 * when done. `signal_number` stays 0 on platforms without signals.
 */
struct mulle_spawn_result
{
   char                     *output;
   size_t                   length;
   int                      status;          /* enum mulle_spawn_status */
   int                      exit_code;
   int                      signal_number;

   struct mulle_allocator   *allocator;
};


static inline void   _mulle_spawn_result_init( struct mulle_spawn_result *p,
                                               struct mulle_allocator *allocator)
{
   p->output        = NULL;
   p->length        = 0;
   p->status        = mulle_spawn_unknown;
   p->exit_code     = 0;
   p->signal_number = 0;
   p->allocator     = allocator;
}


static inline void   mulle_spawn_result_init( struct mulle_spawn_result *p,
                                               struct mulle_allocator *allocator)
{
   if( p)
      _mulle_spawn_result_init( p, allocator);
}


/* releases `output` and resets the struct, the allocator is kept */
MULLE__SPAWN_GLOBAL
void   mulle_spawn_result_done( struct mulle_spawn_result *p);


/*
 * Internal: appends to the capture, growing it as needed. `capacity` is the
 * size of the `output` allocation and is carried by the caller, because it is
 * not part of the result. Shared by the platform backends, so it is declared
 * here rather than duplicated in both.
 */
int   _mulle_spawn_append( struct mulle_spawn_result *p,
                           size_t *capacity,
                           char *bytes,
                           size_t length);


/* run forever */
#define MULLE_SPAWN_NO_TIMEOUT   (-1.0)


/*
 * Returns 0 if a child process was created and its fate is in `p`, -1 if not
 * (in which case `p->status` is `mulle_spawn_failed` and `p->output` is NULL).
 *
 * `p->output` may be NULL with a non-zero `length` being impossible: an
 * empty capture is `output == NULL, length == 0`.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_run( char * const *argv,
                       mulle_relativetime_t timeout,
                       struct mulle_spawn_result *p);


/*
 * The path of the running executable, as UTF-8, for the "run me again" case.
 * Returns the length of the path, or -1 if it does not fit in `buf` (of
 * `length` bytes) or cannot be determined on this platform.
 *
 * The result is what the OS reports, which for a program invoked through a
 * symlink is typically the resolved path, not the name it was called by.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_get_executable_path( char *buf, size_t length);


/*
 * Run a program and *keep* it: the handle shaped half.
 *
 * `mulle_spawn_run` waits for the child to come back, which is the wrong shape
 * for a server, a filter, or anything else you intend to talk to. A handle is
 * the same spawn without the waiting -- `open` starts the program, the ends of
 * its standard streams come back as descriptors, and it stays alive until you
 * terminate it and close the handle.
 *
 * What a handle deliberately does *not* do is capture. There is one reader for
 * a pipe and here the caller is it; capturing would take that away and hand
 * back a buffer that arrives too late to answer with. So `wait` and `close`
 * report the child's fate and leave `output` NULL -- the stream was yours.
 *
 * The descriptors are read/write descriptors, `read`/`write` on POSIX and the
 * same names on the Windows CRT over the pipe, so a caller speaking a protocol
 * over them is the same code on both. They are binary: nothing translates
 * underneath a caller that has its own idea of line endings.
 */
#define MULLE_SPAWN_INHERIT   0   /* the parent's own stream, untouched */
#define MULLE_SPAWN_PIPE      1   /* a pipe, and a descriptor in the handle */
#define MULLE_SPAWN_DISCARD   2   /* the null device, for output nobody wants */


/*
 * Everything `open` needs besides the program and its arguments.
 *
 * A struct rather than a longer argument list because these are independent of
 * each other, most callers want none of them, and one can be added without
 * every existing call site changing. A zeroed struct *is* the default -- an
 * inherited environment and three inherited streams -- because
 * `MULLE_SPAWN_INHERIT` is 0 and `env` is NULL, so `mulle_spawn_options_init`
 * exists for the reader rather than for the backend.
 */
struct mulle_spawn_options
{
   /*
    * The child's environment as `NAME=VALUE` strings, NULL terminated -- `envp`
    * as `execve` takes it. It *replaces* the parent's environment rather than
    * being merged into it, so:
    *
    *    NULL              the parent's environment, unchanged
    *    { "A=B", NULL }   that one variable, and nothing else
    *    { NULL }          no environment at all
    *
    * Not copied and not kept: it has to stay valid until `open` returns, which
    * is when the child is given it.
    *
    * A replaced `PATH` is the one that gets searched for `argv[ 0]` -- the
    * child is looked up the way the environment it is about to receive would
    * resolve it, not the way the parent's would.
    */
   char * const   *env;

   /*
    * The directory the child runs in, or NULL for the one it would inherit.
    *
    * A directory that is not there, or is not a directory, fails the launch
    * rather than starting a program in the wrong place -- which is the failure
    * this exists to prevent, because a child that quietly runs somewhere else
    * looks exactly like a child that worked. On POSIX the check happens in the
    * parent and `errno` says why; on Windows `CreateProcessW` refuses it
    * itself.
    *
    * Not copied and not kept: it has to stay valid until `open` returns.
    */
   char const     *directory;

   int            in_mode;
   int            out_mode;
   int            err_mode;
};


static inline void   _mulle_spawn_options_init( struct mulle_spawn_options *p)
{
   p->env       = NULL;
   p->directory = NULL;
   p->in_mode   = MULLE_SPAWN_INHERIT;
   p->out_mode  = MULLE_SPAWN_INHERIT;
   p->err_mode  = MULLE_SPAWN_INHERIT;
}


static inline void   mulle_spawn_options_init( struct mulle_spawn_options *p)
{
   if( p)
      _mulle_spawn_options_init( p);
}


/*
 * `in`, `out` and `err` are descriptors owned by this handle: `close` closes
 * them. If you close one yourself, set the field to -1 so it is not closed
 * twice -- descriptor numbers get reused, and the second close would land on
 * somebody else's.
 *
 * `pid` is the OS process id, for the parts of the world that are addressed
 * that way (asking the kernel how much memory the child holds, for instance).
 * It is 0 once the child has been reaped.
 */
struct mulle_spawn_handle
{
   int                      in;
   int                      out;
   int                      err;

   unsigned long            pid;
   int                      opened;

   /*
    * The backend's own state. Opaque on purpose: this header is included
    * everywhere, and windows.h or sys/wait.h have no business being. Each
    * backend static asserts that what it puts here fits.
    */
   void                     *platform[ 4];
};


/*
 * A handle has to be a handle before it is a handle. Nothing here is opened,
 * which is what makes `close` safe on one that never was: it closes no
 * descriptor and touches no process, because there is none to close.
 *
 * `open` resets the struct itself, so this is only needed for a handle that may
 * never get opened -- a target reached over a socket somebody else started,
 * say, where the closing code runs either way.
 */
static inline void   mulle_spawn_handle_init( struct mulle_spawn_handle *h)
{
   if( h)
   {
      h->in     = -1;
      h->out    = -1;
      h->err    = -1;
      h->pid    = 0;
      h->opened = 0;
   }
}


/*
 * Starts `argv` and returns 0, or -1 if the program could not be run at all --
 * no process was created, or one was and it never reached its exec. Nothing is
 * left open then, the handle is reset, and on POSIX `errno` says why.
 *
 * That last part is worth the round trip: by the time `open` returns 0 the
 * program *is* running, so a caller never has to tell "cannot start it" from
 * "it started and died at once" by reading a closed pipe. The exception is
 * Windows, where CreateProcessW will hand back a process that dies a moment
 * later for want of a DLL -- a failure it does not report and cannot.
 *
 * The three modes say what becomes of the child's stdin, stdout and stderr, in
 * that order. The child inherits the parent's environment; for one it does not,
 * use `mulle_spawn_open_with_options`.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_open( struct mulle_spawn_handle *h,
                        char * const *argv,
                        int in_mode,
                        int out_mode,
                        int err_mode);


/*
 * `mulle_spawn_open` with the launch described rather than fixed. Today that is
 * the environment the child starts with and the directory it runs in, neither
 * of which `open` can say anything about; the same call grows the rest of what
 * a launch can be set to as it is needed.
 *
 * `options` may be NULL for the defaults. This is the real entry point --
 * `mulle_spawn_open` is this call with a zeroed struct -- and it returns 0 or
 * -1 exactly as that one does, `errno` included.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_open_with_options( struct mulle_spawn_handle *h,
                                     char * const *argv,
                                     struct mulle_spawn_options const *options);


/*
 * Waits up to `timeout` seconds for the child to stop by itself.
 *
 * Returns 1 if it has stopped (and reaps it, so there is no zombie left
 * behind), 0 if it is still running, -1 on error. `MULLE_SPAWN_NO_TIMEOUT`
 * waits for as long as it takes. `p` may be NULL; if it is not, the fate is
 * recorded in it.
 *
 * A stopped child is not necessarily a successful one, and `p->status` is what
 * says which it was, exactly as it would from `mulle_spawn_run`.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_wait( struct mulle_spawn_handle *h,
                        mulle_relativetime_t timeout,
                        struct mulle_spawn_result *p);


/*
 * Asks the child to stop. POSIX sends SIGTERM; Windows calls TerminateProcess,
 * because there is no SIGTERM there to send and an abrupt stop is all the
 * platform offers.
 *
 * It does not wait, so on POSIX a child that means to tidy up first is given
 * the chance to: terminate, wait, then close. Nothing is reaped here.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_terminate( struct mulle_spawn_handle *h);


/*
 * Releases the handle -- the descriptors, the process and the backend's own
 * state -- so that close always leaves nothing behind. A child that is still
 * running is terminated unconditionally first, because close is not the place
 * for cooperation, and is then reaped. `p` may be NULL, and a child that had
 * already stopped reports the fate it stopped with.
 *
 * Afterwards the handle is reset and `pid` is 0.
 */
MULLE__SPAWN_GLOBAL
int   mulle_spawn_close( struct mulle_spawn_handle *h,
                         struct mulle_spawn_result *p);



/*
 * The following files are auto-generated.
 * The _mulle-spawn-provide header is generated by
 * mulle-match-to-c during a reflect.
 */
#include "_mulle-spawn-provide.h"

/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-spawn-versioncheck.h")
#  include "_mulle-spawn-versioncheck.h"
# endif
#endif

#endif
