#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static mulle_utf32_t   random_char( mulle_utf32_t mask)
{
   mulle_utf32_t  c;

   do
   {
      c = rand() % mulle_utf32_max + 1;
      c &= mask;
      if( ! c)
         continue;
   }
   while( mulle_utf32_is_bomcharacter( c) || \
          mulle_utf32_is_surrogatecharacter( c) || \
          mulle_utf32_is_noncharacter( c) || \
          (mulle_utf_is_privatecharacterplane( mulle_utf32_get_unicodeplane( c)) &&  \
          mulle_utf32_is_privatecharacter( c)));

   return( c);
}



static void   random_text( mulle_utf32_t *text, size_t n, mulle_utf32_t mask)
{
   mulle_utf32_t   *sentinel;

   sentinel = &text[ n];
   while( text < sentinel)
      *text++ = random_char( mask);
}


struct buffer
{
   union
   {
      mulle_utf32_t   _32[ 8];
      mulle_utf16_t   _16[ 16];
      char    _8[ 32];
   } text;
   size_t             n;
};


static void  buffer_add( struct buffer *p, void *bytes, size_t len)
{
   memmove( &p->text._8[ p->n], bytes, len);
   p->n += len;
}


static void   test_prediction( mulle_utf32_t text[ 4])
{
   struct buffer   buffer32;
   struct buffer   buffer16;
   struct buffer   buffer8;
   struct mulle_utf_information   info;

   // UTF32 <-> UTF8
   mulle_utf32_information( text,4, &info);

   memset( &buffer8, 0, sizeof( buffer8));
   mulle_utf32_bufferconvert_to_utf8( info.start, info.utf32len, &buffer8, (void *) buffer_add);
   if( buffer8.n != info.utf8len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }

   memset( &buffer32, 0, sizeof( buffer32));
   mulle_utf8_bufferconvert_to_utf32( buffer8.text._8, buffer8.n, &buffer32, (void *) buffer_add);
   if( buffer32.n / sizeof( mulle_utf32_t) != info.utf32len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }


   // UTF32 <-> UTF16
   memset( &buffer16, 0, sizeof( buffer16));
   mulle_utf32_bufferconvert_to_utf16( info.start, info.utf32len, &buffer16, (void *) buffer_add);
   if( buffer16.n / sizeof( mulle_utf16_t) != info.utf16len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }

   memset( &buffer32, 0, sizeof( buffer32));
   mulle_utf16_bufferconvert_to_utf32( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer32, (void *) buffer_add);
   if( buffer32.n / sizeof( mulle_utf32_t) != info.utf32len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }

   // UTF16 <-> UTF8
   memset( &buffer8, 0, sizeof( buffer8));
   mulle_utf16_bufferconvert_to_utf8( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer8, (void *) buffer_add);
   if( buffer8.n != info.utf8len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }

   memset( &buffer16, 0, sizeof( buffer16));
   mulle_utf8_bufferconvert_to_utf16( buffer8.text._8, buffer8.n, &buffer16, (void *) buffer_add);
   if( buffer16.n / sizeof( mulle_utf16_t) != info.utf16len)
   {
      printf( "failed with %ls\n", (wchar_t *) text);
      return;
   }
}


static void   test_conversion( mulle_utf32_t text[ 4])
{
   struct buffer   buffer32;
   struct buffer   buffer16;
   struct buffer   buffer8;


   memset( &buffer32, 0, sizeof( buffer32));
   memset( &buffer8, 0, sizeof( buffer8));

   mulle_utf32_bufferconvert_to_utf8( text, 4, &buffer8, (void *) buffer_add);
   mulle_utf8_bufferconvert_to_utf32( buffer8.text._8, buffer8.n, &buffer32, (void *) buffer_add);


   // buffer.n is bytes!
   if( buffer32.n != 16 || memcmp( text, buffer32.text._32, 4))
   {
      printf( "utf32 to utf8 and back failed with %ls\n", (wchar_t *) text);
      abort();
   }

   memset( &buffer32, 0, sizeof( buffer32));
   memset( &buffer16, 0, sizeof( buffer16));

   mulle_utf32_bufferconvert_to_utf16( text, 4, &buffer16, (void *) buffer_add);
   mulle_utf16_bufferconvert_to_utf32( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer32, (void *) buffer_add);

   // buffer.n is bytes!
   if( buffer32.n != 16 || memcmp( text, buffer32.text._32, 4))
   {
      printf( "utf32 to utf16 failed with %ls\n", (wchar_t *) text);
      abort();
   }

   memset( &buffer32, 0, sizeof( buffer32));
   memset( &buffer16, 0, sizeof( buffer16));
   memset( &buffer8, 0, sizeof( buffer8));

   mulle_utf32_bufferconvert_to_utf16( text, 4, &buffer16, (void *) buffer_add);
   mulle_utf16_bufferconvert_to_utf8( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer8, (void *) buffer_add);

   memset( &buffer16, 0, sizeof( buffer16));
   mulle_utf8_bufferconvert_to_utf16( buffer8.text._8, buffer8.n, &buffer16, (void *) buffer_add);
   mulle_utf16_bufferconvert_to_utf32( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer32, (void *) buffer_add);

   // buffer.n is bytes!
   if( buffer32.n != 16 || memcmp( text, buffer32.text._32, 4))
   {
      printf( "utf32 to utf16 to utf8 to utf16 failed with %S\n", (wchar_t *) text);
      abort();
   }
}


static void  test( mulle_utf32_t text[ 4])
{
   test_prediction( text);
   test_conversion( text);
}


static void   stress_test()
{
   mulle_utf32_t   text[ 4];
   size_t    i;
   mulle_utf32_t   mask;

   mask = 0;
   for( i = 0; i < 1000000; i++)
   {
      switch( rand() % 5)
      {
      case 0 : mask = 0x7F; break;
      case 1 : mask = 0xFF; break;
      case 2 : mask = 0x7FFF; break;
      case 3 : mask = 0xFFFF; break;
      case 4 : mask = 0x1FFFFF; break;
      }

      random_text( text, 4, mask);

      test( text);
   }
}

// 3emos: UTF8   f0 9f 8c 91 f0 9f 9a 9a f0 9f 91 a0  (unbommed)
// 3emos: UTF16  ff fe 3c d8 11 df 3d d8  9a de 3d d8 60 dc (bommed)
// 3emos: UTF32  ff fe 00 00 11 f3 01 00  9a f6 01 00 60 f4 01 00

int  main()
{
   mulle_utf32_t   bom_3emos[ 4]  = { 0xfeff, 0x01f311, 0x01f69a, 0x1f460 };
   mulle_utf32_t   bom_utf16[ 4]  = { 65279, 47177, 29938, 18497 };
   mulle_utf32_t   bom_utf15[ 4]  = { 65279, 32313, 29938, 18497 };
   mulle_utf32_t   ascii_trailing_zero[ 4]  = { 'V', 'f', 'L', 0 };

   test( bom_3emos);
   test( bom_utf15);
   test( bom_utf16);
   test( ascii_trailing_zero);
   stress_test();

   return( 0);
}

