#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   mulle_utf32_t   text[] = { 'A', 'B', 'C', 0 };

   if( mulle_utf32_strnlen( NULL, 0) != 0)
     printf( "FAIL");

   if( mulle_utf32_strnlen( text, 3) != 3)
     printf( "FAIL");

   if( mulle_utf32_strnlen( text, 4) != 3)
     printf( "FAIL");

   return( 0);
}

