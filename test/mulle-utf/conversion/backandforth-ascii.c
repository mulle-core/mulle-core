#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>



struct bytes_buffer
{
   union
   {
      mulle_utf32_t   _32[ 32];
      mulle_utf16_t   _16[ 64];
      char            _8[ 128];
   } text;
   size_t             n;   // n bytes
};


static void  buffer_add( struct bytes_buffer *p, void *bytes, size_t len)
{
   memmove( &p->text._8[ p->n], bytes, len);
   p->n += len;
}


static void   test_buffer_conversion( char *text, int n)
{
   struct bytes_buffer   buffer32 = { 0 };
   struct bytes_buffer   buffer16 = { 0 };
   struct bytes_buffer   buffer8  = { 0 };

   assert( n < 32);

   printf( "ASCII: %.*s\n", (int) n, text);

   mulle_ascii_bufferconvert_to_utf16( text, n, &buffer16, (void *) buffer_add);
   mulle_utf16_bufferconvert_to_utf8( buffer16.text._16, buffer16.n / sizeof( mulle_utf16_t), &buffer8, (void *) buffer_add);
   printf( "UTF16: %.*s\n", (int) buffer8.n, buffer8.text._8);

   mulle_ascii_bufferconvert_to_utf32( text, n, &buffer16, (void *) buffer_add);
   mulle_utf32_bufferconvert_to_utf16( buffer32.text._32, buffer32.n / sizeof( mulle_utf32_t), &buffer8, (void *) buffer_add);
   printf( "UTF32: %.*s\n", (int) buffer8.n, buffer8.text._8);
}



struct unit_buffer
{
   union
   {
      mulle_utf32_t   _32[ 32];
      mulle_utf16_t   _16[ 64];
      char            _8[ 128];
   } text;
   size_t             n;   // n units
};


static void   test_direct_conversion( char *text, int n)
{
   struct unit_buffer   buffer32 = { 0 };
   struct unit_buffer   buffer16 = { 0 };
   struct unit_buffer   buffer8  = { 0 };

   assert( n < 32);

   if( ! mulle_utf8_is_ascii( text, n))
      printf( "Fail ASCII\n");

   printf( "ASCII: %.*s\n", (int) n, text);

   buffer16.n = _mulle_ascii_convert_to_utf16( text, n, buffer16.text._16) - buffer16.text._16;
   buffer8.n  = _mulle_utf16_convert_to_utf8( buffer16.text._16, buffer16.n, buffer8.text._8) - buffer8.text._8;
   printf( "UTF16: %.*s\n", (int) buffer8.n, buffer8.text._8);

   buffer32.n = _mulle_ascii_convert_to_utf32( text, n, buffer32.text._32) - buffer32.text._32;
   buffer8.n  = _mulle_utf32_convert_to_utf8( buffer32.text._32, buffer32.n, buffer8.text._8) - buffer8.text._8;
   printf( "UTF32: %.*s\n", (int) buffer8.n, buffer8.text._8);
}



int  main()
{
   char   *ascii  = "VfL Bochum 1848\n";

   // for coverage
   assert( ! mulle_utf8_is_ascii( NULL, 0));
   assert( mulle_utf8_is_ascii( "", -1));
   assert( ! mulle_utf8_is_ascii( "\x90", 1));

   test_direct_conversion( ascii, strlen( ascii));

   test_buffer_conversion( ascii, strlen( ascii));
   test_buffer_conversion( ascii, -1);

   return( 0);
}

