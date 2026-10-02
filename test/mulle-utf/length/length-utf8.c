#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>

static char  *text_with_emoji = "\x3c\xf0\x9f\x8e\xb2\x3e";
static char  *text_with_euro = "\x41\xe2\x82\xac\x73\x69\x67\x6e";

static void   test( char *s)
{
   printf( "utf8  length: %zu\n", mulle_utf8_strlen( s));
   printf( "utf16 length: %zu\n", mulle_utf8_utf16length( s, -1));
   printf( "utf32 length: %zu\n", mulle_utf8_utf32length( s, -1));
}


int  main()
{
   test( text_with_emoji);
   // surrogates are not valid in UTF8 though
   test( text_with_euro);
   return( 0);
}

