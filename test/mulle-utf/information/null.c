#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static void   test_null()
{
   struct mulle_utf_information   info8;
   struct mulle_utf_information   info16;
   struct mulle_utf_information   info32;
   static mulle_utf32_t            empty[ 1];

   mulle_utf8_information( (char *) empty, 0, &info8);
   mulle_utf16_information( (mulle_utf16_t *) empty, 0, &info16);
   mulle_utf32_information( (mulle_utf32_t *) empty, 0, &info32);

   if( info8.start != empty || info16.start != empty || info32.start != empty)
   {
      printf( "failed\n");
      return;
   }

   printf( "passed\n");
}



int  main()
{
   test_null();
   return( 0);
}

