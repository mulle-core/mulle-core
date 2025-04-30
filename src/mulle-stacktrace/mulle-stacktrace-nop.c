#if (defined( _WIN32) || defined( __MULLE_MUSL__) || defined( __COSMOPOLITAN__) || defined( __MULLE_COSMOPOLITAN__) || defined(__emscripten__))

#include "mulle-stacktrace.h"



void   _mulle_stacktrace_init( struct mulle_stacktrace *stacktrace,
                               mulle_stacktrace_symbolizer_t *symbolize,
                               char *(*trim_belly_fat)( char *),
                               int (*trim_arse_fat)( char *),
                               int (*is_boring)( char *, int size))
{
	// does nothing
   MULLE_C_UNUSED( stacktrace);
   MULLE_C_UNUSED( symbolize);
   MULLE_C_UNUSED( trim_belly_fat);
   MULLE_C_UNUSED( trim_arse_fat);
   MULLE_C_UNUSED( is_boring);
}

// stacktrace may be NULL
void  _mulle_stacktrace( struct mulle_stacktrace *stacktrace,
                         int offset,
                         enum mulle_stacktrace_format format,
                         FILE *fp)
{
	// does absolutely nothing
   MULLE_C_UNUSED( stacktrace);
   MULLE_C_UNUSED( offset);
   MULLE_C_UNUSED( format);
   MULLE_C_UNUSED( fp);
}


void   _mulle_stacktrace_init_default( struct mulle_stacktrace *stacktrace)
{
  // does absolutely nothing
   MULLE_C_UNUSED( stacktrace);
}


int   mulle_stacktrace_count_frames( void)
{
   return( 0);
}

#endif
