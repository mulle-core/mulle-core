#include "mulle-buffer-stdio.h"

#include <errno.h>
#include <stdio.h>


//
//  r   : USEFUL:  readonly.
//  w   : USEFUL:  writeonly
//  a   :          writeonly, seek to first null byte
//  r+  : USEFUL:  read and write
//  w+  : read and write,  The buffer contents are truncated (i.e., '\0' is placed in the first byte of the buffer).
//  a+  : read and write, seek to first null byte
//

// darwin:
//   The fmemopen() function associates the buffer given by the buf and size
//   arguments with a stream.  The buf argument is either a null pointer or a
//   pointer to a buffer that is at least size bytes long.  If a null pointer
//   is specified as the buf argument, fmemopen() allocates size bytes of
//   memory, and this allocation is automatically freed when the stream is
//   closed.  If a non-null pointer is specified, the caller retains ownership
//   of the buffer and is responsible for disposing of it after the stream has
//   been closed.  Buffers can be opened in text-mode (default) or binary-mode
//   (if “b” is present in the second or third position of the mode argument).
//   Buffers opened in text-mode make sure that writes are terminated with a
//   NULL byte, if the last write hasn't filled up the whole buffer. Buffers
//   opened in binary-mode never append a NULL byte.

void   *mulle_buffer_fmemopen( void *bytes, size_t length, const char *mode)
{
   struct mulle_buffer  *buffer;
   int                  append;
   int                  truncate;
   unsigned int         bits;

   if( ! mode)
   {
      errno = EINVAL;
      return( NULL);
   }

// darwin is just stupid broken, but it makes no sense to support it
// for test conformance, since its a bug
// #ifdef __APPLE__
//    if( ! bytes || ! length)
//    {
//       errno = EINVAL;
//       return( NULL);
//    }
// #endif

   bits     = MULLE_BUFFER_IS_TEXT;
   append   = 0;
   truncate = 0;

   switch( *mode++)
   {
   case 'r' : bits   |= MULLE_BUFFER_IS_READONLY;
              break;
   case 'a' : append  = 1; // fall thru
   case 'w' : bits   |= MULLE_BUFFER_IS_WRITEONLY;
              break;
   default  : goto error_einval;
   }

   for(;;)
   {
      switch( *mode++)
      {
      case 'b'  : bits    &= ~MULLE_BUFFER_IS_TEXT;
                  continue;
      case '+'  : bits    &= ~MULLE_BUFFER_IS_READONLY|MULLE_BUFFER_IS_WRITEONLY;
                  truncate = 1;
                  continue;
      case '\0' : goto done_parse;
      default   : goto error_einval;
      }
   }

done_parse:
   buffer = mulle_buffer_alloc( NULL);

   // The fmemopen() function associates the buffer given by the buf and size
   // arguments with a stream.  The buf argument is either a null pointer or a
   // pointer to a buffer that is at least size bytes long.
   // If a null pointer is specified as the buf argument, fmemopen() allocates
   // size bytes of memory, and this allocation is automatically freed when
   // the stream is closed. If a non-null pointer is specified, the caller
   // retains ownership of the buffer and is responsible for disposing of it
   // after the stream has been closed.
   //
   if( bytes)
      mulle_buffer_init_with_static_bytes( buffer,
                                           bytes,
                                           length,
                                           NULL);
   else
      mulle_buffer_init( buffer, length, NULL);

   buffer->_type |= bits;
   buffer->_type |= MULLE_BUFFER_IS_INFLEXIBLE;

   //
   // fmemopen has retarded semantics IMO
   //
   // In append mode, if no null byte is found within the buffer, then the
   // initial position is length+1. (I don't think we can do this, we can just
   // do length)
   //
   if( append)
   {
      while( buffer->_curr < buffer->_sentinel)
      {
         if( *buffer->_curr == '\0')
            break;
         ++buffer->_curr;
      }
   }
   else
      if( truncate && (bits != MULLE_BUFFER_IS_READONLY))
      {
         if( ! mulle_buffer_is_full( buffer))
            *buffer->_curr = '\0';
      }

   return( buffer);

error_einval:
   errno = EINVAL;
   return( NULL);
}



//
// STDIO callback mimicry
//

size_t    mulle_buffer_fread( void *dst, size_t size, size_t nmemb, void *buffer)
{
   size_t   bytes_to_read;
   size_t   bytes_available;
   size_t   bytes_to_copy;
   size_t   count;

   if( ! buffer)
      goto zero;

   if( mulle_buffer_is_writeonly( buffer))
   {
      errno = EBADF;
      return( 0);
   }

   // on older darwin, if we read 4 with "xx\0x" we get 2 bytes back. but this seems
   // to have been fixes.
   // on linux we get 4 bytes back (not sure what subsequent bytes are)
   // we do the linux thing 

   bytes_to_read   = size * nmemb;
   bytes_available = mulle_buffer_remaining_length( buffer);
   bytes_to_copy   = (bytes_to_read > bytes_available) ? bytes_available : bytes_to_read;

   _mulle__buffer_next_bytes( (struct mulle__buffer *) buffer,
                               dst,
                               bytes_to_copy);

   if( ! size)
      goto zero;

   count = bytes_to_copy / size;
   if( ! count)
      goto zero;
   return( count);

zero:
   errno = 0;     // probably "too" nice of us
   return( 0);
}



