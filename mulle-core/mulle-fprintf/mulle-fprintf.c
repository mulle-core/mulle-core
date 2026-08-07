//
//  mulle-fprintf.c
//  mulle-fprintf
//
//  Copyright (c) 2021 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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
/*
 * template: demo/library/PROJECT_SOURCE_DIR/PROJECT_NAME.PROJECT_EXTENSION
 * vendor/extension: mulle-sde/c-demo
 */
#include "include-private.h"

#include "mulle-fprintf.h"

#include <errno.h>


uint32_t   mulle_fprintf_get_version( void)
{
   return( MULLE__FPRINTF_VERSION);
}


int   mulle_printf( char *format, ...)
{
   va_list   args;
   int       rval;

   va_start( args, format);
   rval = mulle_vfprintf( stdout, format, args);
   va_end( args);

   return( rval);
}


int   mulle_fprintf( FILE *fp, char *format, ...)
{
   va_list   args;
   int       rval;

   va_start( args, format);
   rval = mulle_vfprintf( fp, format, args);
   va_end( args);

   return( rval);
}


int   mulle_vfprintf( FILE *fp, char *format, va_list args)
{
   struct mulle_buffer            *buffer;
   struct mulle_flushablebuffer   flushable_buffer;
   char                           storage[ 1024];  // storage for buffer
   int                            rval;
   int                            rval2;

   if( ! fp || ! format)
   {
   	errno = EINVAL;
      return( -1);
   }

   mulle_flushablebuffer_init_with_static_bytes( &flushable_buffer,
                                                 storage,
                                                 sizeof( storage),
                                                 (mulle_flushablebuffer_flusher_t *) fwrite,
                                                 fp,
                                                 NULL);

   buffer = mulle_flushablebuffer_as_buffer( &flushable_buffer);
   rval   = mulle_buffer_vsprintf( buffer, format, args);
   rval2  = mulle_flushablebuffer_done( &flushable_buffer);

   return( rval2 ? rval2 : rval);
}


int   mulle_mvfprintf( FILE *fp, char *format, mulle_vararg_list arguments)
{
   struct mulle_buffer            *buffer;
   struct mulle_flushablebuffer   flushable_buffer;
   char                           storage[ 1024];  // storage for buffer
   int                            rval;
   int                            rval2;

   if( ! fp || ! format)
   {
   	errno = EINVAL;
      return( -1);
   }

   mulle_flushablebuffer_init( &flushable_buffer,
                               storage,
                               sizeof( storage),
                               (mulle_flushablebuffer_flusher_t *) fwrite,
                               fp);

   buffer = mulle_flushablebuffer_as_buffer( &flushable_buffer);
   rval   = mulle_buffer_mvsprintf( buffer, format, arguments);
   rval2  = mulle_flushablebuffer_done( &flushable_buffer);

   return( rval2 ? rval2 : rval);
}

