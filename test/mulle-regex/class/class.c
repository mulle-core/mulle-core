//
//  class.c
//  mulle-regex
//
//  MULLE_UTF32REGEX_EXTENDED: escapes inside character classes, the named
//  classes [[:name:]], and a compile failure instead of a silent wrong answer
//  for an escape that does not exist.
//
//  The codepoints above 127 are written out, because whether they print as
//  text depends on the terminal, and a test should not.
//
#include <mulle-regex/mulle-regex.h>
#include <mulle-fprintf/mulle-fprintf.h>
#include <stdio.h>


static int   is_ascii( mulle_utf32_t *s, size_t len)
{
   size_t   i;

   for( i = 0; i < len; i++)
      if( s[ i] < 32 || s[ i] > 126)
         return( 0);
   return( 1);
}


static void   print_match( mulle_utf32_t *s, struct mulle_range range)
{
   size_t   i;

   if( is_ascii( &s[ range.location], range.length))
   {
      mulle_printf( "\"%.*S\"", (int) range.length, &s[ range.location]);
      return;
   }

   for( i = 0; i < range.length; i++)
      mulle_printf( "%sU+%04X", i ? "," : "", (unsigned int) s[ range.location + i]);
}


// returns 1 on error
static int   test( unsigned int options,
                   char *name,
                   mulle_utf32_t *pattern,
                   mulle_utf32_t *s)
{
   struct mulle_utf32regex   *r;
   struct mulle_range        range;
   int                       result;

   mulle_printf( "%s: ", name);

   r = mulle_utf32regex_compile_options( pattern, options);
   if( ! r)
   {
      mulle_printf( "COMPILE FAILED\n");
      return( 0);
   }

   result = mulle_utf32regex_execute( r, s);
   if( result < 0)
   {
      mulle_printf( "EXECUTE FAILED\n");
      mulle_utf32regex_free( r);
      return( 1);
   }

   if( ! result)
   {
      mulle_printf( "NO MATCH\n");
      mulle_utf32regex_free( r);
      return( 0);
   }

   range = mulle_utf32regex_range_for_index( r, 0);
   print_match( s, range);
   mulle_printf( "\n");

   mulle_utf32regex_free( r);
   return( 0);
}


