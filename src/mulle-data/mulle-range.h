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
//# if defined( _WIN32) || defined( _WIN64)
   typedef ptrdiff_t  intptr_t
#  define INTPTR_MIN   PTRDIFF_MIN
#  define INTPTR_MAX   PTRDIFF_MAX
//# else
//# // error "intptr_t and friends are needed for this platform"
//# endif
#endif


/**
 * A constant representing a "not found" value for an `intptr_t` type.
 * This value is set to the maximum possible value of an `intptr_t` type,
 * which is a convenient way to represent a "not found" or error condition.
 * MEMO: It's convenient this to be different to "notakey"
 */
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


/**
 * `mulle_range_location_min`: The minimum valid location value for a `mulle_range` struct.
 *
 * `mulle_range_location_max`: The maximum valid location value for a `mulle_range` struct. This is one less than the `mulle_not_found_e` value, which represents an "not found" condition.
 *
 * `mulle_range_zero`: A `mulle_range` struct with both location and length set to 0, representing an empty or invalid range.
 */
// like uintptr_min
#define mulle_range_location_min       (0)
// like uintptr_max, not like get_max though!!
#define mulle_range_location_max       (mulle_not_found_e-1)
#define mulle_range_zero               ((struct mulle_range) { 0, 0 })

// the struct and the three defines need to stay compatible with MulleObjC/mulle-objc-type.h


/**
 * Creates a `mulle_range` struct with the given location and length.
 *
 * @param location The starting location of the range.
 * @param length The length of the range.
 * @return A `mulle_range` struct with the specified location and length.
 */
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


/**
 * Creates a `mulle_range` struct that represents a range of locations.
 *
 * The range is defined by the starting location (`location`) and the ending
 * location (`location2`). The length of the range is calculated as
 * `location2 - location + 1`.
 *
 * If `location` is less than `location2`, the range will start at `location`
 * and end at `location2`. If `location` is greater than `location2`, the range
 * will start at `location2` and end at `location`.
 *
 * @param location The starting location of the range.
 * @param location2 The ending location of the range.
 * @return A `mulle_range` struct that represents the specified range of locations.
 */
//
// a location is 1 wide, so location 1 to location 4 is 1, 2, 3, 4 = 4 length
// so 4 - 1 +1
//
static inline struct mulle_range
   mulle_range_make_locations( uintptr_t location, uintptr_t location2)
{
   if( location < location2)
      return( mulle_range_make( location, location2 - location + 1));
   return( mulle_range_make( location2, location - location2 + 1));
}




/**
 * Creates a `mulle_range` struct that represents the full range of valid locations.
 *
 * The returned `mulle_range` struct will have a `location` of 0 and a `length`
 * of the maximum possible value for the `uintptr_t` type, representing the
 * full range of valid locations.
 *
 * @return A `mulle_range` struct that represents the full range of valid
 * locations.
 */
static inline struct mulle_range   mulle_range_make_all( void)
{
    struct mulle_range   range;

    range.location = 0;
    range.length   = -1;
    return( range);
}


/**
 * Creates a `mulle_range` struct that represents an invalid range.
 *
 * The returned `mulle_range` struct will have a `location` of
 * `mulle_not_found_e` and a `length` of 0, indicating an invalid range.
 *
 * @return A `mulle_range` struct that represents an invalid range.
 */
static inline struct mulle_range    mulle_range_make_invalid( void)
{
    struct mulle_range   range;

    range.location = mulle_not_found_e;
    range.length   = 0;
    return( range);
}


/**
 * Checks if two ranges are equal
 *
 * @return 1 if both `mulle_range` are identical, 0 otherwise.
 */
static inline int  mulle_range_equals( struct mulle_range range, struct mulle_range other)
{
   return( range.location == other.location && range.length == other.length);
}



/**
 * Checks if a given `mulle_range` struct represents a valid range.
 *
 * A `mulle_range` is considered valid if its `location` is less than or equal
 * to the maximum possible `uintptr_t` value, and its `length` is greater than
 * 0 and its `location + length` is less than or equal to the maximum possible
 * `uintptr_t` value plus 1.
 *
 * @param range The `mulle_range` struct to check for validity.
 * @return 1 if the `mulle_range` is valid, 0 otherwise.
 */
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


/**
 * Returns the minimum value of the given `mulle_range` struct.
 *
 * This function is an implementation detail and is not part of the public API.
 *
 * @param range The `mulle_range` struct to get the minimum value from.
 * @return The minimum value of the given `mulle_range` struct.
 */
static inline uintptr_t   _mulle_range_get_min( struct mulle_range range)
{
   return( range.location);
}


/**
 * Returns the minimum value of the given `mulle_range` struct.
 *
 * This function is part of the public API.
 *
 * @param range The `mulle_range` struct to get the minimum value from.
 * @return The minimum value of the given `mulle_range` struct.
 */
