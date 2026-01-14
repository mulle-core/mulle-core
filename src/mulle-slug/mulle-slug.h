#ifndef mulle_slug_h__
#define mulle_slug_h__

#include "include.h"

#include <stdint.h>

/*
 *  (c) 2023 nat <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE__SLUG_VERSION  ((0UL << 20) | (0 << 8) | 5)


static inline uint32_t   mulle_slug_get_version_major( void)
{
   return( MULLE__SLUG_VERSION >> 20);
}


static inline uint32_t   mulle_slug_get_version_minor( void)
{
   return( (MULLE__SLUG_VERSION >> 8) & 0xFFF);
}


static inline uint32_t   mulle_slug_get_version_patch( void)
{
   return( MULLE__SLUG_VERSION & 0xFF);
}

// mulle-c11 feature: MULLE_SLUG__GLOBAL
uint32_t   mulle_slug_get_version( void);


//
// you get an allocated string back, that you need to mulle_free
//
extern char   *mulle_utf8_slugify( char *s);

//
// you get a struct mulle_utf8data, whose characters you need to mulle_free
//
extern struct mulle_utf8data   mulle_utf8data_slugify( struct mulle_utf8data  data,
                                                       struct mulle_allocator *allocator);


// adds slugified string to buffer, useful for building up an html page
void  mulle_buffer_add_slugified_utf8data( struct mulle_buffer *buffer,
                                           struct mulle_utf8data data);
//
// You can slugify into an existing buffer. You can use an alloca buffer
// here and then get a) a max sized string and b) no mallocs
//
extern void  mulle_buffer_slugify_utf8data( struct mulle_buffer *buffer,
                                            struct mulle_utf8data data);
/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-slug-versioncheck.h")
#  include "_mulle-slug-versioncheck.h"
# endif
#endif

#endif
