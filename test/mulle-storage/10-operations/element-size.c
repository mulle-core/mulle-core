#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int     a;
   double  b;
};


//
// Test that element_size is reported correctly
//
int  main( int argc, char *argv[])
{
   struct mulle_storage          store;
   struct mulle_indexedstorage   indexed;

   _mulle_storage_init( &store,
                        sizeof( struct x),
                        alignof( struct x),
                        4,
                        NULL);

   if( _mulle_storage_get_element_size( &store) != sizeof( struct x))
      abort();

   _mulle_storage_done( &store);

   _mulle_indexedstorage_init( &indexed,
                               sizeof( struct x),
                               alignof( struct x),
                               4,
                               NULL);

   if( _mulle_indexedstorage_get_element_size( &indexed) != sizeof( struct x))
      abort();

   _mulle_indexedstorage_done( &indexed);
   return( 0);
}
