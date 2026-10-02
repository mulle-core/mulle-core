#include <mulle-utf/mulle-utf.h>

#include <stdio.h>


int  main()
{
   if( mulle_utf8_strnlen( NULL, 0) != 0)
     printf( "FAIL");

   if( mulle_utf8_strnlen( "ABC", 3) != 3)
     printf( "FAIL");

   if( mulle_utf8_strnlen( "ABC", 4) != 3)
     printf( "FAIL");

   return( 0);
}

