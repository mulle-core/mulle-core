//
//  mulle-buffer-stdio.h
//  mulle-fprintf
//
//  Copyright (c) 2024 Nat! - Mulle kybernetiK.
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
#ifndef mulle_buffer_stdio_h__
#define mulle_buffer_stdio_h__

#include "include.h"

#include <stdio.h>

// only needed for windows (for off_t)
#ifdef _WIN32
# include <sys/types.h>
#endif

//
// STDIO LIKE INTERFACE
//
// This is neat when you write functions that should work either with
// FILE or a mulle-buffer.
// e.g.
//   int   read_16bytes( char tmp[ 16],
//                       int( *reader)(void *dst, size_t size, size_t nmem, void *buffer),
//                       void *fp)
//  {
//      return( (*reader)( tmp, 1, 16, fp) == 16)
//  }
//
//  either:  read_16bytes( tmp, fread, fp) or
//  or:      read_16bytes( tmp, mulle_buffer_fread, &buffer)
//
// or using `struct mulle_buffer_stdio_functions`
//
//   int   read_16bytes( char tmp[ 16],
//                       struct mulle_buffer_stdio_functions *functions,
//                       void *fp)
//  {
//      return( (*functions->fread)( tmp, 1, 16, fp) == 16)
//  }
//
//  either:  read_16bytes( tmp, &mulle_stdio_functions, fp) or
//  or:      read_16bytes( tmp, &mulle_buffer_functions, &buffer)
//

/**
 *
 *  r   The stream is opened for reading.
 *  w   The stream is opened for writing.
 *  a   Append; open the stream for writing, with the initial buffer position set to the first null byte.
 *  r+  Open the stream for reading and writing.
 *  w+  Open the stream for reading and writing.  The buffer contents are truncated (i.e., '\0' is placed in the first byte of the buffer).
 *  a+  Append; open the stream for reading and writing, with the initial buffer position set to the first null byte.
 *
 *  Ownership and size follow libc fmemopen():
 *   - buf != NULL: the caller provides at least `size` bytes and retains
 *     ownership; dispose of it after the stream is closed.
 *   - buf == NULL: mulle allocates `size` bytes; ownership transfers to the
 *     buffer and it is freed automatically when the stream is closed.
 *  In both cases the stream is FIXED at `size` bytes (it never grows), exactly
 *  like fmemopen(). The returned handle is heap-allocated, so it must be
 *  released with `mulle_buffer_fclose` (which frees the buffer itself).
 *
 *  For a GROWABLE in-memory stream, use a plain `mulle_buffer` instead of
 *  fmemopen:
 *   - `mulle_buffer_create()`      -> heap buffer, grows; release with
 *                                    `mulle_buffer_fclose` (it frees the struct).
 *   - `struct mulle_buffer b; mulle_buffer_init( &b, 0, NULL)  -> embedded/stack
 *                                    buffer, grows; manage with `mulle_buffer_done`.
 *                                    Do NOT pass it to `mulle_buffer_fclose`,
 *                                    which would free() a stack address.
 *  Both interoperate with the `mulle_buffer_functions` vtable for
 *  read/write/seek.
 *
 *  Unfortunately in a cross-platform scenario (at least darwin and linux,
 *  the fmemopen interface is super flakey and unpredictable when it comes
 *  to seeking and writing in various modes).
 */
MULLE__FPRINTF_GLOBAL
void   *mulle_buffer_fmemopen( void *buf, size_t size, const char *mode);

//
// Closes an in-memory stream and frees the buffer struct itself
// (`mulle_buffer_destroy`). The handle must have been heap-allocated
// (e.g. via mulle_buffer_fmemopen or mulle_buffer_create); do NOT pass an
// embedded/stack `mulle_buffer`, that would free() a stack address.
//
MULLE__FPRINTF_GLOBAL
int    mulle_buffer_fclose( void *buffer);

// mulle_buffer_fgetc treats NULL more gracefully than fgetc, returns EOF
MULLE__FPRINTF_GLOBAL
int    mulle_buffer_fgetc( void *buffer);

/**
 * This function is a fake for use in code that accepts fread as callback
 * pointers. This is slightly obscure.
 *
 * @param dst The memory area to read to
 * @param size Element size
 * @param seek Number of elements to read
 * @param buffer The buffer to read from.
 * @returns The number of elements that were read (not bytes necessariy)
 */

