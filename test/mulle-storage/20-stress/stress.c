#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int     a;
   double  b;
};



int  main( int argc, char *argv[])
{
   struct mulle_storage   store;
   struct mulle_pointerarray  check;
   struct x                   value;
   struct x                   *saved;
   unsigned int               i;
   unsigned int               j;
   unsigned int               n;

   mulle_pointerarray_init( &check, 0, NULL);
   _mulle_storage_init( &store,
                            sizeof( struct x),
                            alignof( struct x),
                            4,
                            NULL);

   for( i = 0; i < 100000; i++)
   {
      n = mulle_pointerarray_get_count( &check);
      if( n && (i % 3 == 0))
      {
         j = rand() % n;
         saved = mulle_pointerarray_get( &check, j);
         mulle_pointerarray_remove_in_range( &check, mulle_range_make( j, 1));
         _mulle_storage_free( &store, saved);
         continue;
      }

      value.a = i + 1848;
      value.b = (double) i * 0.5;
      saved   = _mulle_storage_copy( &store, &value);
      mulle_pointerarray_add( &check, saved);
   }

   _mulle_storage_done( &store);

   mulle_pointerarray_done( &check);
   return( 0);
}