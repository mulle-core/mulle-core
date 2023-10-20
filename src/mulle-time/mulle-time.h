#ifndef mulle_time_h__
#define mulle_time_h__

#define MULLE__TIME_VERSION	((1 << 20) | (1 << 8) | 2)


#include "mulle-timetype.h"
#include "mulle-timespec.h"

#if ! defined( _WIN32)
# include "mulle-timeval.h"
#endif

#include "mulle-absolutetime.h"
#include "mulle-calendartime.h"
#include "mulle-relativetime.h"

#endif
