#ifndef mulle_http_include_h__
#define mulle_http_include_h__

/* This is a central include file to keep dependencies out of the library
   C files. It is usally included by .h files only.

   The advantage is that now .c and .h files become motile. They can
   be moved to other projects and don't need to be edited. Also less typing...

   Therefore it is important that this file is called "include.h" and
   not "mulle-http-include.h" to keep the #include statements in the
   library code uniform.

   The C-compiler will pick up the nearest one.
*/

/* Include the header file automatically generated by c-sourcetree-update.
   Here the prefix is harmless and serves disambiguation. If you have no
   sourcetree, then you don't need it.
 */

#include "_mulle-http-include.h"

#ifdef MULLE_HTTP_BUILD
# define MULLE_HTTP_GLOBAL    MULLE_C_GLOBAL
#else
# if defined( MULLE_HTTP_INCLUDE_DYNAMIC) || (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE_HTTP_INCLUDE_STATIC))
#  define MULLE_HTTP_GLOBAL   MULLE_C_EXTERN_GLOBAL
# else
#  define MULLE_HTTP_GLOBAL   extern
# endif
#endif


/* You can add some more include statements here */

#endif