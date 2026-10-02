//
// mulle_utf16_information() computes utf8len as
//
//    utf8len++             for every non-ASCII unit
//    utf8len++             for every unit >= 0x800   (a surrogate is >= 0x800)
//    utf8len++             once more for a surrogate pair
//    utf8len += src - start   (once per UTF16 unit)
//
// A surrogate pair is 2 units, so it contributes 1 + 1 + 1 + 2 == 5, but a
// character outside the BMP needs only 4 UTF8 bytes. utf8len is therefore
// one byte too large per astral character and disagrees with
// mulle_utf16_utf8length(), which gets it right.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static int   fails;


static void   check( char *description,
                     mulle_utf16_t *s,
                     size_t len,
                     size_t expected_utf8len,
                     size_t expected_utf16len,
                     size_t expected_utf32len)
{
   struct mulle_utf_information   info;
   int                            ok;

   if( mulle_utf16_information( s, len, &info))
   {
      printf( "FAIL: %s (rejected as invalid)\n", description);
      ++fails;
      return;
   }

   ok = 1;
   if( info.utf8len != expected_utf8len)
   {
      printf( "FAIL: %s utf8len (expected %zu, got %zu)\n",
              description, expected_utf8len, info.utf8len);
      ok = 0;
   }
   if( info.utf16len != expected_utf16len)
   {
      printf( "FAIL: %s utf16len (expected %zu, got %zu)\n",
              description, expected_utf16len, info.utf16len);
      ok = 0;
   }
   if( info.utf32len != expected_utf32len)
   {
      printf( "FAIL: %s utf32len (expected %zu, got %zu)\n",
              description, expected_utf32len, info.utf32len);
      ok = 0;
   }
   if( info.utf8len != mulle_utf16_utf8length( s, len))
   {
      printf( "FAIL: %s utf8len disagrees with mulle_utf16_utf8length (%zu)\n",
              description, mulle_utf16_utf8length( s, len));
      ok = 0;
   }

   if( ok)
      printf( "PASS: %s\n", description);
   else
      ++fails;
}


int   main( void)
{
   mulle_utf16_t   ascii[]  = { 'h', 'i' };
   mulle_utf16_t   latin[]  = { 0x00E9 };                 // e acute, 2 utf8 bytes
   mulle_utf16_t   bmp[]    = { 0x20AC };                 // euro,    3 utf8 bytes
   mulle_utf16_t   grin[]   = { 0xD83D, 0xDE00 };         // U+1F600, 4 utf8 bytes
   mulle_utf16_t   mixed[]  = { 'h', 'i', 0xD83D, 0xDE00 };

   check( "ascii \"hi\"",           ascii, 2, 2, 2, 2);
   check( "latin1 U+00E9",          latin, 1, 2, 1, 1);
   check( "bmp U+20AC",             bmp,   1, 3, 1, 1);
   check( "astral U+1F600",         grin,  2, 4, 2, 1);
   check( "\"hi\" plus U+1F600",    mixed, 4, 6, 4, 3);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}
