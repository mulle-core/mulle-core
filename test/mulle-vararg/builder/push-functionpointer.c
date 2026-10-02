//
// Test: mulle_vararg_push_functionpointer and reading it back
// (function pointers are read with mulle_vararg_next_pointer)
//
#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


static int   add( int a, int b)
{
   return( a + b);
}


int   main( void)
{
   mulle_vararg_builderbuffer_t   buf[ mulle_vararg_builderbuffer_n( mulle_vararg_sizeof_functionpointer( void (*)( void)))];
   mulle_vararg_list              list;
   mulle_vararg_list              q;
   int (*fp)( int, int);
   int (*r)( int, int);

   fp = add;

   list = mulle_vararg_list_make( buf);
   mulle_vararg_copy( q, list);

   mulle_vararg_push_functionpointer( q, (void (*)( void)) fp);

   r = (int (*)( int, int)) mulle_vararg_next_pointer( list, void (*)( void));

   if( r != fp || r( 18, 30) != 48)
   {
      printf( "FAIL: function pointer round-trip\n");
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
