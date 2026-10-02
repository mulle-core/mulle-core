#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>

int   main( void)
{
   struct mulle_utf8_conversion_context   ctxt;
   static char  *text =
   {
      "L\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6orem ipsum dolor sit amet, consectetur "
      "adipisici elit, sed eiusmod "
   };
   char             buf[ 5];
   mulle_utf32_t    *s;

   ctxt.buf      = buf;
   ctxt.sentinel = &ctxt.buf[ 4];

   s = mulle_utf8_convert_to_utf32_string( text, -1, NULL);
   mulle_utf32_bufferconvert_to_utf8( s,
                                      4,
                                      &ctxt,
                                      mulle_utf8_conversion_context_add_bytes);
   *ctxt.buf = 0;

   printf( "%s\n", buf);
   mulle_free( s);

   return( 0);
}

