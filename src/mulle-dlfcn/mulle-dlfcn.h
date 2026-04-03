#ifndef mulle_dlfcn_h__
#define mulle_dlfcn_h__

#include <mulle-c11/mulle-c11.h>

#ifdef _WIN32
# ifndef MULLE_BOOL_DEFINED
#  error "you need to include <mulle-c11/mulle-c11-bool.h> before including <windows.h>"
# endif
# include <windows.h>
# include <psapi.h>   // psapi
#endif

#include <dlfcn.h>      // can be dlfcn-win32 or system

#include <stdint.h>

#define MULLE__DLFCN_GLOBAL   extern

/*
 *  (c) 2019 nat ORGANIZATION
 *
 *  version:  major, minor, patch
 */
#define MULLE__DLFCN_VERSION  ((0UL << 20) | (0 << 8) | 16)


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
#   define MULLE_RTLD_DEFAULT    ((void *) -2)
#  else
#   ifdef _WIN32
#    define MULLE_RTLD_DEFAULT    ((void *) 0)
#   else
#    error "need to define MULLE_RTLD_DEFAULT on this platform"
#   endif
#  endif
# endif
#endif


//
// This guarantees to search at least the executable namespace, possibly
// more. This function is needed to avoid the rendezvous "dll" hell and the
// repeated static linking of dl.a in shared libraries, that need to resolve
// symbols like mulle_atinit or mulle_atexit
//
static inline void   *mulle_dlsym_exe( const char *name )
{
#ifdef __WIN32
    FARPROC   symbol;
    HMODULE   hModule;

    hModule = GetModuleHandle( NULL);
    symbol  = GetProcAddress( hModule, name);
    return( *(void **) (&symbol));
#else
    return( dlsym( MULLE_RTLD_DEFAULT, name));
#endif
}

#endif
