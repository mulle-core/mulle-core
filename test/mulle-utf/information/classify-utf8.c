#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>

static int   test( char *text, size_t len)
{
   struct mulle_utf_information   info;

   if( mulle_utf8_information( text, len, &info))
   {
      printf( "Offending character at position %ld (%u)\n",
                           (long) ((char *) info.invalid - text),
                           *(unsigned char *) info.invalid);
      return( 1);
   }
   return( 0);
}


int  main()
{
   int   fails;

   fails  = 0;
   fails += test( (char []) { 'I', 't', 226, 's' }, 4);
   return( 0);
}

