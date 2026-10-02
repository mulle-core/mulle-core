#include <mulle-vararg/mulle-vararg.h>

#include <stdio.h>


int   main( void)
{
   mulle_vararg_builderbuffer_t  buf[ mulle_vararg_builderbuffer_n( sizeof( int) + sizeof( long))];
   mulle_vararg_list             list;
   mulle_vararg_list             q;

   list = mulle_vararg_list_make( buf);

   mulle_vararg_copy( q, list);
   mulle_vararg_push_int( q, 18);
   mulle_vararg_push_long( q, 48L);

   printf( "%d\n",  mulle_vararg_next_integer( list, int));
   printf( "%ld\n", mulle_vararg_next_integer( list, long));

   return( 0);
}
