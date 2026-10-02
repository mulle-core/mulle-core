#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>


struct
{
   void  *a;
   char   b;
} value =
{
   (void *) -1848,
   18
};


int   main( void)
{
   mulle_vararg_list   list;

   _mulle_vararg_start( list, value.a);

   printf( "%d\n", mulle_vararg_next_integer( list, char));

   mulle_vararg_end( list);
   return( 0);
}