static inline uintptr_t   mulle_range_get_min( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));

   return( range.location);
}


/**
 * Returns the maximum value of the given `mulle_range` struct.
 *
 * This function is an implementation detail and is not part of the public API.
 *
 * @param range The `mulle_range` struct to get the maximum value from.
 * @return The maximum value of the given `mulle_range` struct.
 */
static inline uintptr_t   _mulle_range_get_max( struct mulle_range range)
{
   return( range.location + range.length);
}


/**
 * Returns the maximum value of the given `mulle_range` struct.
 *
 * This function is part of the public API.
 *
 * @param range The `mulle_range` struct to get the maximum value from.
 * @return The maximum value of the given `mulle_range` struct.
 */
// use max here like NSMaxRange
static inline uintptr_t   mulle_range_get_max( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));

   return( range.location + range.length);
}


/**
 * Returns the first location of the given `mulle_range` struct.
 *
 * This function is an implementation detail and is not part of the public API.
 *
 * @param range The `mulle_range` struct to get the first location from.
 * @return The first location of the given `mulle_range` struct, or `mulle_not_found_e` if the length is 0.
 */
static inline uintptr_t   _mulle_range_get_first_location( struct mulle_range range)
{
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location);
}


/**
 * Returns the first location of the given `mulle_range` struct.
 *
 * This function is part of the public API.
 *
 * @param range The `mulle_range` struct to get the first location from.
 * @return The first location of the given `mulle_range` struct, or `mulle_not_found_e` if the length is 0.
 */
static inline uintptr_t   mulle_range_get_first_location( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location);
}


/**
 * Returns the last location of the given `mulle_range` struct.
 *
 * This function is an implementation detail and is not part of the public API.
 *
 * @param range The `mulle_range` struct to get the last location from.
 * @return The last location of the given `mulle_range` struct, or `mulle_not_found_e` if the length is 0.
 */
static inline uintptr_t   _mulle_range_get_last_location( struct mulle_range range)
{
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location + range.length - 1);
}


/**
 * Returns the last location of the given `mulle_range` struct.
 *
 * This function is part of the public API.
 *
 * @param range The `mulle_range` struct to get the last location from.
 * @return The last location of the given `mulle_range` struct, or `mulle_not_found_e` if the length is 0.
 */
static inline uintptr_t   mulle_range_get_last_location( struct mulle_range range)
{
   assert( mulle_range_is_valid( range));
   if( ! range.length)
      return( mulle_not_found_e);
   return( range.location + range.length - 1);
}


/**
 * Returns whether the given location is contained within the specified `mulle_range` struct.
 *
 * This function is an implementation detail and is not part of the public API.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to check if it is contained within the range.
 * @return 1 if the location is contained within the range, 0 otherwise.
 */
static inline int   _mulle_range_contains_location( struct mulle_range range,
                                                    uintptr_t location)
{
   return( location - range.location < range.length);
}


/**
 * Returns whether the given location is contained within the specified `mulle_range` struct.
 *
 * This function is part of the public API.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to check if it is contained within the range.
 * @return 1 if the location is contained within the range, 0 otherwise.
 */
static inline int   mulle_range_contains_location( struct mulle_range range,
                                                   uintptr_t location)
{
   assert( mulle_range_is_valid( range));

   return( location - range.location < range.length);
}


/**
 * Returns whether the given location is less than or equal to the last location of the specified `mulle_range` struct.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to compare against the last location of the range.
 * @return 1 if the given location is less than or equal to the last location of the range, 0 otherwise.
 */
static inline int   mulle_range_less_than_or_equal_to_location( struct mulle_range range,
                                                                uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( mulle_range_get_last_location( range) <= location);
}


/**
 * Returns whether the given location is less than the last location of the specified `mulle_range` struct.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to compare against the last location of the range.
 * @return 1 if the given location is less than the last location of the range, 0 otherwise.
 */
//  |..........|  location.
static inline int   mulle_range_less_than_location( struct mulle_range range,
                                                    uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( mulle_range_get_last_location( range) < location);
}


/**
 * Returns whether the given location is greater than or equal to the start
 * location of the specified `mulle_range` struct.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to compare against the start location of the
 *        range.
 * @return 1 if the given location is greater than or equal to the start
 *         location of the range, 0 otherwise.
 */
static inline int   mulle_range_greater_than_or_equal_to_location( struct mulle_range range,
                                                                   uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( range.location >= location);
}


/**
 * Returns whether the given location is greater than the start location of the specified `mulle_range` struct.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to compare against the start location of the range.
 * @return 1 if the given location is greater than the start location of the range, 0 otherwise.
 */
