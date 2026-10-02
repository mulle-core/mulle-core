#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   mulle_utf16_t   s[ 4]  = { 'A', 'B', 'C' , 0 };
   mulle_utf16_t   s2[ 7] = { 'A', 'B', 'A', 'C', 'A', 'B', 0 };
   mulle_utf32_t   emos3_32[ 3]  = { 0x01f311, 0x01f69a, 0x1f460 };
   mulle_utf16_t   emos3[ 256];
   mulle_utf16_t   *end;

   end = _mulle_utf32_convert_to_utf16( emos3_32, 3, emos3);
   assert( end < &emos3[ 256]);
   *end = 0;

   if( mulle_utf16_strchr( NULL, 'a') != NULL)
     printf( "FAIL 1\n");

   if( mulle_utf16_strchr( s, 'A') != s)
     printf( "FAIL 2\n");

   if( mulle_utf16_strchr( s, 'B') != &s[ 1])
     printf( "FAIL 3\n");

   if( mulle_utf16_strchr( s, 'C') != &s[ 2])
     printf( "FAIL 4\n");

   if( mulle_utf16_strchr( s, 'D') != NULL)
     printf( "FAIL 5\n");

   if( mulle_utf16_strchr( s, 0) != &s[ 3])
     printf( "FAIL 6\n");

   if( mulle_utf16_strchr( s2, 'B') != &s2[ 1])
     printf( "FAIL 7\n");

   if( mulle_utf16_strchr( emos3, 'B') != NULL)
     printf( "FAIL 8\n");

   if( mulle_utf16_strchr( emos3, 0x01f69a) == NULL)
     printf( "FAIL 9\n");

   return( 0);
}

