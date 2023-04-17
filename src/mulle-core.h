#ifndef mulle_core_h__
#define mulle_core_h__

#include "include.h"

#include <stdint.h>

/*
 *  (c) 2023 nat <|ORGANIZATION|>
 *
 *  version:  major, minor, patch
 */
#define MULLE_CORE_VERSION  ((0 << 20) | (1 << 8) | 0)


static inline unsigned int   mulle_core_get_version_major( void)
{
   return( MULLE_CORE_VERSION >> 20);
}


static inline unsigned int   mulle_core_get_version_minor( void)
{
   return( (MULLE_CORE_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_core_get_version_patch( void)
{
   return( MULLE_CORE_VERSION & 0xFF);
}

// mulle-c11 feature: MULLE_CORE__GLOBAL
uint32_t   mulle_core_get_version( void);


#include <mulle-c11/mulle-c11.h>
#include <mulle-allocator/mulle-allocator.h>
#include <mulle-data/mulle-data.h>
#include <mulle-buffer/mulle-buffer.h>
#include <mulle-container/mulle-container.h>
#include <mulle-http/mulle-http.h>
#include <mulle-slug/mulle-slug.h>
#include <mulle-unicode/mulle-unicode.h>
#include <mulle-url/mulle-url.h>
#include <mulle-utf/mulle-utf.h>
#include <mulle-vararg/mulle-vararg.h>

#include <mulle-thread/mulle-thread.h>
#include <mulle-concurrent/mulle-concurrent.h>
#include <mulle-aba/mulle-aba.h>
#include <mulle-fifo/mulle-fifo.h>
#include <mulle-multififo/mulle-multififo.h>

#include <mulle-dlfcn/mulle-dlfcn.h>
#include <mulle-mmap/mulle-mmap.h>
#include <mulle-stacktrace/mulle-stacktrace.h>
#include <mulle-fprintf/mulle-fprintf.h>
#include <mulle-sprintf/mulle-sprintf.h>
#include <mulle-time/mulle-time.h>


/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-core-versioncheck.h")
#  include "_mulle-core-versioncheck.h"
# endif
#endif

#endif
