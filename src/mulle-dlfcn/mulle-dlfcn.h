//
//  mulle-dlfcn.h
//  mulle-dlfcn
//
//  Copyright (c) 2019 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
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
#define MULLE__DLFCN_VERSION  ((0UL << 20) | (1 << 8) | 1)


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
