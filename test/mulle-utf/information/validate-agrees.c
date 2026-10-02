//
// mulle_utf*_validate() and mulle_utf*_information() implement two different
// notions of "valid". For any given input they should agree, but:
//
//   embedded NUL      information() treats it as a terminator and succeeds,
//                     validate() returns it as the offending character
//
//   noncharacters     information() uses mulle_utf32_is_noncharacter(), which
//                     covers U+FDD0..U+FDEF and the last two code points of
//                     *every* plane, while mulle_utf32_validate() uses a
//                     file local helper that only knows U+FFFE and U+FFFF.
//                     So U+1FFFE is rejected by one and accepted by the other.
//
// This test does not take a position on which notion is right. It only
// requires the two functions to give the same verdict, because callers use
// them interchangeably.
//
#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


static int   fails;


static void   check_utf8( char *description, char *s, size_t len)
{
   struct mulle_utf_information   info;
   int                            informs;
   int                            validates;

   informs   = mulle_utf8_information( s, len, &info) == 0;
   validates = mulle_utf8_validate( s, len) == NULL;

   if( informs == validates)
   {
      printf( "PASS: utf8 %s (both say %s)\n",
              description, informs ? "valid" : "invalid");
      return;
   }
   printf( "FAIL: utf8 %s (information says %s, validate says %s)\n",
           description,
           informs   ? "valid" : "invalid",
           validates ? "valid" : "invalid");
   ++fails;
}


static void   check_utf32( char *description, mulle_utf32_t *s, size_t len)
{
   struct mulle_utf_information   info;
   int                            informs;
   int                            validates;

   informs   = mulle_utf32_information( s, len, &info) == 0;
   validates = mulle_utf32_validate( s, len) == NULL;

   if( informs == validates)
   {
      printf( "PASS: utf32 %s (both say %s)\n",
              description, informs ? "valid" : "invalid");
      return;
   }
   printf( "FAIL: utf32 %s (information says %s, validate says %s)\n",
           description,
           informs   ? "valid" : "invalid",
           validates ? "valid" : "invalid");
   ++fails;
}


static void   check_utf16( char *description, mulle_utf16_t *s, size_t len)
{
   struct mulle_utf_information   info;
   int                            informs;
   int                            validates;

   informs   = mulle_utf16_information( s, len, &info) == 0;
   validates = mulle_utf16_validate( s, len) == NULL;

   if( informs == validates)
   {
      printf( "PASS: utf16 %s (both say %s)\n",
              description, informs ? "valid" : "invalid");
      return;
   }
   printf( "FAIL: utf16 %s (information says %s, validate says %s)\n",
           description,
           informs   ? "valid" : "invalid",
           validates ? "valid" : "invalid");
   ++fails;
}


int   main( void)
{
   char            u8_plain[]  = { 'h', 'i' };
   char            u8_nul[]    = { 'h', 'a', 0, 'h', 'a' };
   char            u8_bad[]    = { 'h', (char) 0xC3 };

   mulle_utf16_t   u16_plain[] = { 'h', 'i' };
   mulle_utf16_t   u16_nul[]   = { 'h', 'a', 0, 'h', 'a' };

   mulle_utf32_t   u32_plain[] = { 'h', 'i' };
   mulle_utf32_t   u32_nul[]   = { 'h', 'a', 0, 'h', 'a' };
   mulle_utf32_t   u32_fffe[]  = { 0xFFFE };
   mulle_utf32_t   u32_1fffe[] = { 0x1FFFE };
   mulle_utf32_t   u32_10fffe[]= { 0x10FFFE };
   mulle_utf32_t   u32_fdd0[]  = { 0xFDD0 };
   mulle_utf32_t   u32_surr[]  = { 0xD800 };

   check_utf8( "plain ascii",           u8_plain, 2);
   check_utf8( "embedded NUL",          u8_nul,   5);
   check_utf8( "truncated sequence",    u8_bad,   2);

   check_utf16( "plain ascii",          u16_plain, 2);
   check_utf16( "embedded NUL",         u16_nul,   5);

   check_utf32( "plain ascii",          u32_plain,  2);
   check_utf32( "embedded NUL",         u32_nul,    5);
   check_utf32( "U+FFFE",               u32_fffe,   1);
   check_utf32( "U+1FFFE",              u32_1fffe,  1);
   check_utf32( "U+10FFFE",             u32_10fffe, 1);
   check_utf32( "U+FDD0",               u32_fdd0,   1);
   check_utf32( "lone surrogate U+D800",u32_surr,   1);

   if( ! fails)
      printf( "\nAll tests passed!\n");
   else
      printf( "\n%d test(s) failed!\n", fails);

   return( fails);
}
