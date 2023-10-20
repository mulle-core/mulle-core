#ifndef mulle_range_h__
#define mulle_range_h__

#include "include.h"

#include <stdint.h>
#include <assert.h>



// C11 says intptr_t is optional, but we need it. We use it for finding
// values and returning an error code.
// size_t is the "indexing" value for an array
//
#ifndef INTPTR_MAX
# error "intprt_t and friends are needed for this platform"
#endif

// Convenient to be different to "notakey"
#define mulle_not_found_e     ((uintptr_t) INTPTR_MAX)


//
// The range values are unsigned, but the actual range is limited to
// positive signed values. The range is measured with 'uintptr_t'.
//
//
// a range with zero length can be valid
// the location of a range with zero length is basically undefined, but
// actually has to be within mulle_range_location_max
//
// a range that extends over mulle_range_location_max is invalid
//
struct mulle_range
{
   uintptr_t   location;
   uintptr_t   length;
};


// like uintptr_min
#define mulle_range_location_min       (0)
// like uintptr_max, not like get_max though!!
#define mulle_range_location_max       (mulle_not_found_e-1)
#define mulle_range_zero               ((struct mulle_range) { 0, 0 })

// the struct and the three defines need to stay compatible with MulleObjC/mulle-objc-type.h


//
// experimentally make max=0xF....E and notfound=0xF...F
// #define mulle_range_location_max       ((~(uintptr_t) 0) - 1)
// compatible would be max=0x7....E and notfound=0x7...F
//
// Why this is moot: if we are storing void *, they will take up 2 or 3 bits
// worth of address space
//
static inline struct mulle_range
   mulle_range_make( uintptr_t location, uintptr_t length)
{
   struct mulle_range   range;

/* Oftentimes it's OK to create invalid ranges, and filter them out
   later with mulle_range_is_valid, as the checking logic needs not to be done 
   "before" mulle_range is created 

   assert( location >= mulle_range_location_min && location <= mulle_range_location_max);
   assert( location + length <= mulle_range_location_max + 1);
   assert( location + length >= location);         // wrap around
*/
   range.location = location;
   range.length   = length;
   return( range);
}


static inline struct mulle_range
   mulle_range_make_all( void)
{
    struct mulle_range   range;

    range.location = 0;
    range.length   = -1;
    return( range);
}


static inline struct mulle_range
   mulle_range_make_invalid( void)
{
    struct mulle_range   range;

    range.location = mulle_not_found_e;
    range.length   = 0;
    return( range);
}


static inline int  mulle_range_is_valid( struct mulle_range range)
{
   uintptr_t   end;

   if( range.location > mulle_range_location_max)
      return( 0);

   // zero length is always valid if location is sane
   if( ! range.length)
      return( 1);

   end = range.location + range.length;
   return( end > range.location && end <= mulle_range_location_max + 1);
}

static inline uintptr_t   _mulle_range_get_min( struct mulle_range range)
{
   return( range.location);
}


static inline uintptr_t   mulle_range_get_min( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));

   return( range.location);
}


static inline uintptr_t   _mulle_range_get_max( struct mulle_range range)
{
   return( range.location + range.length);
}


// use max here like NSMaxRange
static inline uintptr_t   mulle_range_get_max( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));

   return( range.location + range.length);
}


static inline uintptr_t   _mulle_range_get_first_location( struct mulle_range range)
{
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location);
}


static inline uintptr_t   mulle_range_get_first_location( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location);
}


static inline uintptr_t   _mulle_range_get_last_location( struct mulle_range range)
{
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location + range.length - 1);
}


static inline uintptr_t   mulle_range_get_last_location( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location + range.length - 1);
}


static inline int   _mulle_range_contains_location( struct mulle_range range,
                                                    uintptr_t location)
{
   return( location - range.location < range.length);
}


static inline int   mulle_range_contains_location( struct mulle_range range,
                                                   uintptr_t location)
{
   assert( mulle_range_is_valid( range));

   return( location - range.location < range.length);
}


static inline int   mulle_range_less_than_or_equal_to_location( struct mulle_range range,
                                                                uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( mulle_range_get_last_location( range) <= location);
}

//  |..........|  location.
static inline int   mulle_range_less_than_location( struct mulle_range range,
                                                    uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( mulle_range_get_last_location( range) < location);
}