//  location  |..........|
static inline int   mulle_range_greater_than_location( struct mulle_range range,
                                                              uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( range.location > location);
}


/**
 * Returns whether the given location is same as the start location of the
 * specified `mulle_range` struct.
 *
 * @param range The `mulle_range` struct to check.
 * @param location The location to compare against the start location of the range.
 * @return 1 if the given location is greater than the start location of the range, 0 otherwise.
 */
//  location  |..........|
static inline int   mulle_range_equal_to_location( struct mulle_range range,
                                                   uintptr_t location)
{
   if( ! range.length)
      return( 0);

   return( range.location == location);
}


/**
 * Returns the absolute distance between the specified `mulle_range` and a given location.
 *
 * @param range The `mulle_range` struct to calculate the distance from.
 * @param location The location to calculate the distance to.
 * @return The absolute distance between the range and the location, or `UINTPTR_MAX` if the range is empty.
 */
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


/**
 * Validates the given `mulle_range` against the specified length.
 *
 * If the `range.length` is equal to `(uintptr_t)-1`, it is treated as a
 * "full" range and the range is updated to cover the entire length.
 *
 * If the end of the range is greater than the specified length, or the end
 * is less than the start location, the function returns an invalid
 * `mulle_range`.
 *
 * @param range The `mulle_range` to validate.
 * @param length The length to validate the range against.
 * @return The validated `mulle_range`. If the range is invalid, an invalid
 *         `mulle_range` is returned.
 */
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
   if( range.length == (uintptr_t) -1)
      range = mulle_range_make( range.location, length - range.location);

   //
   // assume NSUInteger is 8 bit and range is { 3, 255 }, then we need to
   // check also for a negative length/location value making things difficult
   //
   end = range.location + range.length;  // don't want to assert here
   if( end > length || end < range.location)
      return( mulle_range_make_invalid());

   return( range);
}


/**
 * Checks if a larger `mulle_range` contains a smaller `mulle_range`.
 *
 * The function handles the case where the smaller `mulle_range` has a length
 * of 0. In this case, the function returns 1 (true) if the location of the
 * smaller `mulle_range` is within the bounds of the larger `mulle_range`.
 *
 * If the smaller `mulle_range` has a non-zero length, the function checks if
 * the last location of the smaller `mulle_range` is also within the bounds of
 * the larger `mulle_range`.
 *
 * @param big The larger `mulle_range` to check.
 * @param small The smaller `mulle_range` to check if it is contained within
 *        the larger `mulle_range`.
 * @return 1 (true) if the smaller `mulle_range` is contained within the larger
 *         `mulle_range`, 0 (false) otherwise.
 *///
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


/**
 * Checks if two `mulle_range`s intersect`.
 *
 * @param a The one `mulle_range` to check.
 * @param b The other `mulle_range` to check
 * @return 1 (true) if the `mulle_range`s intersect, 0 (false) otherwise.
 *///
int   mulle_range_intersects( struct mulle_range range,
                              struct mulle_range other);

/**
 * Computes the intersection of two `mulle_range` values.
 *
 * This function takes two `mulle_range` values and returns a new `mulle_range`
 * that represents the intersection of the two input ranges. If the two input
 * ranges do not overlap, the returned `mulle_range` will be invalid (i.e.
 * `mulle_range_is_valid(result) == 0`).
 *
 * @param range The first `mulle_range` value.
 * @param other The second `mulle_range` value.
 * @return The intersection of the two input `mulle_range` values.
 */
MULLE__DATA_GLOBAL
struct mulle_range   mulle_range_intersection( struct mulle_range range,
                                               struct mulle_range other);


// old name
static inline
struct mulle_range   mulle_range_intersect( struct mulle_range range,
                                            struct mulle_range other)
{
   return( mulle_range_intersection( range, other));
}


/**
 * Computes the union of two `mulle_range` values.
 *
 * This function takes two `mulle_range` values and returns a new `mulle_range`
 * that represents the union of the two input ranges. If the two input
 * ranges do not overlap, the returned `mulle_range` will represent the
 * concatenation of the two input ranges.
 *
 * @param range The first `mulle_range` value.
 * @param other The second `mulle_range` value.
 * @return The union of the two input `mulle_range` values.
 */
MULLE__DATA_GLOBAL
struct mulle_range   mulle_range_union( struct mulle_range range,
                                        struct mulle_range other);

