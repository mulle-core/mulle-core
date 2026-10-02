#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>



int  main()
{
   char            nextstep[ 256];
   int             n;
   size_t    m;
   char            buf[ 256 * 4] = { 0 };
   char            *end;
   char            *s;
   char            *t;
   int             i;
   size_t    off;

   for( n = 0, i = 32; i < 254; i++, n++)
      nextstep[ n] = i;

   end = _mulle_nextstep_convert_to_utf8( nextstep, n, buf);

   off = 32;
   s   = buf;
   for(;;)
   {
      m = 16;
      t = mulle_utf8_skiputf32( s, &m);
      if( m != 16)
         break;

      printf( "%02x: %.*s\n", (unsigned int) off, (int) (t - s), s);
      s    = t;
      off += 16;
   }

   if( end != s)
      printf( "%02x: %.*s\n", (unsigned int) off, (int) (end - s), s);

   return( 0);
}

