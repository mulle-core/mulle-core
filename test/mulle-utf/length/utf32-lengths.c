//
// mulle_utf32_utf16length() tests `x >= 0xFFFF` where it must test
// `x >= 0x10000`. U+FFFF is a BMP code point and occupies a single UTF16
// unit, so the reported length is one too large for any string containing it.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static int   fails;


static void   check( char *description,
                     mulle_utf32_t *s,
                     size_t len,
                     size_t expected_utf16len,
                     size_t expected_utf8len)
{
   size_t   utf16len;
   size_t   utf8len;
   int      ok;

   utf16len = mulle_utf32_utf16length( s, len);
   utf8len  = mulle_utf32_utf8length( s, len);
   ok       = 1;

   if( utf16len != expected_utf16len)
   {
      printf( "FAIL: %s utf16length (expected %zu, got %zu)\n",
              description, expected_utf16len, utf16len);
      ok = 0;
   }
   if( utf8len != expected_utf8len)
   {
      printf( "FAIL: %s utf8length (expected %zu, got %zu)\n",
              description, expected_utf8len, utf8len);
      ok = 0;
   }

   if( ok)
      printf( "PASS: %s\n", description);
   else
      ++fails;
}


int   main( void)
{
   mulle_utf32_t   ascii[]  = { 'h', 'i' };
   mulle_utf32_t   latin[]  = { 0x00E9 };
   mulle_utf32_t   bmp[]    = { 0x20AC };
   mulle_utf32_t   fffd[]   = { 0xFFFD };   // replacement character
   mulle_utf32_t   fffe[]   = { 0xFFFE };   // noncharacter, still one unit
   mulle_utf32_t   ffff[]   = { 0xFFFF };   // noncharacter, still one unit
   mulle_utf32_t   astral[] = { 0x10000 };  // first astral, two units
   mulle_utf32_t   grin[]   = { 0x1F600 };
   mulle_utf32_t   maxcp[]  = { 0x10FFFD };

   check( "ascii \"hi\"",  ascii,  2, 2, 2);
   check( "U+00E9",        latin,  1, 1, 2);
   check( "U+20AC",        bmp,    1, 1, 3);
   check( "U+FFFD",        fffd,   1, 1, 3);
   check( "U+FFFE",        fffe,   1, 1, 3);
   check( "U+FFFF",        ffff,   1, 1, 3);
   check( "U+10000",       astral, 1, 2, 4);
   check( "U+1F600",       grin,   1, 2, 4);
   check( "U+10FFFD",      maxcp,  1, 2, 4);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}
