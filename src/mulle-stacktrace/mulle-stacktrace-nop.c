#if (defined( _WIN32) || defined( __MULLE_MUSL__) || defined( __COSMOPOLITAN__) || defined( __MULLE_COSMOPOLITAN__) || defined(__emscripten__))

#include "mulle-stacktrace.h"



void   _mulle_stacktrace_init( struct mulle_stacktrace *stacktrace,
                               mulle_stacktrace_symbolizer_t *symbolize,
                               char *(*trim_belly_fat)( char *),
                               int (*trim_arse_fat)( char *),
                               int (*is_boring)( char *, int size))
{
	// does nothing
}

// stacktrace may be NULL
void  _mulle_stacktrace( struct mulle_stacktrace *stacktrace,
                         int offset,
                         enum mulle_stacktrace_format format,
                         FILE *fp)
{
	// does absolutely nothing
}


void   _mulle_stacktrace_init_default( struct mulle_stacktrace *stacktrace)
{
  // does absolutely nothing
}


int   mulle_stacktrace_count_frames( void)
{
   return( 0);
}

#endif
