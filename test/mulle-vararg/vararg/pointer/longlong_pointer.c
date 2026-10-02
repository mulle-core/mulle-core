#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>


struct
{
   long long  a;
   void       *b;
} value =
{
   -1848,
   (void *) 1848
};


int   main( void)
{
   mulle_vararg_list   list;

   _mulle_vararg_start( list, value.a);

   printf( "%ld\n", (long) (intptr_t) mulle_vararg_next_pointer( list, void *));

   mulle_vararg_end( list);
   return( 0);
}
