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
#if ! (defined( _WIN32) || defined( __MULLE_MUSL__) || defined( __COSMOPOLITAN__) || defined( __MULLE_COSMOPOLITAN__) || defined(__emscripten__))

#define _GNU_SOURCE

#include "mulle-stacktrace.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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


// csv output:
//
//    address,symbol,symbol_offset,segment_name,segment_offset
//
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
   char        *segment_name;
   char        *symbol_name;
   char        buf[ 512];
   Dl_info     info;
   int         havedl;
   int         i;
   intptr_t    segment_offset;
   intptr_t    symbol_offset;
   size_t      max;
   void        **p;
   void        **sentinel;
   void        *address;
   void        *segment_address;
   void        *symbol_address;
   void        *userinfo;
#if 0
   int         symbol_name_length;
   int         segment_name_length;
#endif

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

      address         = *--p;
      symbol_address  = NULL;
      segment_address = NULL;
      symbol_offset   = 0;
      segment_offset  = (intptr_t) address;
      symbol_name     = "";
      segment_name    = "";

      //musl doesn't have  it ?
      max     = 0x800;
      havedl  = dladdr( address, &info);
      if( havedl)
      {
         if( info.dli_saddr)
         {
            symbol_address = info.dli_saddr;
            symbol_offset  = (intptr_t) address - (intptr_t) symbol_address;
            max            = symbol_offset < max ? symbol_offset : max;
         }
         segment_name    = info.dli_fname ? (char *) info.dli_fname : "";
         segment_address = info.dli_fbase;
         segment_offset  = (intptr_t) address - (intptr_t) segment_address;
         symbol_name     = info.dli_sname ? (char *) info.dli_sname : "";
      }

      // try to improve on max with symbolizer, if we aren't close enough
      // complete heuristic
      if( max)
      {
         s = stacktrace->symbolize( address, max - 1, buf, sizeof( buf), &userinfo);
         if( s && (! symbol_name || strcmp( symbol_name, s)))
         {
            symbol_name   = s;
            symbol_offset = 0; // now unknown really
         }
      }

#if 0
      // for some weird reason symbol_name and segment_name have linefeeds
      // at the end
      symbol_name_length  = symbol_name ? strlen( symbol_name) : 0;
      while( symbol_name_length)
      {
         if( isprint( symbol_name[ symbol_name_length - 1]))
            break;
         symbol_name_length--;
      }

      segment_name_length = segment_name ? strlen( segment_name) : 0;
      while( segment_name_length)
      {
         if( isprint( segment_name[ segment_name_length - 1]))
            break;
         segment_name_length--;
      }
#endif

      // address,segment_offset,symbol_offset,symbol_address,symbol_name,segment_address,segment_name
      if( format == mulle_stacktrace_csv)
      {
         fprintf( fp, "%p,0x%tx,0x%td,",
                        address,
                        segment_offset,
                        symbol_offset);
         if( symbol_address)
            fprintf( fp, "%p,\"%s\",",
                        symbol_address,
                        symbol_name);
         else
            fprintf( fp, ",\"%s\",", symbol_name);

         fprintf( fp, "0x%tx,\"%s\"\n",
                      (intptr_t) segment_address,
                      segment_name);

         continue;
      }

      if( havedl)
      {
         if( symbol_name)
         {
            if( stacktrace->is_boring( (char *) symbol_name, -1))
               continue;

            if( symbol_offset)
               fprintf( fp, "%s %s+0x%0lx", delim, symbol_name, (long) symbol_offset);
            else
               fprintf( fp, "%s %s", delim, symbol_name);
         }

         if( segment_name)
         {
            s = strrchr( segment_name, '/');
            if( s)
               s = &s[ 1];
            else
               s = (char *) segment_name;

            // relative address of shared lib is not really useful
            fprintf( fp, "%s %s:0x%tx", delim, s, segment_offset);
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


   // mulle_stacktrace_normal   = 0,
   // mulle_stacktrace_trimmed  = 1,
   // mulle_stacktrace_linefeed = 2,
   // mulle_stacktrace_csv      = 3

   switch( format)
   {
   case mulle_stacktrace_normal  :
   case mulle_stacktrace_trimmed :
      fprintf( fp, " : [");
      delimchar = " |";;
      break;

   case mulle_stacktrace_linefeed :
      delimchar = "\n";
      break;

   default :
      fprintf( fp, "address,segment_offset,symbol_offset,symbol_address,symbol_name,segment_address,segment_name\n");
      delimchar = "\n";
      break;
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

   switch( format)
   {
   case mulle_stacktrace_normal   :                    // fall thru
   case mulle_stacktrace_trimmed  : fputc( ']', fp);
                                    break;

   case mulle_stacktrace_linefeed : fputc( '\n', fp);  // fall thru
   default :                        break;
   }
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
