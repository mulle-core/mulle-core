#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int   a;
   char  c;
};

int  main( int argc, char *argv[])
{
   struct mulle_storage   store;


   _mulle_storage_init( &store,
                            sizeof( struct x),
                            alignof( struct x),
                            1024,
                            NULL);

   _mulle_storage_done( &store);
   return( 0);
}