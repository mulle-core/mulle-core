/*
 *   This file will be regenerated by `mulle-project-versioncheck`.
 *   Any edits will be lost.
 */
#ifndef mulle_utf_versioncheck_h__
#define mulle_utf_versioncheck_h__

#if defined( MULLE__ALLOCATOR_VERSION)
# ifndef MULLE__ALLOCATOR_VERSION_MIN
#  define MULLE__ALLOCATOR_VERSION_MIN  ((7UL << 20) | (0 << 8) | 0)
# endif
# ifndef MULLE__ALLOCATOR_VERSION_MAX
#  define MULLE__ALLOCATOR_VERSION_MAX  ((8UL << 20) | (0 << 8) | 0)
# endif
# if MULLE__ALLOCATOR_VERSION < MULLE__ALLOCATOR_VERSION_MIN
#  error "mulle-allocator is too old"
# endif
# if MULLE__ALLOCATOR_VERSION >= MULLE__ALLOCATOR_VERSION_MAX
#  error "mulle-allocator is too new"
# endif
#endif

#if defined( MULLE__DATA_VERSION)
# ifndef MULLE__DATA_VERSION_MIN
#  define MULLE__DATA_VERSION_MIN  ((0UL << 20) | (3 << 8) | 1)
# endif
# ifndef MULLE__DATA_VERSION_MAX
#  define MULLE__DATA_VERSION_MAX  ((0UL << 20) | (4 << 8) | 0)
# endif
# if MULLE__DATA_VERSION < MULLE__DATA_VERSION_MIN
#  error "mulle-data is too old"
# endif
# if MULLE__DATA_VERSION >= MULLE__DATA_VERSION_MAX
#  error "mulle-data is too new"
# endif
#endif

#endif
