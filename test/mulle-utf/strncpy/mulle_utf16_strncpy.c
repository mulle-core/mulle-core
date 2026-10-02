#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   mulle_utf16_t   text[]  = { 'A', 'B', 'C', 0 };
   mulle_utf16_t   dst[16];
   mulle_utf16_t   *s;

   if( mulle_utf16_strncpy( NULL, 0, NULL) != NULL)
     printf( "FAIL");

   s = mulle_utf16_strncpy( dst, 16, text);
   if( s != dst)
     printf( "FAIL");
   if( mulle_utf16_strlen( s) != 3)
     printf( "FAIL");
   if( mulle_utf16_strcmp( s, text))
     printf( "FAIL");

   return( 0);
}

