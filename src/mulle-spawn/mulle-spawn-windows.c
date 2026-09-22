/*
 *  mulle-spawn-windows.c
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
#ifdef _WIN32

#include "mulle-spawn.h"
#include "include-private.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>


#define MULLE_SPAWN_CHUNK   4096


/*
 * The command line, assembled a wchar at a time. It is built in UTF-16 because
 * that is what CreateProcessW takes; quoting a converted copy afterwards would
 * mean quoting two encodings instead of one.
 */
struct mulle_spawn_wbuffer
{
   wchar_t                  *bytes;
   size_t                   length;
   size_t                   capacity;
   struct mulle_allocator   *allocator;
};


static int   mulle_spawn_wbuffer_append( struct mulle_spawn_wbuffer *p,
                                         const wchar_t *s,
                                         size_t length)
{
   wchar_t   *q;
   size_t    size;

   if( p->length + length + 1 > p->capacity)
   {
      size = p->capacity ? p->capacity * 2 : 256;
      while( size < p->length + length + 1)
         size *= 2;

      q = mulle_allocator_realloc( p->allocator, p->bytes,
                                   size * sizeof( wchar_t));
      if( ! q)
         return( -1);

      p->bytes    = q;
      p->capacity = size;
   }

   memcpy( p->bytes + p->length, s, length * sizeof( wchar_t));
   p->length           += length;
   p->bytes[ p->length] = 0;
   return( 0);
}


static int   mulle_spawn_wbuffer_append_char( struct mulle_spawn_wbuffer *p,
                                              wchar_t c)
{
   return( mulle_spawn_wbuffer_append( p, &c, 1));
}


/*
 * Appends one argument, quoted so the C runtime in the child hands back
 * exactly these bytes. These are the rules the runtime documents (and that
 * CommandLineToArgvW implements): a backslash is literal unless a double quote
 * follows it, in which case backslashes double -- and one extra backslash
 * makes the quote itself literal.
 */
static int   mulle_spawn_wbuffer_append_argument( struct mulle_spawn_wbuffer *p,
                                                  const wchar_t *arg)
{
   size_t   backslashes;
   size_t   i;
   int      quoted;

   quoted = ( ! *arg) || (wcspbrk( arg, L" \t\n\v\"") != NULL);
   if( quoted)
   {
      if( mulle_spawn_wbuffer_append_char( p, L'"'))
         return( -1);
   }

   for(;;)
   {
      backslashes = 0;
      while( *arg == L'\\')
      {
         arg++;
         backslashes++;
      }

      if( ! *arg)
      {
         /* trailing backslashes precede the closing quote, so they double */
         if( quoted)
            backslashes *= 2;
         for( i = 0; i < backslashes; i++)
         {
            if( mulle_spawn_wbuffer_append_char( p, L'\\'))
               return( -1);
         }
         if( ! quoted)
            return( 0);
         return( mulle_spawn_wbuffer_append_char( p, L'"'));
      }

      /* a quote is escaped by the backslashes before it, and needs one more */
      if( *arg == L'"')
         backslashes = backslashes * 2 + 1;

      for( i = 0; i < backslashes; i++)
      {
         if( mulle_spawn_wbuffer_append_char( p, L'\\'))
            return( -1);
      }
      if( mulle_spawn_wbuffer_append_char( p, *arg))
         return( -1);
      arg++;
   }
}


/*
 * mulle-spawn speaks UTF-8, Windows speaks UTF-16. Converting at the boundary
 * keeps a non-ASCII path or argument from being mangled by the active code
 * page -- which is also why a consumer of a spawned child wants the UTF-8
 * application manifest, or the child will mangle it right back.
 */
static wchar_t   *mulle_spawn_utf8_to_utf16( struct mulle_allocator *allocator,
                                             const char *s)
{
   wchar_t   *result;
   int       size;

   if( ! s)
      return( NULL);

   size = MultiByteToWideChar( CP_UTF8, 0, s, -1, NULL, 0);
   if( size <= 0)
      return( NULL);

   result = mulle_allocator_malloc( allocator, (size_t) size * sizeof( wchar_t));
   if( ! result)
      return( NULL);

   if( ! MultiByteToWideChar( CP_UTF8, 0, s, -1, result, size))
   {
      mulle_allocator_free( allocator, result);
      return( NULL);
   }
   return( result);
}


