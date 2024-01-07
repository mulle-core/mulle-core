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

   end1 = mulle_range_get_max( range);
   end2 = mulle_range_get_max( other);
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

   end1      = mulle_range_get_max( range);
   end2      = mulle_range_get_max( other);
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
   last   = (int) n - 1;
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

   if( ! buf)
      return( 0);

   first  = 0;
   last   = (int) n - 1;
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
   last   = (int) n - 1;
   middle = (first + last) / 2;

   while( first <= last)
   {
      p = &buf[ middle];
      if( p->location < mulle_range_get_max( search))
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


void   mulle_range_subtract( struct mulle_range a,
                             struct mulle_range b,
                             struct mulle_range result[ 2])
{
   uintptr_t   a_end;
   uintptr_t   b_end;

   // 1. completely separate
   //
   //                 a.........a_end
   //   b.....b_end
   //

   if( ! mulle_range_intersect( a, b).length)
   {
      result[ 0] = a;
      result[ 1] = mulle_range_zero;
      return;
   }

   a_end = mulle_range_get_max( a);
   b_end = mulle_range_get_max( b);

   // 2. completely clobbers
   //
   //              a.....a_end
   //          b..............b_end
   //
   if( b.location <= a.location && b_end >= a_end)
   {
      result[ 0] =
      result[ 1] = mulle_range_zero;
      return;
   }


   // 3.
   //          a..............a_end
   //              b.....b_end
   //
   //     make a hole
   //
   if( a.location < b.location && a_end > b_end)
   {
      result[ 0] = mulle_range_make( a.location, b.location - a.location);
      result[ 1] = mulle_range_make( b_end, a_end - b_end);
      return;
   }

   // 4. range removes part of the front
   //
   //      a.........a_end
   //   b.....b_end
   //
   if( a_end > b_end)
   {
      result[ 0] = mulle_range_zero;
      result[ 1] = mulle_range_make( b_end, a_end - b_end);
      return;
   }

   // 5. range removes part of the back
   //
   //  a.........a_end
   //        b.......b_end
   //
   result[ 0] = mulle_range_make( a.location, b.location - a.location);
   result[ 1] = mulle_range_zero;
}


MULLE__DATA_GLOBAL
unsigned int   mulle_range_insert( struct mulle_range a,
                                   struct mulle_range b,
                                   struct mulle_range result[ 2])
{
   uintptr_t   a_end;
   uintptr_t   b_end;

   if( ! mulle_range_is_valid( a))
      return( 0);

   if( ! b.length)
      return( 0); // no like

   a_end = _mulle_range_get_max( a);
   b_end = _mulle_range_get_max( b);

   // 1. completely separate
   //
   //                 a.........a_end
   //   b.....b_end
   //
   if( b.location > a_end || b_end < a.location)
      return( 0); // no like


   // 2.
   //           111 2222222222
   //          a...|..........a_end
   //              b.....b_end
   //
   // same as
   //           111 2222222222
   //          a...|..........a_end
   //              b..................b_end
   //     make a hole
   //
   if( b.location >= a.location)
   {
      result[ 0] = mulle_range_make( a.location, b.location - a.location);
      result[ 1] = mulle_range_make( b_end, a_end - b.location);
      return( 2);
   }

   // 3.
   //           111 2222222222
   //          a..............a_end
   //       b.....b_end
   result[ 0] = mulle_range_zero;
   result[ 1] = mulle_range_make( b_end, a.length);
   return( 2);
}
