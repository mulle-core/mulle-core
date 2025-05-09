/*
 *   This file will be regenerated by `mulle-project-versioncheck`.
 *   Any edits will be lost.
 */
#ifndef mulle_regex_versioncheck_h__
#define mulle_regex_versioncheck_h__

#if defined( MULLE__UTF_VERSION)
# ifndef MULLE__UTF_VERSION_MIN
#  define MULLE__UTF_VERSION_MIN  ((5UL << 20) | (0 << 8) | 1)
# endif
# ifndef MULLE__UTF_VERSION_MAX
#  define MULLE__UTF_VERSION_MAX  ((6UL << 20) | (0 << 8) | 0)
# endif
# if MULLE__UTF_VERSION < MULLE__UTF_VERSION_MIN || MULLE__UTF_VERSION >= MULLE__UTF_VERSION_MAX
#  pragma message("MULLE__UTF_VERSION     is " MULLE_C_STRINGIFY_MACRO( MULLE__UTF_VERSION))
#  pragma message("MULLE__UTF_VERSION_MIN is " MULLE_C_STRINGIFY_MACRO( MULLE__UTF_VERSION_MIN))
#  pragma message("MULLE__UTF_VERSION_MAX is " MULLE_C_STRINGIFY_MACRO( MULLE__UTF_VERSION_MAX))
#  if MULLE__UTF_VERSION < MULLE__UTF_VERSION_MIN
#   error "mulle-utf is too old"
#  else
#   error "mulle-utf is too new"
#  endif
# endif
#endif

#endif
