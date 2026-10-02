#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   char   *s = "ABC";
   char   *s2 = "ABACAB";
   mulle_utf32_t   emos3_32[ 3]  = { 0x01f311, 0x01f69a, 0x1f460 };
   char   emos3[ 256];
   char   *end;

   end = _mulle_utf32_convert_to_utf8( emos3_32, 3, emos3);
   assert( end < &emos3[ sizeof( emos3)]);
   *end = 0;

   if( mulle_utf8_strstr( NULL, "a") != NULL)
     printf( "FAIL 1\n");

   if( mulle_utf8_strstr( s, "A") != s)
     printf( "FAIL 2\n");

   if( mulle_utf8_strstr( s, "B") != &s[ 1])
     printf( "FAIL 3\n");

   if( mulle_utf8_strstr( s, "C") != &s[ 2])
     printf( "FAIL 4\n");

   if( mulle_utf8_strstr( s, "AB") != &s[ 0])
     printf( "FAIL 5\n");

   if( mulle_utf8_strstr( s, "BC") != &s[ 1])
     printf( "FAIL 6\n");

   if( mulle_utf8_strstr( s, NULL) != NULL) // sic!
     printf( "FAIL 7\n");

   if( mulle_utf8_strstr( s, "") != s) // sic!
     printf( "FAIL 8\n");

   if( mulle_utf8_strstr( s, "D") != NULL)
     printf( "FAIL 9\n");

   if( mulle_utf8_strstr( s2, "AB") != &s2[ 0])
     printf( "FAIL 10\n");

   if( mulle_utf8_strstr( emos3, "B") != NULL)
     printf( "FAIL 11\n");

   if( mulle_utf8_strstr( emos3, emos3) != emos3)
     printf( "FAIL 12\n");

   return( 0);
}

