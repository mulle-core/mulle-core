/*
 * template: demo/library/PROJECT_SOURCE_DIR/PROJECT_NAME.h
 * vendor/extension: mulle-sde/c-demo
 */
#ifndef mulle_fifo_h__
#define mulle_fifo_h__

#include "include.h"

#include <stdint.h>

/*
 *  (c) 2021 mulle-kybernetik-tv <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE_FIFO_VERSION  ((0 << 20) | (1 << 8) | 1)


static inline unsigned int   mulle_fifo_get_version_major( void)
{
   return( MULLE_FIFO_VERSION >> 20);
}


static inline unsigned int   mulle_fifo_get_version_minor( void)
{
   return( (MULLE_FIFO_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_fifo_get_version_patch( void)
{
   return( MULLE_FIFO_VERSION & 0xFF);
}


MULLE_FIFO_GLOBAL
uint32_t   mulle_fifo_get_version( void);


/*
 * The following files are auto-generated.
 * The _mulle-fifo-provide header is generated by
 * mulle-match-to-c during a reflect.
 */
#include "_mulle-fifo-provide.h"



/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-fifo-versioncheck.h")
#  include "_mulle-fifo-versioncheck.h"
# endif
#endif

#endif