int   main( int argc, char *argv[])
{
   int   errors;

   errors = 0;

   // the flag is off by default, so [[:space:]] is the set { [, :, s, p, a, c, e }
   errors += test( 0, "off-space",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { ' ', ' ', 'x', 0 });

   errors += test( 0, "off-tab-is-t",
                   (mulle_utf32_t[]) { 'a', '\\', 't', 'b', 0 },
                   (mulle_utf32_t[]) { 'a', 't', 'b', 0 });

   // escapes and named classes
   errors += test( MULLE_UTF32REGEX_EXTENDED, "space",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', ' ', ' ', 'b', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "space-in-range",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', 'x', 0 },
                   (mulle_utf32_t[]) { ' ', 'x', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "escaped-tab",
                   (mulle_utf32_t[]) { '[', '\\', 't', ']', 0 },
                   (mulle_utf32_t[]) { 9, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "escaped-tab-is-not-t",
                   (mulle_utf32_t[]) { '[', '\\', 't', ']', 0 },
                   (mulle_utf32_t[]) { 't', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "escaped-range",
                   (mulle_utf32_t[]) { '[', '\\', 't', '-', '\\', 'r', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 9, 10, 11, 12, 13, 'b', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "escaped-bracket",
                   (mulle_utf32_t[]) { '[', '\\', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', ']', ']', 'b', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "alpha",
                   (mulle_utf32_t[]) { '[', '[', ':', 'a', 'l', 'p', 'h', 'a', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'b', 'c', '1', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "alpha-umlaut",
                   (mulle_utf32_t[]) { '[', '[', ':', 'a', 'l', 'p', 'h', 'a', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 0x00E4, 0x4E00, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "alpha-plane1",
                   (mulle_utf32_t[]) { '[', '[', ':', 'a', 'l', 'p', 'h', 'a', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x1D400, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "alpha-emoji",
                   (mulle_utf32_t[]) { '[', '[', ':', 'a', 'l', 'p', 'h', 'a', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x1F600, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "space-emoji",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x1F600, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "space-nbsp",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x00A0, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "digit",
                   (mulle_utf32_t[]) { '[', '[', ':', 'd', 'i', 'g', 'i', 't', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'x', '1', '2', 'y', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "digit-arabic",
                   (mulle_utf32_t[]) { '[', '[', ':', 'd', 'i', 'g', 'i', 't', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x0660, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "never-space",
                   (mulle_utf32_t[]) { '[', '^', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'b', ' ', 'c', 'd', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "alpha-and-literal",
                   (mulle_utf32_t[]) { '[', '[', ':', 'a', 'l', 'p', 'h', 'a', ':', ']', '_', '-', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', '_', '-', 'b', '!', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "two-named",
                   (mulle_utf32_t[]) { '[', '[', ':', 'd', 'i', 'g', 'i', 't', ':', ']', '[', ':', 'p', 'u', 'n', 'c', 't', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { '1', '!', '?', 'a', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "ident",
                   (mulle_utf32_t[]) { '[', '[', ':', 'i', 'd', 'e', 'n', 't', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', '1', '_', 'b', '!', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "ident-umlaut",
                   (mulle_utf32_t[]) { '[', '[', ':', 'i', 'd', 'e', 'n', 't', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 0x00E4, 0x4E00, 0, 0 });

   // [:word:] is alphanumeric plus underscore, so it takes the superscript
   // two, which is not an identifier
   errors += test( MULLE_UTF32REGEX_EXTENDED, "word-superscript",
                   (mulle_utf32_t[]) { '[', '[', ':', 'w', 'o', 'r', 'd', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x00B2, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "ident-superscript",
                   (mulle_utf32_t[]) { '[', '[', ':', 'i', 'd', 'e', 'n', 't', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 0x00B2, 0, 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "xdigit",
                   (mulle_utf32_t[]) { '[', '[', ':', 'x', 'd', 'i', 'g', 'i', 't', ':', ']', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'F', '9', 'z', 0 });

   // a plain class has to keep working with the option on
   errors += test( MULLE_UTF32REGEX_EXTENDED, "plain-range",
                   (mulle_utf32_t[]) { '[', '0', '-', '9', ']', '+', 0 },
                   (mulle_utf32_t[]) { '1', '2', 'a', 0 });

   //
   // A plain negated class is an ANYBUT node, which is a *different* code path
   // from the [^[:space:]] of never-space above: a named class compiles to
   // ANYCLASS and answers out of a bitmap, while a plain one is answered by
   // mulle_utf32_strcspn(). Nothing here covered that path, and a
   // single-member set is where it went wrong - it computed the strspn answer,
   // so [^a]+ accepted "aaa" and rejected "bc".
   //
   // See BUG_REPORT.md, "mulle_utf32_strcspn() computes strspn".
   //
   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a",
                   (mulle_utf32_t[]) { '[', '^', 'a', ']', 0 },
                   (mulle_utf32_t[]) { 'b', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a-none",
                   (mulle_utf32_t[]) { '[', '^', 'a', ']', 0 },
                   (mulle_utf32_t[]) { 'a', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a-plus",
                   (mulle_utf32_t[]) { '[', '^', 'a', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'b', 'c', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a-plus-none",
                   (mulle_utf32_t[]) { '[', '^', 'a', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'a', 'a', 0 });

   // the leftmost match is not at the start here
   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a-plus-later",
                   (mulle_utf32_t[]) { '[', '^', 'a', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'b', 'c', 0 });

   // two members and a range take the general path, so these are the control
   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-ab-plus",
                   (mulle_utf32_t[]) { '[', '^', 'a', 'b', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'c', 'd', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-range-plus",
                   (mulle_utf32_t[]) { '[', '^', 'a', '-', 'c', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'x', 'y', 'z', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-range-none",
                   (mulle_utf32_t[]) { '[', '^', 'a', '-', 'c', ']', 0 },
                   (mulle_utf32_t[]) { 'b', 0 });

   // the shape a comment stripper needs: everything up to a hash
   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-hash-plus",
                   (mulle_utf32_t[]) { '[', '^', '#', ']', '+', 0 },
                   (mulle_utf32_t[]) { 'a', 'b', 'c', '#', 'd', 'e', 'f', 0 });

   // and the old spelling of a class is still a class
   errors += test( MULLE_UTF32REGEX_EXTENDED, "not-a-named-class",
                   (mulle_utf32_t[]) { '[', '[', '.', 'x', '.', ']', ']', 0 },
                   (mulle_utf32_t[]) { 'x', 0 });

   // the escapes are a class thing, there are no new atoms
   errors += test( MULLE_UTF32REGEX_EXTENDED, "atom-escape-fails",
                   (mulle_utf32_t[]) { 'a', '\\', 't', 'b', 0 },
                   (mulle_utf32_t[]) { 'a', 't', 'b', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "perl-escape-fails",
                   (mulle_utf32_t[]) { '\\', 'd', '+', 0 },
                   (mulle_utf32_t[]) { '5', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "unknown-class-fails",
                   (mulle_utf32_t[]) { '[', '[', ':', 'n', 'o', 's', 'u', 'c', 'h', ':', ']', ']', 0 },
                   (mulle_utf32_t[]) { 'x', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "unterminated-class-fails",
                   (mulle_utf32_t[]) { '[', '[', ':', 's', 'p', 'a', 'c', 'e', ':', ']', 0 },
                   (mulle_utf32_t[]) { ' ', 0 });

   errors += test( MULLE_UTF32REGEX_EXTENDED, "trailing-escape-fails",
                   (mulle_utf32_t[]) { '[', 'a', '\\', 0 },
                   (mulle_utf32_t[]) { 'a', 0 });

   return( errors ? 1 : 0);
}