/**
 * Subtracts a `mulle_range` from another `mulle_range`.
 *
 * This function takes two `mulle_range` values, `a` and `b`, and computes the
 * result of subtracting `b` from `a`. The result is stored in the `result`
 * array, which will contain either one or two `mulle_range` values.
 *
 * If `b` completely overlaps `a`, the resulting ranges will be `{ 0, 0 }, { 0, 0 }`.
 * If the resulting range starts with `a.location`, it will be stored in `result[0]`,
 * otherwise it will be stored in `result[1]. This ensures that `0` is always the
 * prefix and `1` is the suffix. Therefore, it's possible that `result[0].length == 0`
 * and `result[1].length > 0` (i.e. `b` cuts off the head of `a`).
 *
 * @param a The `mulle_range` to subtract from.
 * @param b The `mulle_range` to subtract.
 * @param result An array of two `mulle_range` values to store the result.
 */
//
// this punches holes into ranges, you get two ranges back as the
// result. one is the left side of the hole, and the other the right side
// if b completely clobbers a, the ranges will be { 0, 0 }, { 0, 0 }
// If the resulting range starts with a.location it will be in result[ 0]
// otherwise in result[ 1]. This is so, that 0 is always the prefix and 1 is
// the suffix. Therefore its possible that result[0].length == 0 and
// result[1].length > 0 (b cuts of a's head)
//
MULLE__DATA_GLOBAL
void   mulle_range_subtract( struct mulle_range a,
                             struct mulle_range b,
                             struct mulle_range result[ 2]);



/**
 * Subtracts a location from a `mulle_range`.
 *
 * This function takes a `mulle_range` value `a` and a location `location`, and
 * computes the result of subtracting the range `{ location, 1 }` from `a`. The
 * result is stored in the `result` array, which will contain either one or two
 * `mulle_range` values.
 *
 * If the resulting range starts with `a.location`, it will be stored in
 * `result[0]`, otherwise it will be stored in `result[1]`. This ensures that `0`
 * is always the prefix and `1` is the suffix. Therefore, it's possible that
 * `result[0].length == 0` and `result[1].length > 0` (i.e. `location` cuts off
 * the head of `a`).
 *
 * @param a The `mulle_range` to subtract from.
 * @param location The location to subtract.
 * @param result An array of two `mulle_range` values to store the result.
 */
// this punches holes into ranges, you can get 2 ranges back as the
// result, one is the left side of the hole, and the other the right side
static inline
void   mulle_range_subtract_location( struct mulle_range a,
                                      uintptr_t location,
                                      struct mulle_range result[ 2])
{
   mulle_range_subtract( a, mulle_range_make( location, 1), result);
}


/**
 * Computes the state of the `mulle_range` `a` after insertion of the `mulle_range` `b`.
 * `b` must be adjacent to or intersect `a` (else 0 is returned).
 * The function returns either one or two result ranges. If two ranges are returned,
 * then `b` created a hole (not part of the result). `result[0]` is the unshifted
 * range and `result[1]` is the shifted range.
 *
 * Example: `a=[0-9] b=[2-3], result[2] = { [0-2], [5-14] }`
 *
 * @param a The `mulle_range` to insert `b` into.
 * @param b The `mulle_range` to insert.
 * @param result An array of two `mulle_range` values to store the result.
 * @return The number of `mulle_range` values stored in the `result` array (either 1 or 2).
 */
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

/**
 * Performs a binary search on the provided `mulle_range` array to find the index
 * where a new `mulle_range` with the given `search_location` should be inserted.
 *
 * This function assumes that the `search_location` is not already contained in
 * the `mulle_range` array. You should call `mulle_range_contains_bsearch` first
 * to verify this.
 *
 * The returned index can be from 0 to `n` (inclusive), so the caller should
 * verify the index before using it to access the `buf` array, as it could
 * overflow.
 *
 * @param buf The `mulle_range` array to search.
 * @param n The number of elements in the `buf` array.
 * @param search_location The location to search for in the `mulle_range` array.
 * @return The index where a new `mulle_range` with the given `search_location`
 *         should be inserted.
 */
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

/**
 * Performs a binary search on the provided `mulle_range` array to find the index
 * of the `mulle_range` that contains the given `search` range.
 *
 * @param buf The `mulle_range` array to search.
 * @param n The number of elements in the `buf` array.
 * @param search The `mulle_range` to search for in the `buf` array.
 * @return The `mulle_range` in the `buf` array that contains the `search` range,
 *         or `NULL` if no such `mulle_range` is found.
 */
// returns range containing search or NULL
MULLE__DATA_GLOBAL
struct mulle_range   *mulle_range_contains_bsearch( struct mulle_range *buf,
                                                    unsigned int n,
                                                    struct mulle_range search);

/**
 * Performs a binary search on the provided `mulle_range` array to find the index
 * of the `mulle_range` that intersects the given `search` range.
 *
 * @param buf The `mulle_range` array to search.
 * @param n The number of elements in the `buf` array.
 * @param search The `mulle_range` to search for in the `buf` array.
 * @return The `mulle_range` in the `buf` array that intersects the `search` range,
 *         or `NULL` if no such `mulle_range` is found.
 */
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