long    mulle_buffer_ftell( void *buffer)
{
   if( ! buffer)
      return( 0);

   return( _mulle__buffer_get_seek( (struct mulle__buffer *) buffer));
}


int    mulle_buffer_fseek( void *buffer, long seek, int mode)
{
   int   rval;

   if( ! buffer)
   {
      errno = EBADF;
      return( -1);
   }

   // compatibility ...
   if( mode == SEEK_END && (((struct mulle__buffer *) buffer)->_type & MULLE_BUFFER_IS_WRITEONLY))
   {
      errno = ENOSPC;
      return( -1);
   }

   rval = _mulle__buffer_set_seek( (struct mulle__buffer *) buffer, seek, mode);
   if( rval)
      errno = EINVAL;
   return( rval);
}


// not setting errno to zero is
int   mulle_buffer_fgetc( void *buffer)
{
   int   c;

   if( ! buffer)
      goto eof;

   if( mulle_buffer_is_writeonly( buffer))
   {
      errno = EBADF;
      return( EOF);
   }

   c = mulle_buffer_next_byte( buffer);
   if( c != -1)
      return( c);

eof:
   errno = 0;
   return( EOF);
}


int   mulle_buffer_fputc( int c, void *buffer)
{
   if( ! buffer)
      return( EOF);

   if( mulle_buffer_is_readonly( buffer))
   {
      errno = EBADF;
      return( EOF);
   }

   _mulle__buffer_add_byte( (struct mulle__buffer *) buffer,
                            (unsigned char) c,
                            ((struct mulle_buffer *) buffer)->_allocator);

#ifdef __GLIBC__
   // stupid code for fmemopen compatibility (as observed on linux)
   if( mulle_buffer_is_inflexible( buffer))
      return( (unsigned char) c);
#endif

   return( _mulle__buffer_has_overflown( buffer) ? EOF : (unsigned char) c);
}


int   mulle_buffer_fputs( const char *s, void *buffer)
{
   size_t   before;

   if( ! buffer)
      return( 0);

   if( mulle_buffer_is_readonly( buffer))
   {
      errno = EBADF;
      return( EOF);
   }

   before = mulle_buffer_get_length( buffer);
   mulle_buffer_add_c_string( buffer, (char *) s);

#ifdef __GLIBC__
   // stupid code for fmemopen compatibility (as observed on linux)
   if( mulle_buffer_is_inflexible( buffer))
      return( (int) (mulle_buffer_get_length( buffer) - before));
#endif

   return( _mulle__buffer_has_overflown( buffer)
           ? EOF
           : (int) (mulle_buffer_get_length( buffer) - before));
}


int   mulle_buffer_fflush( void *buffer)
{
   if( ! buffer)
   {
      errno = EBADF;
      return( EOF);
   }

   // When  a  stream that has been opened for writing is flushed
   // a null byte is written at the end of
   // the buffer if there is space.  The caller should ensure that an extra
   // byte is available in the buffer (and that size counts that byte) to allow
   // for this.
   if( ((struct mulle__buffer *) buffer)->_type & MULLE_BUFFER_IS_TEXT)
      _mulle__buffer_zero_last_byte_no_truncate( (struct mulle__buffer *) buffer);

   if( _mulle__buffer_is_flushable( (struct mulle__buffer *) buffer))
      _mulle__buffer_flush( (struct mulle__buffer *) buffer);
   return( 0);
}



size_t    mulle_buffer_fwrite( void *src, size_t size, size_t nmemb, void *buffer)
{
   size_t   bytes_to_write;
   void     *dst;

   if( ! buffer)
      return( EOF); // seems the compatible way (no errno) dont ask

   if( mulle_buffer_is_readonly( buffer))
   {
      errno = EBADF;
      return( EOF);
   }

   if( ! size) // catches division by zero
      goto zero;

   bytes_to_write = size * nmemb;
   dst            = mulle_buffer_advance( buffer, bytes_to_write);

   // ain't doing partial writes, except if inflexible, then it gets weird
   // for compatibility

   if( ! dst)
   {
#ifdef __GLIBC__
      if( mulle_buffer_is_inflexible( buffer))
      {
         size_t   remaining;

         remaining = mulle_buffer_remaining_length( buffer);
         mulle_buffer_add_bytes( buffer, src, remaining);
         return( nmemb); // (sic!) turbo retarded
      }
#endif
      errno = ENOSPC;
      return( 0);
   }

   memmove( dst, src, bytes_to_write);
   return( nmemb);

zero:
   errno = 0;     // probably "too" nice of us
   return( 0);
}



int   mulle_buffer_fclose( void *buffer)
{
   mulle_buffer_fflush( buffer);
   mulle_buffer_destroy( buffer);
   return( 0);
}