MULLE__FPRINTF_GLOBAL
size_t  mulle_buffer_fread( void *dst, size_t size, size_t nmemb, void *buffer);

MULLE__FPRINTF_GLOBAL
int     mulle_buffer_fputc( int c, void *buffer);

MULLE__FPRINTF_GLOBAL
int     mulle_buffer_fputs( const char *s, void *buffer);

/**
 * This function is a fake for use in code that accepts fseek as callback
 * pointers. This is slightly obscure.
 *
 * @param buffer The buffer to set the seek position for.
 * @param seek The seek position, relative to the mode.
 * @param mode The seek mode, one of SEEK_SET, SEEK_CUR, or SEEK_END.
 * @return The new seek position, or 0 if the buffer is invalid.
 */
MULLE__FPRINTF_GLOBAL
int     mulle_buffer_fseek( void *buffer, long seek, int mode);

// Named mulle_buffer_stdio_lseek (not mulle_buffer_lseek) to avoid conflict
// with the type-safe static inline mulle_buffer_lseek in mulle-buffer.h.
// This is the void* adapter for the mulle_buffer_stdio_functions table.
MULLE__FPRINTF_GLOBAL
off_t   mulle_buffer_stdio_lseek( void *buffer, off_t seek, int mode);

MULLE__FPRINTF_GLOBAL
long    mulle_buffer_ftell( void *buffer);

MULLE__FPRINTF_GLOBAL
size_t  mulle_buffer_fwrite( void *src, size_t size, size_t nmemb, void *buffer);

MULLE__FPRINTF_GLOBAL
int     mulle_buffer_fflush( void *buffer);


//
// problems: you really want `off_t` instead of `long` for seeking
//           so this was added.
//           what about fmemopen in here ? is this useful ?
struct mulle_buffer_stdio_functions
{
   void    *(*fmemopen)( void *buf, size_t size, const char *mode);
   int      (*fclose)( void *buffer);
   int      (*fgetc)( void *buffer);
   size_t   (*fread)( void *dst, size_t size, size_t nmemb, void *buffer);
   int      (*fputc)( int c, void *buffer);
   int      (*fputs)( const char *s, void *buffer);
   int      (*fseek)( void *buffer, long seek, int mode);
   long     (*ftell)( void *buffer);
   size_t   (*fwrite)( void *src, size_t size, size_t nmemb, void *buffer);
   int      (*fflush)( void *buffer);

   off_t    (*lseek)( void *buffer, off_t seek, int mode);
};


//
// convention: pass NULL to a (struct mulle_buffer_stdio_functions *)
//             parameter and they will use mulle_buffer_functions
//
MULLE__FPRINTF_GLOBAL
struct mulle_buffer_stdio_functions   mulle_buffer_functions;

MULLE__FPRINTF_GLOBAL
struct mulle_buffer_stdio_functions   mulle_stdio_functions;


// TODO:
//
// How about ?
//
// struct mulle_FILE
// {
//   void                          *buffer;
//   mulle_buffer_stdio_functions  *functions;
// }
//
// static int mulle_FILE_getc( struct mulle_FILE *fp)
// {
//    return( (*fp->functions)( fgetc, fp->buffer)
// }
//
// But what about mulle_buffer_fprintf and mulle_fprintf ?



// convenience:
// static char   *read_text( char *filepath)
// {
//    char   *s;
//
//    mulle_buffer_do_filepath( buffer, filepath, MULLE_BUFFER_IS_TEXT)
//    {
//       s = mulle_buffer_extract_string( buffer);
//    }
//    return( s);
// }


// TODO: this has nothing to do with mulle_buffer_stdio_functions
//       move to own file ?

#pragma mark - read from FILE into buffer

/**
 * Initializes a buffer with the contents of a file. This uses FILE for I/O.
 * Memory mapping might be available through mulle-mmap.
 * This does not write!
 *
 * @param buffer The buffer to initialize.
 * @param filepath Name of the file to read from
 * @param binary use MULLE_BUFFER_IS_BINARY for "rb" MULLE_BUFFER_IS_TEXT for "r"
 * @param allocator The allocator to use for the buffer's storage.
 */
