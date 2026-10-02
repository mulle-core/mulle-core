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

   mulle_pointerarray_init( &check, 0, NULL);
   _mulle_storage_init( &store,
                            sizeof( struct x),
                            alignof( struct x),
                            4,
                            NULL);

   for( i = 0; i < 1000; i++)
   {
      value.a = i + 1848;
      value.b = (double) i * 0.5;
      saved = _mulle_storage_copy( &store, &value);
      mulle_pointerarray_add( &check, saved);
   }

   for( i = 0; i < 1000; i++)
   {
      saved = mulle_pointerarray_get( &check, i);
      _mulle_storage_free( &store, saved);
   }

   _mulle_storage_done( &store);

   mulle_pointerarray_done( &check);
   return( 0);
}