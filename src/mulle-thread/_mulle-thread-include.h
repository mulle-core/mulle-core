/*
 *   This file will be regenerated by `mulle-sourcetree-to-c` via
 *   `mulle-sde reflect` and any edits will be lost.
 *   Suppress generation of this file with:
 *
 *      mulle-sde environment set MULLE_SOURCETREE_TO_C_INCLUDE_FILE DISABLE
 *
 *   To not let mulle-sourcetree-to-c generate any header files:
 *
 *      mulle-sde environment set MULLE_SOURCETREE_TO_C_RUN DISABLE
 *
 */

#ifndef _mulle_thread_include_h__
#define _mulle_thread_include_h__

// To remove the following dependency (headers and library) completely:
//   `mulle-sde dependency remove mulle-c11`
// (Use 97F390B6-FA26-4BAC-BB87-90A654AAB3B6 instead of mulle-c11, if there are duplicate entries)
//
// You can tweak the following #include with these commands:
//    remove #include: `mulle-sde dependency mark mulle-c11 no-header`
//    rename              : `mulle-sde dependency|library set mulle-c11 include whatever.h`
//    reorder             : `mulle-sde dependency move mulle-c11 <up|down>`
//    toggle #include: `mulle-sde dependency mark mulle-c11 [no-]import`
//    toggle public       : `mulle-sde dependency mark mulle-c11 [no-]public`
//    toggle optional     : `mulle-sde dependency mark mulle-c11 [no-]require`
//    remove for platform : `mulle-sde dependency mark mulle-c11 no-platform-<uname>`
//        (use `mulle-sourcetree-to-c --unames` to list known values)
#include <mulle-c11/mulle-c11.h>   // mulle-c11

// To remove the following dependency (headers and library) completely:
//   `mulle-sde dependency remove stdthreads`
// (Use 91a3a06c-669f-4cd4-9060-562b3d0b6315 instead of stdthreads, if there are duplicate entries)
//
// You can tweak the following #include with these commands:
//    remove #include: `mulle-sde dependency mark stdthreads no-header`
//    rename              : `mulle-sde dependency|library set stdthreads include whatever.h`
//    reorder             : `mulle-sde dependency move stdthreads <up|down>`
//    toggle #include: `mulle-sde dependency mark stdthreads [no-]import`
//    toggle public       : `mulle-sde dependency mark stdthreads [no-]public`
//    toggle optional     : `mulle-sde dependency mark stdthreads [no-]require`
//    remove for platform : `mulle-sde dependency mark stdthreads no-platform-<uname>`
//        (use `mulle-sourcetree-to-c --unames` to list known values)
# if defined( __FreeBSD__)
# include <stdthreads.h>   // stdthreads
#endif

// To remove the following dependency (headers and library) completely:
//   `mulle-sde dependency remove pthread`
// (Use 8fd8130b-1d75-4633-89b4-57e61467847e instead of pthread, if there are duplicate entries)
//
// You can tweak the following #include with these commands:
//    remove #include: `mulle-sde dependency mark pthread no-header`
//    rename              : `mulle-sde dependency|library set pthread include whatever.h`
//    reorder             : `mulle-sde dependency move pthread <up|down>`
//    toggle #include: `mulle-sde dependency mark pthread [no-]import`
//    toggle public       : `mulle-sde dependency mark pthread [no-]public`
//    toggle optional     : `mulle-sde dependency mark pthread [no-]require`
//    remove for platform : `mulle-sde dependency mark pthread no-platform-<uname>`
//        (use `mulle-sourcetree-to-c --unames` to list known values)
# if ! defined( __ANDROID__) && ! defined( _WIN32)
# ifdef __has_include
#  if __has_include(<pthread.h>)
#    include <pthread.h>   // pthread
#   define HAVE_LIB_PTHREAD
#  endif
# endif
#endif

#endif