static inline int   mulle_range_greater_than_or_equal_to_location( struct mulle_range range,
                                                                   uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( range.location >= location);
}


//  location  |..........|
static inline int   mulle_range_greater_than_location( struct mulle_range range,
                                                              uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( range.location > location);
}


//  distance is an absolute value
static inline uintptr_t   mulle_range_distance_to_location( struct mulle_range range,
                                                            uintptr_t location)
{
   if( ! range.length)
      return( UINTPTR_MAX);

   if( location >= range.location)
   {
      if( location < mulle_range_get_max( range))
         return( 0);
      return( location - mulle_range_get_last_location( range));
   }
   return( range.location - location);
}



static inline struct mulle_range
   mulle_range_validate_against_length( struct mulle_range range,
                                        uintptr_t length)
{
   uintptr_t  end;

   //
   // specialty, if length == -1, it means "full" range
   // this speeds up these cases, where you want to specify full range
   // but need to call -length first to create the range, and then
   // later call -length again to validate the range...
   //
   if( range.length == (uintptr_t) -1)
      range = mulle_range_make( 0, length);

   //
   // assume NSUInteger is 8 bit and range is { 3, 255 }, then we need to
   // check also for a negative length/location value making things difficult
   //
   end = mulle_range_get_max( range);
   if( end > length || end < range.location)
      return( mulle_range_make_invalid());

   return( range);
}

//
// the problem here is mostly, if a zero length range can contain another
// zero length range. Does a non-zero length range contain a zero length
// range. We say yes if the location is in range.
//
static inline int  mulle_range_contains( struct mulle_range big, struct mulle_range small)
{
   if( ! mulle_range_contains_location( big, small.location))
      return( 0);
   if( ! small.length)
      return( 1);
   return( mulle_range_contains_location( big, mulle_range_get_max( small) - 1));
}


MULLE__DATA_GLOBAL
struct mulle_range   mulle_range_intersect( struct mulle_range range,
                                            struct mulle_range other);

MULLE__DATA_GLOBAL
struct mulle_range   mulle_range_union( struct mulle_range range,
                                        struct mulle_range other);

// this punches holes into ranges, you can get 1 or 2 ranges back as the
// result
MULLE__DATA_GLOBAL
unsigned int   mulle_range_subtract( struct mulle_range a,
                                     struct mulle_range b,
                                     struct mulle_range result[ 2]);

//
// Computes the state of 'a' after insertion of a range 'b'. 'b' must be
// adjacent, or intersect 'a' (else 0 is returned).
// You get either one or two result ranges back. If you get two, then
// 'b' created a hole (not part of the result). result[ 0] is the unshifted
// range and result[1] is the shifted range:
//
// Example a=[0-9] b=[2-3], result[ 2] = { [0-2], [5-14] }
//
MULLE__DATA_GLOBAL
unsigned int   mulle_range_insert( struct mulle_range a,
                                   struct mulle_range b,
                                   struct mulle_range result[ 2]);


// this punches holes into ranges, you can get 1 or 2 ranges back as the
// result
static inline
unsigned int   mulle_range_subtract_location( struct mulle_range a,
                                              uintptr_t location,
                                              struct mulle_range result[ 2])
{
   return( mulle_range_subtract( a, mulle_range_make( location, 1), result));
}

//
// Find a hole. Or rather the place in buf, were you would insert a new range.
// **This only works if its been ascertained that location is not already
// contained. (i.e. run mulle_range_contains_bsearch beforehand). You get the
// index back where you want to insert a new range.
// This can be 0 to n (incl), so don't use the return value unverified to
// buf, as it could overflow.
//
unsigned int   _mulle_range_hole_bsearch( struct mulle_range *buf,
                                          unsigned int n,
                                          uintptr_t search_location);

// returns range containing search or NULL
MULLE__DATA_GLOBAL
struct mulle_range   *mulle_range_contains_bsearch( struct mulle_range *buf,
                                                    unsigned int n,
                                                    struct mulle_range search);

MULLE__DATA_GLOBAL
struct mulle_range   *mulle_range_intersects_bsearch( struct mulle_range *buf,
                                                      unsigned int n,
                                                      struct mulle_range search);



#define mulle_range_for( range, name)                                \
   for( uintptr_t name          = (range).location,                  \
                  name ## __end = (range).location + (range).length; \
        name < name ## __end;                                        \
        ++name                                                       \
      )

#endif
