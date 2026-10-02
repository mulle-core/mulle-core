//
// U+E000 is the first character of the Private Use Area. It is NOT a
// surrogate. mulle_utf32_is_surrogatecharacter() uses an inclusive upper
// bound (c <= 0xE000) and therefore misclassifies it.
//
// Every caller of mulle_utf32_is_surrogatecharacter() inherits the error:
// mulle_utf16_validate(), mulle_utf16_information() and mulle_utf16_utf32length()
// all mishandle a string containing U+E000.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static int   fails;


static void   expect_int( char *description, int actual, int expected)
{
   if( actual == expected)
   {
      printf( "PASS: %s\n", description);
      return;
   }
   printf( "FAIL: %s (expected %d, got %d)\n", description, expected, actual);
   ++fails;
}


int   main( void)
{
   mulle_utf16_t                  pua[]  = { 'A', 0xE000, 'B' };
   mulle_utf16_t                  lo[]   = { 'A', 0xDFFF, 'B' };
   struct mulle_utf_information   info;

   //
   // classification
   //
   expect_int( "U+D800 is a surrogate",
               mulle_utf32_is_surrogatecharacter( 0xD800), 1);
   expect_int( "U+DFFF is a surrogate",
               mulle_utf32_is_surrogatecharacter( 0xDFFF), 1);
   expect_int( "U+E000 is not a surrogate",
               mulle_utf32_is_surrogatecharacter( 0xE000), 0);
   expect_int( "U+E000 is not a low surrogate",
               mulle_utf32_is_lowsurrogatecharacter( 0xE000), 0);

   //
   // a lone low surrogate really is invalid, so this must stay rejected
   //
   expect_int( "lone low surrogate is invalid UTF16",
               mulle_utf16_information( lo, 3, &info), -1);

   //
   // consequences for U+E000
   //
   expect_int( "mulle_utf16_information accepts U+E000",
               mulle_utf16_information( pua, 3, &info), 0);
   expect_int( "mulle_utf16_validate accepts U+E000",
               mulle_utf16_validate( pua, 3) == NULL, 1);
   expect_int( "mulle_utf16_utf32length counts U+E000 as one character",
               (int) mulle_utf16_utf32length( pua, 3), 3);
   expect_int( "mulle_utf16_utf8length of \"A<U+E000>B\"",
               (int) mulle_utf16_utf8length( pua, 3), 5);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}
