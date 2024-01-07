//
//  mulle-stacktrace-execinfo.c
//  mulle-core
//
//  Created by Nat! on 04.11.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#if ! (defined( _WIN32) || defined( __MULLE_MUSL__))

#define _GNU_SOURCE

#include "mulle-stacktrace.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>

// clang speciality
#ifdef __has_include
# if __has_include( <dlfcn.h>)
#  include <dlfcn.h>
#  define HAVE_DLSYM  1
# endif
#endif



#ifdef __APPLE__
//
// it gives 0   libMulleStandaloneObjC.dylib        0x000000010ab7e46d stacktrace + 61
// move past the address
static char   *_trim_belly_fat( char *s)
{
   char   *hex;
   char   *sym;

   hex = strstr( s, " 0x");
   if( ! hex)
      return( s);
   hex++;

   sym = strchr( hex, ' ');
   if( ! sym)
      return( hex);

   while( *sym == ' ')
      ++sym;
   if( ! *sym)
      return( hex);

   return( sym);
}


static int   _trim_arse_fat( char *s)
{
   char   *offset;

   offset = strstr( s, " + ");
   if( ! offset)
      return( (int) strlen( s));

   return( (int) (offset - s));
}

#else

//
// it gives /lib64/ld-linux-x86-64.so.2(+0x10ca) [0x7ffff7fd40ca]
// or  /home/src/../libMulleObjC-standalone.so(_mulle_objc_universe_crunch+0x176) [0x7ffff7f6fa5c]
static char   *_trim_belly_fat( char *s)
{
   char   *hex;
   char   *memo;

   // if we have (+0x), then remove leading path but keep filename
   hex = strstr( s, "(+0x");
   if( hex)
   {
      memo = s;
      while( s < hex)
      {
         s = strchr( s, '/');
         if( ! s)
            return( memo);
         memo = ++s;
      }
      return( memo);
   }

   // remove boring filename path
   memo = s;
   s = strchr( s, '(');
   return( s ? s : memo);
}


static int   _trim_arse_fat( char *s)
{
   char   *offset;

   offset = strstr( s, " [0x");
   if( ! offset)
      return( (int) strlen( s));

   return( (int) (offset - s));
}

#endif

static char  *_symbolize_nothing( void *adresse, size_t max, char *buf, size_t len, void **userinfo)
{
   return( NULL);
}


static char   *_keep_belly_fat( char *s)
{
   return( s);
}


static int   _keep_arse_fat( char *s)
{
   return( (int) strlen( s));
}


#define stracktrace_has_prefix( s, prefix)   ! strncmp( s, prefix, sizeof( prefix) - 1)

static int   _trim_boring_functions( char *s, int size)
{
   if( size == -1)
      size = strlen( s);

   if( size == 3 && ! strncmp( s, "0x0", 3))
      return( 1);

   if( stracktrace_has_prefix( s, "test_calloc_or_raise"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_realloc_or_raise"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_realloc"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_calloc"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_free"))
      return( 1);
   if( stracktrace_has_prefix( s, "libmulle-testallocator"))
      return( 1);

   //
   if( stracktrace_has_prefix( s, "mulle_objc"))
      return( 1);
   if( stracktrace_has_prefix( s, "_mulle_objc"))
      return( 1);

   return( 0);
}


static int   keep_boring_functions( char *s, int size)
{
   return( 0);
}


#ifndef HAVE_DLSYM

static int   _dump_less_shabby( struct mulle_stacktrace *stacktrace,
                               char *s,
                               FILE *fp,
                               enum mulle_stacktrace_format format,
                               char *delim)
{
   int   size;

   if( format == mulle_stacktrace_normal)
   {
      fprintf( fp, "%s%s", delim, s);
      return( 0);
   }

   s    = stacktrace->trim_belly_fat( s);
   size = stacktrace->trim_arse_fat( s);

   if( stacktrace->is_boring( s, size))
      return( 1);

   fprintf( fp, "%s%.*s", delim, size, s);
   return( 0);
}



static void  _shabby_default_dump( struct mulle_stacktrace *stacktrace,
                                  void **callstack,
                                  int frames,
                                  int offset,
                                  FILE *fp,
                                  char *delimchar,
                                  enum mulle_stacktrace_format format)
{
   char   **strs;
   char   **p;
   char   *delim;
   char   **sentinel;
   char   *s;

   strs     = backtrace_symbols( callstack, frames);
   p        = &strs[ frames];
   sentinel = &strs[ offset];

   delim = "";
   while( p > sentinel)
   {
      s = *--p;
      if( ! _dump_less_shabby( stacktrace, s, fp, format, delim))
         delim = delimchar;
   }
   free( strs);
}

#else

static struct mulle_stacktrace   dummy =
{
   _symbolize_nothing,
   _trim_belly_fat,
   _trim_arse_fat,
   _trim_boring_functions
};


