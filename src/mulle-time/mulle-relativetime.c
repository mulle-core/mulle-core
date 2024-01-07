#define _GNU_SOURCE

#include "mulle-relativetime.h"

#include <assert.h>
#include "mulle-absolutetime.h"


static mulle_absolutetime_t    load_timestamp;

MULLE_C_CONSTRUCTOR( load)
static void   load( void)
{
   load_timestamp = mulle_absolutetime_now();
}

//
// a) you don't need -all_load for this to work, since when this function
//    is not called (linked), then we don't need the constructor to run
//    either.
//
mulle_relativetime_t   mulle_relativetime_now( void)
{
   assert( load_timestamp != 0.0);
   return( mulle_absolutetime_now() - load_timestamp);
}

