#ifndef mulle_dlfcn_h__
#define mulle_dlfcn_h__

#include "include.h"

#include <stdint.h>


/*
 *  (c) 2019 nat ORGANIZATION
 *
 *  version:  major, minor, patch
 */
#define MULLE__DLFCN_VERSION  ((0UL << 20) | (0 << 8) | 12)


static inline unsigned int   mulle_dlfcn_get_version_major( void)
{
   return( MULLE__DLFCN_VERSION >> 20);
}


static inline unsigned int   mulle_dlfcn_get_version_minor( void)
{
   return( (MULLE__DLFCN_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_dlfcn_get_version_patch( void)
{
   return( MULLE__DLFCN_VERSION & 0xFF);
}


MULLE__DLFCN_GLOBAL
uint32_t   mulle_dlfcn_get_version( void);



//#ifdef _WIN32 
//# include "mulle-dlfcn-windows.h"
//#endif
//
#ifdef RTLD_DEFAULT
# define MULLE_RTLD_DEFAULT    RTLD_DEFAULT
#else
# ifdef __linux__
#  define MULLE_RTLD_DEFAULT   0
# else
#  ifdef __APPLE__
#   define RTLD_DEFAULT    ((void *) -2)
#  else
#   error "need to define MULLE_RTLD_DEFAULT on this platform"
#  endif
# endif
#endif


#ifdef __has_include
# if __has_include( "_mulle-dlcfn-versioncheck.h")
#  include "_mulle-dlcfn-versioncheck.h"
# endif
#endif

#endif