static void   mulle_stacktrace_dump( struct mulle_stacktrace *stacktrace,
                                     void **callstack,
                                     int frames,
                                     int offset,
                                     FILE *fp,
                                     char *delimchar,
                                     enum mulle_stacktrace_format format)
{
   char        *delim;
   char        *s;
   Dl_info     info;
   ptrdiff_t   diff;
   size_t      max;
   int         havedl;
   void        **p;
   void        **sentinel;
   void        *address;
   void        *userinfo;
//   char        **strs;
   char        buf[ 512];
   int         i;

   // non underscore shouldn't crash for NULL params
   if( ! stacktrace || ! callstack || ! fp || ! delimchar)
      return;

   p        = &callstack[ frames];
   sentinel = &callstack[ offset];

   userinfo = NULL;
   stacktrace->symbolize( NULL, 0, NULL, 0, &userinfo);

   i = 0;
   while( p > sentinel)
   {
      delim = (i == 0) ? "" : delimchar;
      ++i;

      address = *--p;
      max     = 0x800;
      //musl doesn't have  it ?
      havedl  = dladdr( address, &info);
      if( havedl && info.dli_saddr)
      {
         max = (intptr_t) address - (intptr_t) info.dli_saddr;
         if( max > 0x800)
            max = 0x800;
      }

      // try to improve on max with symbolizer
      if( max)
      {
         s = stacktrace->symbolize( address, max - 1, buf, sizeof( buf), &userinfo);
         if( s)
         {
            if( ! stacktrace->is_boring( s, -1))
               fprintf( fp, "%s %s", delim, s);
            continue;
         }
      }

      if( havedl)
      {
         if( info.dli_sname)
         {
            if( ! stacktrace->is_boring( (char *) info.dli_sname, -1))
            {
               diff = (intptr_t) address - (intptr_t) info.dli_saddr;
               if( diff)
                  fprintf( fp, "%s %s+0x%0lx", delim, info.dli_sname, (long) diff);
               else
                  fprintf( fp, "%s %s", delim, info.dli_sname);
            }
            continue;
         }

         if( info.dli_fname)
         {
            s = strrchr( info.dli_fname, '/');
            if( s)
               s = &s[ 1];
            else
               s = (char *) info.dli_fname;

            // relative address of shared lib is not really useful
            fprintf( fp, "%s %s:%p", delim, s, address);
            continue;
         }
      }

#if 0  // bringt nix, kann auch nicht mehr
      strs = backtrace_symbols( p, 1);
      _dump_less_shabby( stacktrace, *strs, fp, format, delim);
      free( strs);
#endif
      fprintf( fp, "%s %p", delim, address);
   }
   stacktrace->symbolize( NULL, (size_t) -1, NULL, 0, &userinfo);
}

#endif


void   _mulle_stacktrace( struct mulle_stacktrace *stacktrace,
                          int offset,
                          enum mulle_stacktrace_format format,
                          FILE *fp)
{
   char   *delimchar;

   if( ! stacktrace)
      stacktrace = &dummy;
   if( ! fp)
      fp = stderr;

   delimchar = "\n";
   if( ! (format & mulle_stacktrace_linefeed))
   {
      fprintf( fp, " : [");
      delimchar = " |";
   }

   {
      void   *callstack[ 256];
      int    frames;

      frames   = backtrace( callstack, 256);
#ifdef HAVE_DLSYM
      mulle_stacktrace_dump( stacktrace, callstack, frames, offset, fp, delimchar, format);
#else
      _shabby_default_dump( stacktrace, callstack, frames, offset, fp, delimchar, format);
#endif
   }

   fputc( (format & mulle_stacktrace_linefeed) ? '\n' : ']', fp);
}


int   mulle_stacktrace_count_frames( void)
{
   void   *callstack[ 256];
   int    frames;

   frames = backtrace( callstack, 256);
   return( frames);
}


void   _mulle_stacktrace_init_default( struct mulle_stacktrace *stacktrace)
{
   stacktrace->symbolize      = _symbolize_nothing;
   stacktrace->trim_belly_fat = _trim_belly_fat;
   stacktrace->trim_arse_fat  = _trim_arse_fat;
   stacktrace->is_boring      = _trim_boring_functions;
}


void   _mulle_stacktrace_init( struct mulle_stacktrace *stacktrace,
                               mulle_stacktrace_symbolizer_t *p_symbolize,
                               char *(*p__trim_belly_fat)( char *),
                               int (*p__trim_arse_fat)( char *),
                               int (*p_is_boring)( char *, int size))
{
   stacktrace->symbolize       = p_symbolize       ? p_symbolize       : _symbolize_nothing;
   stacktrace->trim_belly_fat  = p__trim_belly_fat ? p__trim_belly_fat : _keep_belly_fat;
   stacktrace->trim_arse_fat   = p__trim_arse_fat  ? p__trim_arse_fat  : _keep_arse_fat;
   stacktrace->is_boring       = p_is_boring       ? p_is_boring       : keep_boring_functions;
}

#endif
