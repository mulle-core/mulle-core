#include <mulle-utf/mulle-utf.h>

#include <stdio.h>

int  main()
{
   mulle_utf16_t  s[ 4]  ={ 'A', 'B', 'C',  0 };
   mulle_utf16_t  s2[ 7] = { 'A', 'B', 'A', 'C', 'A','B', 0 };
   mulle_utf16_t  AB[ 4] = { 'A', 'B', 0 };
   mulle_utf16_t  BC[ 4] = { 'B', 'C', 0 };
   mulle_utf16_t  A[ 2] = { 'A', 0 };
   mulle_utf16_t  B[ 2] = { 'B', 0 };
   mulle_utf16_t  C[ 2] = { 'C', 0 };
   mulle_utf16_t  D[ 2] = { 'D', 0 };
   mulle_utf16_t  empty[ 1] = { 0 };
   mulle_utf32_t  emos3_32[ 3]  = { 0x01f311, 0x01f69a, 0x1f460 };
   mulle_utf16_t  emos3[ 256];
   mulle_utf16_t  *end;

   end = _mulle_utf32_convert_to_utf16( emos3_32, 3, emos3);
   *end = 0;

   if( mulle_utf16_strstr( NULL, A) != NULL)
     printf( "FAIL 1\n");

   if( mulle_utf16_strstr( s, A) != s)
     printf( "FAIL 2\n");

   if( mulle_utf16_strstr( s, B) != &s[ 1])
     printf( "FAIL 3\n");

   if( mulle_utf16_strstr( s, C) != &s[ 2])
     printf( "FAIL 4\n");

   if( mulle_utf16_strstr( s, AB) != &s[ 0])
     printf( "FAIL 5\n");

   if( mulle_utf16_strstr( s, BC) != &s[ 1])
     printf( "FAIL 6\n");

   if( mulle_utf16_strstr( s, NULL) != NULL) // sic!
     printf( "FAIL 7\n");

   if( mulle_utf16_strstr( s, empty) != s) // sic!
     printf( "FAIL 8\n");

   if( mulle_utf16_strstr( s, D) != NULL)
     printf( "FAIL 9\n");

   if( mulle_utf16_strstr( s2, AB) != &s2[ 0])
     printf( "FAIL 10\n");

   if( mulle_utf16_strstr( emos3, B) != NULL)
     printf( "FAIL 11\n");

   if( mulle_utf16_strstr( emos3, emos3) != emos3)
     printf( "FAIL 12\n");

   return( 0);
}

