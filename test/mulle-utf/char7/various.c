#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>



static void   test_is_char7string( char *s)
{
   int  result;
   int  result2;

   if( mulle_char7_get_maxlength() == mulle_char7_maxlength32)
      result = mulle_char7_is_char7string32( s, -1);
   else
      result = mulle_char7_is_char7string64( s, -1);

   result2 = mulle_char7_is_char7string( s, -1);

   printf( "mulle_char7_is_char7string( %s) : %s\n",
      s ? s : "NULL",
      (result == result2) ? "OK" : "FAIL");
}


static void   test_is_char7string32( char *s)
{
   printf( "mulle_char7_is_char7string32( %s)=%s\n",
      s ? s : "NULL",
      mulle_char7_is_char7string32( s, -1) ? "YES" : "NO");
}


static void   test_is_char7string64( char *s)
{
   printf( "mulle_char7_is_char7string64( %s)=%s\n",
      s ? s : "NULL",
      mulle_char7_is_char7string32( s, -1) ? "YES" : "NO");
}



static void   test( char *s)
{
   test_is_char7string( s);
   test_is_char7string32( s);
   test_is_char7string64( s);
}


int  main()
{
   char   *s1 = "VfL Bochum 1848";
   char   *s2 = "VfL";

   test( NULL);
   test( s1);
   test( s2);

   return( 0);
}