off_t   mulle_buffer_lseek( void *buffer, off_t offset, int mode)
{
   int    rval;
   long   seek;

   if( ! buffer)
   {
      errno = EBADF;
      return( -1);
   }

   seek = (long) offset;
   if( (off_t) seek != offset)
   {
      errno = EINVAL;
      return( -1);
   }

   rval = _mulle__buffer_set_seek( (struct mulle__buffer *) buffer, seek, mode);
   if( rval)
   {
      errno = EINVAL;
      return( -1);
   }
   return( (off_t) mulle_buffer_ftell( (struct mulle__buffer *) buffer));
}


// implement lseek on fseek
static off_t   mulle_FILE_lseek( void *buffer, off_t offset, int mode)
{
   FILE   *fp = buffer;
   int    rval;
   long   seek;

   if( ! fp)
   {
      errno = EBADF;
      return( -1);
   }

   seek = (long) offset;
   if( (off_t) seek != offset)
   {
      errno = EINVAL;
      return( -1);
   }

   rval = fseek( fp, seek, mode);
   if( rval)
      return( -1);

   return( (off_t) ftell( fp));
}

// what can I do ? Seem this is only used with Windows gcc matrix ?
#if defined( _WIN32)
static void   *fmemopen( void *string, size_t size, const char *mode)
{
   fprintf( stderr, "Sorry but there is no fmemopen on windows. Use mulle_buffer_fmemopen instead\n");
   abort();

   MULLE_C_UNUSED( string);
   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( mode);
}
#endif


struct mulle_buffer_stdio_functions   mulle_stdio_functions =
{
   .fread    = (size_t (*)(void *, size_t, size_t, void *)) fread,
   .fwrite   = (size_t (*)(void *, size_t, size_t, void *)) fwrite,

   .fmemopen = (void * (*)(void *, size_t, const char *)) fmemopen,
   .fclose   = (int (*)(void *)) fclose,

   .fflush   = (int (*)(void *)) fflush,
   .fgetc    = (int (*)(void *)) fgetc,
   .fputc    = (int (*)(int,  void *)) fputc,
   .fputs    = (int (*)(const char *, void *)) fputs,
   .fseek    = (int (*)(void *, long int,  int)) fseek,
   .ftell    = (long int (*)(void *)) ftell,

   .lseek    = mulle_FILE_lseek,
};


struct mulle_buffer_stdio_functions   mulle_buffer_functions =
{
   .fread    = mulle_buffer_fread,
   .fwrite   = mulle_buffer_fwrite,

   .fmemopen = mulle_buffer_fmemopen,
   .fclose   = mulle_buffer_fclose,

   .fflush   = mulle_buffer_fflush,
   .fgetc    = mulle_buffer_fgetc,
   .fputc    = mulle_buffer_fputc,
   .fputs    = mulle_buffer_fputs,
   .fseek    = mulle_buffer_fseek,
   .ftell    = mulle_buffer_ftell,

   .lseek    = mulle_buffer_lseek
};




//
// FILE I/O
//
// this has nothing to do with above really
//

int   mulle_buffer_init_with_filepath( struct mulle_buffer *buffer,
                                       char *filepath,
                                       int mode,
                                       struct mulle_allocator *allocator)
{
   FILE     *fp;
   size_t   rval;

   // lets do this first, if the fopen later fails its not a problem
   // but buffer will be consistenly inited
   mulle_buffer_init( buffer, 0, allocator);
   if( ! filepath)
      return( (errno = EINVAL));

   fp = fopen( filepath, mode == MULLE_BUFFER_IS_BINARY ? "rb" : "r");
   if( ! fp)
      return( errno);

   rval = mulle_buffer_fread_FILE_all( buffer, fp);
   fclose( fp);

   return( (rval == 0 && errno != EFBIG) ? errno : 0);
}


size_t   mulle_buffer_fread_FILE( struct mulle_buffer *buffer,
                                  size_t size,
                                  size_t nmem,
                                  FILE *fp)
{
   void     *space;
   size_t   rval;

   if( ! buffer)
   {
      errno = EINVAL;
      return( 0);
   }

   if( ! size || ! nmem)
   {
      errno = EFBIG; // assume overflow ;)
      return( 0);
   }

   space = mulle_buffer_guarantee( buffer, nmem * size);
   rval  = fread( space, 1, nmem * size, fp);
   mulle_buffer_advance( buffer, rval);
   return( rval);
}


size_t   mulle_buffer_fread_FILE_all( struct mulle_buffer *buffer,
                                      FILE *fp)
{
   long   current_pos;
   long   file_size;

   current_pos = ftell( fp);
   if( current_pos == -1)
      return( 0);

   if( fseek( fp, 0, SEEK_END) == -1)
      return( 0);
   file_size = ftell( fp);
   if( file_size == -1)
      return( 0);

   // LONG_MAX actually observed value, when opening a directory
   // Starting tests...
   // current_pos=0
   // file_size=9223372036854775807
   // size=9223372036854775807 nmem=1
   // memory allocation:: Cannot allocate memory
   if( file_size == LONG_MAX)
   {
      errno = EISDIR;   // only observed then
      return( 0);
   }

   if( fseek( fp, current_pos, SEEK_SET) == -1)
      return( 0);

   return( mulle_buffer_fread_FILE( buffer, 1, file_size, fp));
}

