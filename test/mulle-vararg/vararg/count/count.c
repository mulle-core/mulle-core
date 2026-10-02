#include <mulle-vararg/mulle-vararg.h>
#include <stdio.h>


struct pointer_32_t
{
   void  *pointers[ 32];
};


static size_t   count( struct pointer_32_t *array)
{
   mulle_vararg_list   list;
   size_t              n;

   _mulle_vararg_start( list, array->pointers[ 0]);

   n = mulle_vararg_count_pointers( list, array->pointers[ 0]);

   mulle_vararg_end( list);
   return( n);
}


int main()
{
   struct pointer_32_t   array = { (void *) 0x1848, (void *) 0x1849, NULL };

   printf( "%ld\n", (long) count( &array));
   return( 0);
}
