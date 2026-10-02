#ifndef MULLE_FMEMOPEN_COMPAT_H
#define MULLE_FMEMOPEN_COMPAT_H

#include <stdio.h>

// Windows doesn't have fmemopen
#ifdef _WIN32
#define HAVE_FMEMOPEN 0
static inline FILE *fmemopen(void *buf, size_t size, const char *mode)
{
   return NULL;
}
#else
#define HAVE_FMEMOPEN 1
#endif

#endif
