#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static void   test( mulle_utf32_t original[ 4])
{
   char            *text8;
   mulle_utf16_t   *text16;
   mulle_utf32_t   *text32;

   text8  = mulle_utf32_convert_to_utf8_string( original, 4, NULL);
   text32 = mulle_utf8_convert_to_utf32_string( text8, -1, NULL);
   if( mulle_utf32_strncmp( text32, original, 4))
   {
      printf( "FAIL1\n");
   }
   mulle_free( text8);
   mulle_free( text32);

   text16 = mulle_utf32_convert_to_utf16_string( original, 4, NULL);
   text32 = mulle_utf16_convert_to_utf32_string( text16, -1, NULL);
   if( mulle_utf32_strncmp( text32, original, 4))
   {
      printf( "FAIL2\n");
   }
   mulle_free( text16);
   mulle_free( text32);
}



// 3emos: UTF8   f0 9f 8c 91 f0 9f 9a 9a f0 9f 91 a0  (unbommed)
// 3emos: UTF16  ff fe 3c d8 11 df 3d d8  9a de 3d d8 60 dc (bommed)
// 3emos: UTF32  ff fe 00 00 11 f3 01 00  9a f6 01 00 60 f4 01 00

int  main()
{
   mulle_utf32_t   text_3emos[ 4]  = { 0x01f311, 0x01f69a, 0x1f460 };
   mulle_utf32_t   text_utf16[ 4]  = { 47177, 29938, 18497 };
   mulle_utf32_t   text_utf15[ 4]  = { 32313, 29938, 18497 };
   mulle_utf32_t   ascii_trailing_zero[ 4]  = { 'V', 'f', 'L', 0 };

   test( text_3emos);
   test( text_utf15);
   test( text_utf16);
   test( ascii_trailing_zero);

   return( 0);
}

