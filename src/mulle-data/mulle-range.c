#include "mulle-range.h"

#include <assert.h>


struct mulle_range   mulle_range_intersect( struct mulle_range range,
                                            struct mulle_range other)
{
   uintptr_t            location;
   uintptr_t            min;
   uintptr_t            end1;
   uintptr_t            end2;
   struct mulle_range   result;

   end1 = mulle_range_get_end( range);
   end2 = mulle_range_get_end( other);
   min  = (end1 < end2) ? end1 : end2;
   location  = (range.location > other.location) ? range.location : other.location;

   if( min < location)
      result.location = result.length = 0;
   else
   {
      result.location = location;
      result.length   = min - location;
   }

   return( result);
}


struct mulle_range   mulle_range_union( struct mulle_range range,
                                        struct mulle_range other)
{
   uintptr_t            location;
   uintptr_t            end;
   uintptr_t            end1;
   uintptr_t            end2;
   struct mulle_range   result;

   if( ! other.length)
      return( range);
   if( ! range.length)
      return( other);

   end1      = mulle_range_get_end( range);
   end2      = mulle_range_get_end( other);
   end       = (end1 > end2) ? end1 : end2;
   location  = (range.location < other.location) ? range.location : other.location;

   result.location = location;
   result.length   = end - result.location;
   return(result);
}



//
// Find a hole, this only works if its been ascertained
// that location is not contained
//
unsigned int   _mulle_range_hole_bsearch( struct mulle_range *buf,
                                          unsigned int n,
                                          uintptr_t search_location)
{
   int                   first;
   int                   last;
   int                   middle;
   struct mulle_range   *p;

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->location < search_location)
      {
         if( middle == last || p[ 1].location > search_location)
            return( middle + 1);

         first = middle + 1;
      }
      else
      {
         last = middle - 1;
      }

      middle = (first + last) / 2;
   }

   return( last + 1);
}


// find
struct mulle_range   *mulle_range_contains_bsearch( struct mulle_range *buf,
                                                    unsigned int n,
                                                    struct mulle_range search)
{
   int                  first;
   int                  last;
   int                  middle;
   struct mulle_range   *p;

   if( ! mulle_range_is_valid( search))
      return( 0);

   if( ! buf || ! n || ! search.length)
      return( 0);

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->location <= search.location)
      {
         if( mulle_range_contains( *p, search))
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( 0);
}


// find
struct mulle_range   *mulle_range_intersects_bsearch( struct mulle_range *buf,
                                                      unsigned int n,
                                                      struct mulle_range search)
{
   int                  first;
   int                  last;
   int                  middle;
   struct mulle_range   *p;

   if( ! mulle_range_is_valid( search))
      return( 0);

   if( ! buf || ! n || ! search.length)
      return( 0);

   first  = 0;
   last   = n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->location < mulle_range_get_end( search))
      {
         if( mulle_range_intersect( *p, search).length)
            return( p);

         first = middle + 1;
      }
      else
         last = middle - 1;

      middle = (first + last) / 2;
   }

   return( 0);
}


unsigned int   mulle_range_subtract( struct mulle_range a,
                                     struct mulle_range b,
                                     struct mulle_range result[ 2])
{
   uintptr_t   a_end;
   uintptr_t   b_end;

   // 1. completely separate
   //
   //   b.....b_end
   //                 a.........a_end
   //

   if( ! mulle_range_intersect( a, b).length)
   {
      result[ 0] = a;
      return( 1);
   }

   a_end = mulle_range_get_end( a);
   b_end = mulle_range_get_end( b);

   // 2.
   //
   //              a.....a_end
   //          b..............b_end
   //
   //     make a hole
   //
   if( b.location <= a.location && b_end >= a_end)
   {
      result[ 0] = mulle_range_make( 0, 0);
      return( 1);
   }


   // 3.
   //
   //              b.....b_end
   //          a..............a_end
   //
   //     make a hole
   //
   if( a.location < b.location && a_end > b_end)
   {
      result[ 0] = mulle_range_make( a.location, b.location - a.location);
      result[ 1] = mulle_range_make( b_end, a_end - b_end);
      return( 2);
   }

   // 4. range removes part of the front
   //
   //   b.....b_end
   //      a.........a_end
   //
   if( a_end > b_end)
   {
      result[ 0] = mulle_range_make( b_end, a_end - b_end);
      return( 1);
   }

   // 5. range removes part of the back
   //
   //        b.......b_end
   //  a.........a_end
   //
   result[ 0] = mulle_range_make( a.location, b.location - a.location);
   return( 1);
}
