//
//  range.c
//  mulle-regex
//
//  mulle_utf32regex_range_for_index() has to report a group that did not
//  participate in the match (and an out of range index) with
//  mulle_range_make_invalid(), i.e. a range that mulle_range_is_valid()
//  rejects. The old code returned LONG_MAX, which happens to equal
//  mulle_not_found_e on LP64, but on LLP64 (Windows) `long` is 32 bit while
//  INTPTR_MAX is 64 bit - so there the sentinel looked like a real location.
//
#include <mulle-regex/mulle-regex.h>
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>


static void   check( struct mulle_utf32regex *r, unsigned int i)
{
   struct mulle_range   range;

   range = mulle_utf32regex_range_for_index( r, i);
   if( mulle_range_is_valid( range))
      mulle_printf( "group[%u] = %lu,%lu valid\n",
                    i,
                    (unsigned long) range.location,
                    (unsigned long) range.length);
   else
      mulle_printf( "group[%u] = invalid\n", i);
}


int   main( int argc, char *argv[])
{
   struct mulle_utf32regex   *r;

   // (a)(b)?(c) on "ac": group 2 does not participate in the match
   r = mulle_utf32regex_compile( (mulle_utf32_t[]) { '(', 'a', ')',
                                                    '(', 'b', ')', '?',
                                                    '(', 'c', ')', 0});
   if( ! r)
   {
      mulle_printf( "compile failed\n");
      return( 1);
   }

   if( mulle_utf32regex_execute( r, (mulle_utf32_t[]) { 'a', 'c', 0}) != 1)
   {
      mulle_printf( "execute failed\n");
      mulle_utf32regex_free( r);
      return( 1);
   }

   check( r, 0);
   check( r, 1);
   check( r, 2);
   check( r, 3);
   check( r, 99);

   mulle_utf32regex_free( r);
   return( 0);
}
