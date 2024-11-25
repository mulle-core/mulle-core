#ifndef mulle_buffer_stdio_h__
#define mulle_buffer_stdio_h__

#include "include.h"


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
 * Unfortunately in a cross-platform scenario (at least darwin and linux,
 * the fmemopen interface is super flakey and unpredictable when it comes
 * to seeking and writing in various modes).
 */
MULLE__FPRINTF_GLOBAL
void   *mulle_buffer_fmemopen( void *buf, size_t size, const char *mode);

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
size_t   mulle_buffer_fread( void *dst, size_t size, size_t nmemb, void *buffer);

MULLE__FPRINTF_GLOBAL
int      mulle_buffer_fputc( int c, void *buffer);

MULLE__FPRINTF_GLOBAL
int      mulle_buffer_fputs( const char *s, void *buffer);

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
int      mulle_buffer_fseek( void *buffer, long seek, int mode);

MULLE__FPRINTF_GLOBAL
off_t    mulle_buffer_lseek( void *buffer, off_t seek, int mode);

MULLE__FPRINTF_GLOBAL
long     mulle_buffer_ftell( void *buffer);

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

#endif
