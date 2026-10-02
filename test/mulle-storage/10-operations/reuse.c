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

   for( i = 0; i < 10; i++)
   {
      value.a = i + 1848;
      value.b = (double) i * 0.5;
      saved = _mulle_storage_copy( &store, &value);
      mulle_pointerarray_add( &check, saved);
   }

   // free every second one
   for( i = 0; i < 10; i += 2)
   {
      saved = mulle_pointerarray_get( &check, i);
      _mulle_storage_free( &store, saved);
   }

   for( i = 0; i < 10; i += 2)
   {
      value.a = i + 1848;
      value.b = (double) i * 0.5;
      saved   = _mulle_storage_copy( &store, &value);
      // should all be reused addresses
      if( mulle_pointerarray_find( &check, saved) == mulle_not_found_e)
         abort();
   }
   _mulle_storage_done( &store);
   mulle_pointerarray_done( &check);
   return( 0);
}