static char   *mulle_spawn_utf16_to_utf8( struct mulle_allocator *allocator,
                                          const wchar_t *s)
{
   char   *result;
   int    size;

   if( ! s)
      return( NULL);

   size = WideCharToMultiByte( CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
   if( size <= 0)
      return( NULL);

   result = mulle_allocator_malloc( allocator, (size_t) size);
   if( ! result)
      return( NULL);

   if( ! WideCharToMultiByte( CP_UTF8, 0, s, -1, result, size, NULL, NULL))
   {
      mulle_allocator_free( allocator, result);
      return( NULL);
   }
   return( result);
}


/*
 * A child inherits a handle only if the handle was made inheritable, and the
 * standard handles we were given are not ours to re-flag. Duplicating them
 * yields an inheritable copy whichever way they were created.
 *
 * `did_duplicate` tells the caller whether it owns the result and has to close
 * it -- closing the parent's own stdin or stderr would be a nasty surprise.
 */
static HANDLE   mulle_spawn_inheritable_handle( DWORD std, int *did_duplicate)
{
   HANDLE   handle;
   HANDLE   result;

   *did_duplicate = 0;

   handle = GetStdHandle( std);
   if( ! handle || handle == INVALID_HANDLE_VALUE)
      return( INVALID_HANDLE_VALUE);

   if( DuplicateHandle( GetCurrentProcess(), handle,
                        GetCurrentProcess(), &result,
                        0, TRUE, DUPLICATE_SAME_ACCESS))
   {
      *did_duplicate = 1;
      return( result);
   }

   /* nothing to duplicate: use it as it is and hope it is inheritable */
   return( handle);
}


/*
 * Undo the one thing the platform adds behind the child's back. A Windows CRT
 * writes a text mode stream with \r\n for every \n the program wrote, and that
 * is what arrives down the pipe. Removing the \r of a \r\n pair is exactly the
 * inverse of that translation and nothing else: a lone \r is left alone, and a
 * program that deliberately wrote \r\n gets its \r\n back rather than \r\r\n.
 *
 * Without it the same child reads differently on Windows than on POSIX, and
 * every caller would have to know which platform it is on -- the one thing a
 * portable spawn must not ask.
 */
static void   mulle_spawn_undouble_newlines( char *bytes, size_t *length)
{
   size_t   i;
   size_t   n;
   size_t   out;

   n   = *length;
   out = 0;

   for( i = 0; i < n; i++)
   {
      if( bytes[ i] == '\r' && i + 1 < n && bytes[ i + 1] == '\n')
         continue;
      bytes[ out++] = bytes[ i];
   }

   bytes[ out] = 0;
   *length     = out;
}


/*
 * The command line CreateProcessW wants, and the application name beside it.
 * They are two allocations rather than one because the caller frees them at
 * different moments: the application name has to outlive the call, the command
 * line is handed over to CreateProcessW.
 */
struct mulle_spawn_command
{
   wchar_t                  *application;
   wchar_t                  *commandline;
   struct mulle_allocator   *allocator;
};


static int   mulle_spawn_command_init( struct mulle_spawn_command *p,
                                       char * const *argv,
                                       struct mulle_allocator *allocator)
{
   struct mulle_spawn_wbuffer   wbuf;
   wchar_t                      **wargv;
   int                          failed;
   int                          i;
   int                          nargs;

   p->application = NULL;
   p->commandline = NULL;
   p->allocator   = allocator;

   wbuf.bytes     = NULL;
   wbuf.length    = 0;
   wbuf.capacity  = 0;
   wbuf.allocator = allocator;

   for( nargs = 0; argv[ nargs]; nargs++)
      ;

   wargv = mulle_allocator_malloc( allocator,
                                   (size_t) (nargs + 1) * sizeof( wchar_t *));
   if( ! wargv)
      return( -1);

   /* zeroed, so the cleanup path can free a half built array */
   memset( wargv, 0, (size_t) (nargs + 1) * sizeof( wchar_t *));
   failed = 1;

   for( i = 0; i < nargs; i++)
   {
      wargv[ i] = mulle_spawn_utf8_to_utf16( allocator, argv[ i]);
      if( ! wargv[ i])
         goto done;
   }

   /*
    * The child parses this itself, and its argv[ 0] is the first token here --
    * not necessarily the application name we hand CreateProcessW.
    */
   for( i = 0; i < nargs; i++)
   {
      if( i)
      {
         if( mulle_spawn_wbuffer_append_char( &wbuf, L' '))
            goto done;
      }
      if( mulle_spawn_wbuffer_append_argument( &wbuf, wargv[ i]))
         goto done;
   }

   p->application = wargv[ 0];
   p->commandline = wbuf.bytes;

   wargv[ 0]  = NULL;      /* now the command's, not the scratch array's */
   wbuf.bytes = NULL;
   failed     = 0;

done:
   for( i = 0; i < nargs; i++)
   {
      if( wargv[ i])
         mulle_allocator_free( allocator, wargv[ i]);
   }
   mulle_allocator_free( allocator, wargv);

   if( wbuf.bytes)
      mulle_allocator_free( allocator, wbuf.bytes);

   return( failed ? -1 : 0);
}


static void   mulle_spawn_command_done( struct mulle_spawn_command *p)
{
   if( p->application)
      mulle_allocator_free( p->allocator, p->application);
   if( p->commandline)
      mulle_allocator_free( p->allocator, p->commandline);

   p->application = NULL;
   p->commandline = NULL;
}


/*
 * The environment block CreateProcessW wants: the `NAME=VALUE` strings as
 * UTF-16, each with its own terminator, and one more to end the block. It is
 * the one thing in here that is not a string.
 *
 * The caller's order is kept. Windows documents the block as sorted
 * alphabetically, and has never required it -- the child walks it, and the
 * block NSTask builds is built the same way.
 *
 * `capacity` is measured first and then filled, so a failure to convert costs
 * a NULL rather than a half written block. The count includes the extra
 * terminator, which is why the arithmetic below is exactly in bounds.
 */
static wchar_t   *mulle_spawn_environment_block( struct mulle_allocator *allocator,
                                                 char * const *env)
{
   size_t    offset;
   size_t    capacity;
   wchar_t   *block;
   int       count;
   int       i;
   int       n;

   capacity = 2;                 /* the two terminators, which an empty block is */
   for( count = 0; env[ count]; count++)
      capacity += (size_t) MultiByteToWideChar( CP_UTF8, 0, env[ count], -1, NULL, 0);

   block = mulle_allocator_malloc( allocator, capacity * sizeof( wchar_t));
   if( ! block)
      return( NULL);

   offset = 0;
   for( i = 0; i < count; i++)
   {
      n = MultiByteToWideChar( CP_UTF8, 0, env[ i], -1,
                               &block[ offset], (int) (capacity - offset));
      if( ! n)
      {
         mulle_allocator_free( allocator, block);
         return( NULL);
      }
      offset += (size_t) n;
   }

   block[ offset]     = 0;
   block[ offset + 1] = 0;
   return( block);
}


int   mulle_spawn_run( char * const *argv,
                       mulle_relativetime_t timeout,
                       struct mulle_spawn_result *p)
{
   char                        chunk[ MULLE_SPAWN_CHUNK];
   struct mulle_spawn_command  command;
   PROCESS_INFORMATION         pi = { 0 };
   SECURITY_ATTRIBUTES         sa = { 0 };
   STARTUPINFOW                si = { 0 };
   mulle_relativetime_t        deadline;
   size_t                      capacity;
   DWORD                       available;
   DWORD                       exit_code;
   DWORD                       nread;
   HANDLE                      hErr;
   HANDLE                      hIn;
   HANDLE                      hRead;
   HANDLE                      hWrite;
   int                         failed;
   int                         has_deadline;
   int                         killed;
   int                         own_err;
   int                         own_in;
   int                         reaped;
   int                         rval;

   if( ! p)
      return( -1);

   command.application = NULL;
   command.commandline = NULL;

   hErr   = INVALID_HANDLE_VALUE;
   hIn    = INVALID_HANDLE_VALUE;
   hRead  = INVALID_HANDLE_VALUE;
   hWrite = INVALID_HANDLE_VALUE;

   capacity = 0;
   failed   = 0;
   killed   = 0;
   own_err  = 0;
   own_in   = 0;
   reaped   = 0;
   rval     = -1;

   _mulle_spawn_result_init( p, p->allocator);

   if( ! argv || ! argv[ 0])
   {
      p->status = mulle_spawn_failed;
      return( -1);
   }

   if( mulle_spawn_command_init( &command, argv, p->allocator))
      goto fail;

   sa.nLength              = sizeof( sa);
   sa.lpSecurityDescriptor = NULL;
   sa.bInheritHandle       = TRUE;

   if( ! CreatePipe( &hRead, &hWrite, &sa, 0))
      goto fail;

   /* only the write end is meant for the child */
   (void) SetHandleInformation( hRead, HANDLE_FLAG_INHERIT, 0);

   hIn  = mulle_spawn_inheritable_handle( STD_INPUT_HANDLE, &own_in);
   hErr = mulle_spawn_inheritable_handle( STD_ERROR_HANDLE, &own_err);

   si.cb         = sizeof( si);
   si.dwFlags    = STARTF_USESTDHANDLES;
   si.hStdInput  = hIn;
   si.hStdOutput = hWrite;
   si.hStdError  = hErr;

   if( ! CreateProcessW( command.application, command.commandline,
                         NULL, NULL, TRUE,
                         CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
      goto fail;

   CloseHandle( hWrite);
   hWrite = INVALID_HANDLE_VALUE;

   deadline     = 0.0;
   has_deadline = (timeout >= 0.0);
   if( has_deadline)
      deadline = mulle_relativetime_now() + timeout;

   for(;;)
   {
      /*
       * An anonymous pipe offers no way to wait for data with a deadline, so
       * ask how much is there and read only that. Once the child is gone and
       * the pipe is drained this reports a broken pipe, which is the EOF.
       */
      if( ! PeekNamedPipe( hRead, NULL, 0, NULL, &available, NULL))
         break;

      if( available)
      {
         if( ! ReadFile( hRead, chunk, sizeof( chunk), &nread, NULL))
            break;
         if( ! nread)
            break;

         if( _mulle_spawn_append( p, &capacity, chunk, (size_t) nread))
         {
            failed = 1;
            break;
         }
         continue;
      }

      if( has_deadline && mulle_relativetime_now() >= deadline)
      {
         killed = 1;
         break;
      }
      Sleep( 1);
   }

   CloseHandle( hRead);
   hRead = INVALID_HANDLE_VALUE;

   if( ! killed && ! failed)
   {
      for(;;)
      {
         if( WaitForSingleObject( pi.hProcess, 0) == WAIT_OBJECT_0)
         {
            reaped = 1;
            break;
         }
         if( has_deadline && mulle_relativetime_now() >= deadline)
            break;
         Sleep( 1);
      }
   }

   /*
    * Whatever the child was doing, it is not doing it any more. TerminateProcess
    * is unconditional, which is exactly the property a deadline needs.
    */
   if( ! reaped)
   {
      killed = 1;
      (void) TerminateProcess( pi.hProcess, 1);
      (void) WaitForSingleObject( pi.hProcess, INFINITE);
   }

   /* the capture is complete, so what the child wrote can be recovered */
   if( p->output)
      mulle_spawn_undouble_newlines( p->output, &p->length);

   if( failed)
   {
      p->status = mulle_spawn_failed;
      goto done;
   }

   /*
    * A child was created, so from here on its fate *is* the result: -1 is only
    * for "nothing was run at all". A termination by deadline included, which is
    * what the POSIX backend reads too.
    */
   if( killed)
      p->status = mulle_spawn_terminated;
   else
   {
      exit_code = 0;
      (void) GetExitCodeProcess( pi.hProcess, &exit_code);

      p->status    = mulle_spawn_exited;
      p->exit_code = (int) exit_code;
   }

   rval = 0;

done:
   if( hRead != INVALID_HANDLE_VALUE)
      CloseHandle( hRead);
   if( hWrite != INVALID_HANDLE_VALUE)
      CloseHandle( hWrite);
   if( own_in && hIn != INVALID_HANDLE_VALUE)
      CloseHandle( hIn);
   if( own_err && hErr != INVALID_HANDLE_VALUE)
      CloseHandle( hErr);

   if( pi.hThread)
      CloseHandle( pi.hThread);
   if( pi.hProcess)
      CloseHandle( pi.hProcess);

   mulle_spawn_command_done( &command);

   return( rval);

fail:
   p->status = mulle_spawn_failed;
   goto done;
}


int   mulle_spawn_get_executable_path( char *buf, size_t length)
{
   char      *utf8;
   wchar_t   wide[ 4096];
   size_t    len;
   size_t    wide_size;
   DWORD     size;

   if( ! buf || ! length)
      return( -1);

   wide_size = sizeof( wide) / sizeof( wide[ 0]);
   size      = GetModuleFileNameW( NULL, wide, (DWORD) wide_size);
   if( ! size || size == (DWORD) wide_size)
      return( -1);

   utf8 = mulle_spawn_utf16_to_utf8( NULL, wide);
   if( ! utf8)
      return( -1);

   len = strlen( utf8);
   if( len + 1 > length)
   {
      mulle_allocator_free( NULL, utf8);
      return( -1);
   }

   memcpy( buf, utf8, len + 1);
   mulle_allocator_free( NULL, utf8);
   return( (int) len);
}

/*
 * -------------------------------------------------------------------- handles
 *
 * The same CreateProcess, minus the loop that reads until the child is gone:
 * the caller has the descriptors now, and does its own reading.
 */
struct mulle_spawn_platform
{
   HANDLE   process;
   HANDLE   thread;
   DWORD    exit_code;
   int      reaped;
};


#define MULLE_SPAWN_PROCESS(h)   ((struct mulle_spawn_platform *) (h)->platform)


/*
 * Turns a pipe end into a descriptor, which is what a caller wants if it is to
 * speak the same protocol on this platform as on the other one. The descriptor
 * owns the handle from here on -- and `_open_osfhandle` only takes it when it
 * succeeds, so a failure still has to close it.
 */
static int   mulle_spawn_take_fd( HANDLE *p_handle, int flags)
{
   intptr_t   fd;

   fd = _open_osfhandle( (intptr_t) *p_handle, flags);
   if( fd == -1)
   {
      CloseHandle( *p_handle);
      *p_handle = INVALID_HANDLE_VALUE;
      return( -1);
   }

   *p_handle = INVALID_HANDLE_VALUE;
   return( (int) fd);
}


static HANDLE   mulle_spawn_null_handle( DWORD access)
{
   SECURITY_ATTRIBUTES   sa;

   sa.nLength              = sizeof( sa);
   sa.lpSecurityDescriptor = NULL;
   sa.bInheritHandle       = TRUE;

   return( CreateFileW( L"NUL", access, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        &sa, OPEN_EXISTING, 0, NULL));
}


static void   mulle_spawn_record_fate( DWORD exit_code,
                                       int killed,
                                       struct mulle_spawn_result *p)
{
   if( ! p)
      return;

   p->exit_code     = 0;
   p->signal_number = 0;   /* there are no signals here, whatever it did */

   if( killed)
   {
      p->status = mulle_spawn_terminated;
      return;
   }

   /*
    * A child that died of a fault arrives as a non zero exit code in the top
    * half of the range -- there is no signal_number to put it in and the enum
    * has no "crashed", so the code is reported as the platform gives it.
    */
   p->status    = mulle_spawn_exited;
   p->exit_code = (int) exit_code;
}


int   mulle_spawn_open_with_options( struct mulle_spawn_handle *h,
                                     char * const *argv,
                                     struct mulle_spawn_options const *options)
{
   /* a zeroed one is the default: inherited environment, inherited streams */
   static struct mulle_spawn_options const   default_options;

   struct mulle_spawn_command    command;
   struct mulle_spawn_platform   *platform;
   PROCESS_INFORMATION           pi    = { 0 };
   SECURITY_ATTRIBUTES           sa    = { 0 };
   STARTUPINFOW                  si    = { 0 };
   wchar_t                       *directory   = NULL;
   wchar_t                       *environment = NULL;
   HANDLE                        child_err  = INVALID_HANDLE_VALUE;
   HANDLE                        child_in   = INVALID_HANDLE_VALUE;
   HANDLE                        child_out  = INVALID_HANDLE_VALUE;
   HANDLE                        parent_err = INVALID_HANDLE_VALUE;
   HANDLE                        parent_in  = INVALID_HANDLE_VALUE;
   HANDLE                        parent_out = INVALID_HANDLE_VALUE;
   int                           created;
   int                           own_err;
   int                           own_in;
   int                           own_out;
   int                           rval;

   MULLE_C_ASSERT( sizeof( struct mulle_spawn_platform) <= sizeof( h->platform));

   if( ! h || ! argv || ! argv[ 0])
      return( -1);

   if( ! options)
      options = &default_options;

   memset( h, 0, sizeof( *h));
   h->in  = -1;
   h->out = -1;
   h->err = -1;

   platform          = MULLE_SPAWN_PROCESS( h);
   platform->process = NULL;
   platform->thread  = NULL;
   platform->reaped  = 0;

   command.application = NULL;
   command.commandline = NULL;

   created = 0;
   own_err = 0;
   own_in  = 0;
   own_out = 0;
   rval    = -1;

   sa.nLength              = sizeof( sa);
   sa.lpSecurityDescriptor = NULL;
   sa.bInheritHandle       = TRUE;

   if( mulle_spawn_command_init( &command, argv, NULL))
      return( -1);

   /*
    * For each of the three streams: what the child gets, and what we keep.
    * INHERIT duplicates, because our own standard handles are not ours to
    * re-flag as inheritable; DISCARD is the null device; PIPE gives the parent
    * the end it needs and leaves only the child's end inheritable.
    */
   switch( options->in_mode)
   {
   case MULLE_SPAWN_PIPE :
      if( ! CreatePipe( &child_in, &parent_in, &sa, 0))
         goto done;
      (void) SetHandleInformation( parent_in, HANDLE_FLAG_INHERIT, 0);
      break;

   case MULLE_SPAWN_DISCARD :
      child_in = mulle_spawn_null_handle( GENERIC_READ);
      break;

   default :
      child_in = mulle_spawn_inheritable_handle( STD_INPUT_HANDLE, &own_in);
      break;
   }

   /* an inherited stream may legitimately be absent; a made one may not */
   if( child_in == INVALID_HANDLE_VALUE && options->in_mode != MULLE_SPAWN_INHERIT)
      goto done;

   switch( options->out_mode)
   {
   case MULLE_SPAWN_PIPE :
      if( ! CreatePipe( &parent_out, &child_out, &sa, 0))
         goto done;
      (void) SetHandleInformation( parent_out, HANDLE_FLAG_INHERIT, 0);
      break;

   case MULLE_SPAWN_DISCARD :
      child_out = mulle_spawn_null_handle( GENERIC_WRITE);
      break;

   default :
      child_out = mulle_spawn_inheritable_handle( STD_OUTPUT_HANDLE, &own_out);
      break;
   }

   if( child_out == INVALID_HANDLE_VALUE && options->out_mode != MULLE_SPAWN_INHERIT)
      goto done;

   switch( options->err_mode)
   {
   case MULLE_SPAWN_PIPE :
      if( ! CreatePipe( &parent_err, &child_err, &sa, 0))
         goto done;
      (void) SetHandleInformation( parent_err, HANDLE_FLAG_INHERIT, 0);
      break;

   case MULLE_SPAWN_DISCARD :
      child_err = mulle_spawn_null_handle( GENERIC_WRITE);
      break;

   default :
      child_err = mulle_spawn_inheritable_handle( STD_ERROR_HANDLE, &own_err);
      break;
   }

   if( child_err == INVALID_HANDLE_VALUE && options->err_mode != MULLE_SPAWN_INHERIT)
      goto done;

   si.cb         = sizeof( si);
   si.dwFlags    = STARTF_USESTDHANDLES;
   si.hStdInput  = child_in;
   si.hStdOutput = child_out;
   si.hStdError  = child_err;

   /*
    * NULL is CreateProcessW's "the same environment as the parent", which is
    * what an absent `env` means -- so a block is only built when there is one
    * to build.
    */
   if( options->env)
   {
      environment = mulle_spawn_environment_block( NULL, options->env);
      if( ! environment)
         goto done;
   }

   /*
    * No check of our own here, unlike POSIX: CreateProcessW refuses an
    * `lpCurrentDirectory` that is not a directory, which is the failure the
    * caller asked for. NULL is its "the directory this process is in".
    */
   if( options->directory)
   {
      directory = mulle_spawn_utf8_to_utf16( NULL, options->directory);
      if( ! directory)
         goto done;
   }

   if( ! CreateProcessW( command.application, command.commandline,
                         NULL, NULL, TRUE,
                         CREATE_NO_WINDOW, (LPVOID) environment, directory,
                         &si, &pi))
      goto done;

   created = 1;

   if( parent_in != INVALID_HANDLE_VALUE)
   {
      h->in = mulle_spawn_take_fd( &parent_in, _O_WRONLY | _O_BINARY | _O_NOINHERIT);
      if( h->in < 0)
         goto done;
   }

   if( parent_out != INVALID_HANDLE_VALUE)
   {
      h->out = mulle_spawn_take_fd( &parent_out, _O_RDONLY | _O_BINARY | _O_NOINHERIT);
      if( h->out < 0)
         goto done;
   }

   if( parent_err != INVALID_HANDLE_VALUE)
   {
      h->err = mulle_spawn_take_fd( &parent_err, _O_RDONLY | _O_BINARY | _O_NOINHERIT);
      if( h->err < 0)
         goto done;
   }

   platform->process = pi.hProcess;
   platform->thread  = pi.hThread;
   pi.hProcess       = NULL;
   pi.hThread        = NULL;

   h->pid    = (unsigned long) GetProcessId( platform->process);
   h->opened = 1;
   rval      = 0;

done:
   /* the child's ends are not the parent's to keep, started or not */
   if( child_in != INVALID_HANDLE_VALUE && (own_in || options->in_mode != MULLE_SPAWN_INHERIT))
      CloseHandle( child_in);
   if( child_out != INVALID_HANDLE_VALUE && (own_out || options->out_mode != MULLE_SPAWN_INHERIT))
      CloseHandle( child_out);
   if( child_err != INVALID_HANDLE_VALUE && (own_err || options->err_mode != MULLE_SPAWN_INHERIT))
      CloseHandle( child_err);

   /*
    * A handle that cannot be handed over whole is not worth handing over: a
    * child nobody holds the descriptors for is a leak with extra steps, so it
    * goes down with the attempt.
    */
   if( created && rval)
   {
      (void) TerminateProcess( pi.hProcess, 1);
      (void) WaitForSingleObject( pi.hProcess, INFINITE);
   }

   if( pi.hThread)
      CloseHandle( pi.hThread);
   if( pi.hProcess)
      CloseHandle( pi.hProcess);

   if( rval)
   {
      if( parent_in != INVALID_HANDLE_VALUE)
         CloseHandle( parent_in);
      if( parent_out != INVALID_HANDLE_VALUE)
         CloseHandle( parent_out);
      if( parent_err != INVALID_HANDLE_VALUE)
         CloseHandle( parent_err);

      if( h->in >= 0)
         _close( h->in);
      if( h->out >= 0)
         _close( h->out);
      if( h->err >= 0)
         _close( h->err);

      h->in  = -1;
      h->out = -1;
      h->err = -1;
   }

   if( directory)
      mulle_allocator_free( NULL, directory);
   if( environment)
      mulle_allocator_free( NULL, environment);

   mulle_spawn_command_done( &command);
   return( rval);
}


int   mulle_spawn_wait( struct mulle_spawn_handle *h,
                        mulle_relativetime_t timeout,
                        struct mulle_spawn_result *p)
{
   struct mulle_spawn_platform   *platform;
   DWORD                         exit_code;
   DWORD                         milliseconds;
   DWORD                         waited;

   if( ! h || ! h->opened)
      return( -1);

   platform = MULLE_SPAWN_PROCESS( h);
   if( platform->reaped)
   {
      mulle_spawn_record_fate( platform->exit_code, 0, p);
      return( 1);
   }

   milliseconds = (timeout < 0.0) ? INFINITE : (DWORD) (timeout * 1000.0);

   waited = WaitForSingleObject( platform->process, milliseconds);
   if( waited == WAIT_TIMEOUT)
      return( 0);
   if( waited != WAIT_OBJECT_0)
      return( -1);

   exit_code = 0;
   if( ! GetExitCodeProcess( platform->process, &exit_code))
      return( -1);

   platform->exit_code = exit_code;
   platform->reaped    = 1;
   h->opened           = 0;
   h->pid              = 0;

   mulle_spawn_record_fate( exit_code, 0, p);
   return( 1);
}


int   mulle_spawn_terminate( struct mulle_spawn_handle *h)
{
   struct mulle_spawn_platform   *platform;

   if( ! h || ! h->opened)
      return( -1);

   platform = MULLE_SPAWN_PROCESS( h);
   if( platform->reaped || ! platform->process)
      return( 0);

   /* there is no SIGTERM to send, so this is as abrupt as the platform gets */
   if( ! TerminateProcess( platform->process, 1))
      return( -1);

   return( 0);
}


int   mulle_spawn_close( struct mulle_spawn_handle *h,
                         struct mulle_spawn_result *p)
{
   struct mulle_spawn_platform   *platform;
   DWORD                         exit_code;
   int                           killed;

   if( ! h)
      return( -1);

   platform = MULLE_SPAWN_PROCESS( h);
   killed   = 0;

   /*
    * Only a handle that was opened has a platform to look at. One that was
    * merely initialized closes its descriptors, of which it has none, and
    * nothing else -- that is the whole reason handle_init exists.
    */
   if( h->opened)
   {
      if( ! platform->reaped && platform->process)
      {
         /*
          * A child that has not come back is not an error to report, it is a
          * process to end. TerminateProcess is unconditional, which is exactly
          * what close needs.
          */
         (void) TerminateProcess( platform->process, 1);
         killed = 1;
         (void) WaitForSingleObject( platform->process, INFINITE);

         exit_code = 0;
         if( GetExitCodeProcess( platform->process, &exit_code))
         {
            platform->exit_code = exit_code;
            platform->reaped    = 1;
         }
      }

      if( p && platform->reaped)
         mulle_spawn_record_fate( platform->exit_code, killed, p);

      if( platform->thread)
         CloseHandle( platform->thread);
      if( platform->process)
         CloseHandle( platform->process);
   }

   if( h->in >= 0)
      _close( h->in);
   if( h->out >= 0)
      _close( h->out);
   if( h->err >= 0)
      _close( h->err);

   memset( h, 0, sizeof( *h));
   h->in  = -1;
   h->out = -1;
   h->err = -1;
   return( 0);
}

#endif // _WIN32
