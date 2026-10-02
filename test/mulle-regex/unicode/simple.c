#include <mulle-regex/mulle-regex.h>
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>


static int   _test( struct mulle_utf32regex  *r,
                    mulle_utf32_t *s,
                    mulle_utf32_t *pattern,
                    mulle_utf32_t *replace)
{
   int                  result;
   mulle_utf32_t        *buf;
   size_t               dst_length;
   struct mulle_range   fixRanges[ 2];
   struct mulle_range   range;

   result = mulle_utf32regex_execute( r, s);
   if( result < 0)
   {
      mulle_printf( "execute fail\n");
      return( 1);
   }

   if( result == 0)  // no match, keep what we've got
   {
      mulle_printf( "no match\n");
      return( 0);
   }

   dst_length = mulle_utf32regex_substitution_length( r, replace);
   buf        = mulle_malloc( sizeof( mulle_utf32_t) * (dst_length + 1));
   result     = mulle_utf32regex_substitute( r, replace, buf, dst_length + 1, 1);
   if( result < 0)
   {
      mulle_printf( "substitution fail\n");
      mulle_free( buf);
      return( 1);
   }

   range = mulle_utf32regex_range_for_index( r, 0);
   mulle_range_subtract( mulle_range_make( 0, mulle_utf32_strlen( s)), range, fixRanges);
   mulle_printf( "%.*S%S%.*S\n",
                     (int) fixRanges[ 0].length, &s[ fixRanges[ 0].location],
                     buf,
                     (int) fixRanges[ 1].length, &s[ fixRanges[ 1].location]);
   mulle_free( buf);
   return( 0);
}


static int   test( mulle_utf32_t *s, mulle_utf32_t *pattern, mulle_utf32_t *replace)
{
   struct mulle_utf32regex   *r;
   int                       rc;

   mulle_printf( "%S | %S -> %S: ", s, pattern, replace);

   r = mulle_utf32regex_compile( pattern);
   if( ! r)
   {
      mulle_printf( "compile fail\n");
      return( 1);
   }

   rc = _test( r, s, pattern, replace);

   mulle_utf32regex_free( r);
   return( rc);
}


//
// This little test "acertains" that mulle_utf32regex_substitute does not
// keep the prefix/suffix of the original string, which is somewhat
// disappointing (you have to do it yourself). This is solved in
// NSString+Regex.m though
//
int   main( int argc, char *argv[])
{
   int             errors;
   mulle_utf32_t   text[]    = { 'a', 'a', 'b', 'b', 'c', 'c', 'd', 'd',  0 };
   mulle_utf32_t   pattern[] = { '(', 'b', '+', ')', '(', 'c', '+', ')',  0 };
   mulle_utf32_t   subst[]   = { '{', '\\', '1', ',', '\\', '2', ' ', '/', '/', ' ', '\\', '1', ' ', '\\', '2', '}', 0 };

   errors = 0;

   errors += test( text, pattern, subst);

   return( errors ? 1 : 0);
}
