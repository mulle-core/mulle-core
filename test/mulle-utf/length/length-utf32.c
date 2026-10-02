#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>


static mulle_utf32_t  text_with_emoji[] =
{
   0x0000003c, 0x0001f3b2, 0x0000003e, 0
};

static mulle_utf32_t  text_with_surrogate[] =
{
   0x0000003c, 0x0001F990, 0x0000003e, 0
};


static void   test( mulle_utf32_t *s)
{
   printf( "utf8  length: %zu\n", mulle_utf32_utf8length( s, -1));
   printf( "utf16 length: %zu\n", mulle_utf32_utf16length( s, -1));
   printf( "utf32 length: %zu\n", mulle_utf32_strlen( s));
}


int  main()
{
   test( text_with_emoji);
   test( text_with_surrogate);
   return( 0);
}

