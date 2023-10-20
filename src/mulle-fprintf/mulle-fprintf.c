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
                                                 (mulle_flushablebuffer_flusher_t) fwrite,
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
                               (mulle_flushablebuffer_flusher_t) fwrite,
                               fp);

   buffer = mulle_flushablebuffer_as_buffer( &flushable_buffer);
   rval   = mulle_buffer_mvsprintf( buffer, format, arguments);
   rval2  = mulle_flushablebuffer_done( &flushable_buffer);

   return( rval2 ? rval2 : rval);
}