MULLE__FPRINTF_GLOBAL
int   mulle_buffer_init_with_filepath( struct mulle_buffer *buffer,
                                       char *filepath,
                                       int mode,
                                       struct mulle_allocator *allocator);




// read specified amount of bytes from FILE
// Returns the number of bytes read, or 0 with errno set on error
// (fp == NULL yields EINVAL). The buffer must be initialized.
MULLE__FPRINTF_GLOBAL
size_t   mulle_buffer_fread_FILE( struct mulle_buffer *buffer,
                                  size_t size,
                                  size_t nmem,
                                  FILE *fp);

// read remaining bytes from FILE
// Returns the number of bytes read (0 for an empty file), with errno == 0 on
// success. On failure returns 0 with errno set. Non-seekable streams (pipes,
// sockets) are not supported and report an error via errno.
MULLE__FPRINTF_GLOBAL
size_t   mulle_buffer_fread_FILE_all( struct mulle_buffer *buffer,
                                      FILE *fp);


#define mulle_buffer_do_filepath( name, filepath, mode)                       \
   for( struct mulle_buffer                                                   \
          name ## __storage,                                                  \
          *name = &name ## __storage,                                         \
          *name ## __i = (mulle_buffer_init_with_filepath( name,              \
                                                           filepath,          \
                                                           mode,              \
                                                           NULL), NULL);      \
        ! name ## __i;                                                        \
        name ## __i = ( mulle_buffer_done( &name ## __storage), (void *) 0x1) \
      )                                                                       \
                                                                              \
      MULLE_C_CONFINED_LOOP                                                   \
      for( int  name ## __j = 0;    /* break protection */                    \
           name ## __j < 1;                                                   \
           name ## __j++)


//
// FLUSHABLE TO FILE *
//
// The flushable buffer core lives in mulle-buffer. These convenience macros
// bind it to a FILE * via fwrite, so a mulle-buffer user does not need
// stdio.h for the flushable layer alone.
//

/**
 * Defines a macro that creates a `mulle_flushablebuffer` instance and a loop to use it.
 *
 * This macro creates a static `mulle_flushablebuffer` instance with a 128-byte internal
 * buffer, and a `fwrite` flusher function that writes to the provided `FILE*`. It then
 * defines a loop that uses this buffer, flushing it when the loop completes.
 *
 * The final flush result is intentionally discarded. If you need to detect a
 * failed flush (see `mulle_flushablebuffer_done`), initialize the
 * flushablebuffer yourself and call `mulle_flushablebuffer_done` manually,
 * rather than using this convenience macro.
 *
 * @param name The name to use for the `mulle_flushablebuffer` instance and loop variables.
 * @param fp The `FILE*` to write the buffer contents to.
 */
//
// Flush to FILE *
//
#define _mulle_flushablebuffer_chars_to_struct( len) \
   ((len + sizeof( struct mulle_flushablebuffer) - 1) / sizeof( struct mulle_flushablebuffer))

static inline size_t   _mulle_flushablebuffer_fwrite( void *buf,
                                                      size_t one,
                                                      size_t len,
                                                      void *userinfo)
{
   return( fwrite( buf, one, len, (FILE *) userinfo));
}

#define mulle_flushablebuffer_do_FILE( name, fp)                                                     \
   for( struct mulle_flushablebuffer                                                                 \
            name ## __alloca[ _mulle_flushablebuffer_chars_to_struct( MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY)], \
            name ## __storage = _mulle_flushablebuffer_static_data( name ## __alloca,                 \
                                                                    sizeof( name ## __alloca),        \
                                                                    _mulle_flushablebuffer_fwrite,    \
                                                                    (fp)),                            \
            *name ## __i = NULL;                                                                       \
         ! name ## __i;                                                                               \
         name ## __i = ( (void) mulle_flushablebuffer_done( &name ## __storage), (void *) 0x1)        \
       )                                                                                              \
                                                                                                     \
       MULLE_C_CONFINED_LOOP                                                                           \
       for( struct mulle_buffer                                                                        \
             *name = (struct mulle_buffer *) &name ## __storage,                                       \
             *name ## __j = 0;    /* break protection */                                               \
             name ## __j < (struct mulle_buffer *) 1;                                                  \
             name ## __j++)

#endif
