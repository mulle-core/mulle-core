#include <mulle-regex/mulle-regex.h>
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>


static int   _test( struct mulle_utf32regex  *r,
                    mulle_utf32_t *s,
                    mulle_utf32_t *pattern,
                    mulle_utf32_t *replace)
{
   int             result;
   mulle_utf32_t   *buf;
   size_t          dst_length;

   result = mulle_utf32regex_execute( r, s);
   if( result < 0)
   {
      mulle_printf( "execute fail\n");
      return(1);
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

   mulle_printf( "%S\n", buf);
   mulle_free( buf);
   return( 0);
}


static int   test( mulle_utf32_t *s, mulle_utf32_t *pattern, mulle_utf32_t *replace)
{
   struct mulle_utf32regex   *r;
   int                       rc;

   mulle_printf( "%S/%S/%S: ", s, pattern, replace);

   r = mulle_utf32regex_compile( pattern);
   if( ! r)
   {
      mulle_printf( "compile fail\n");
      return( 1);;
   }

   rc = _test( r, s, pattern, replace);

   mulle_utf32regex_free( r);
   return( rc);
}


//
// run this with --valgrind, this is just to "prove" that we don't
// overflow / crash with UTF32 input as the original tests just do ascii
//
int   main( int argc, char *argv[])
{
   int             errors;
   mulle_utf32_t   pattern_utf15[] = { '(', 23589, 21787, '*', ')', 15173, 15909, 0 };
   mulle_utf32_t   pattern_big[]   = { 0x0000c67c, '(', 0x000e49d5, '|', 0x000c3154, ')', '+', 0x000d303d, 0 };
   mulle_utf32_t   text_utf15[]    = { 23589, 21787, 21787, 15173, 15909, 0 };
   mulle_utf32_t   text_big[]      = { 0x0000c67c, 0x000e49d5, 0x000c3154, 0x000d303d, 0 };
   mulle_utf32_t   subst_utf15[]   = { 's', 23589, 21787, 15173, 15909, 0 };
   mulle_utf32_t   subst_big[]     = { 's', 0x00002603, 0x0000FE0F, 'd', 0 };

   errors = 0;

   errors += test( text_utf15, pattern_utf15, subst_utf15);
   errors += test( text_utf15, pattern_utf15, subst_big);
   errors += test( text_utf15, pattern_big, subst_utf15);
   errors += test( text_utf15, pattern_big, subst_big);

   errors += test( text_big, pattern_utf15, subst_utf15);
   errors += test( text_big, pattern_utf15, subst_big);
   errors += test( text_big, pattern_big, subst_utf15);
   errors += test( text_big, pattern_big, subst_big);

   return( errors ? 1 : 0);
}
