#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int     a;
   double  b;
};


//
// Test NULL-safe wrappers don't crash
//
int  main( int argc, char *argv[])
{
   struct mulle_storage   store;
   struct x               *p;

   // NULL-safe wrappers should not crash
   mulle_storage_init( NULL, sizeof( struct x), alignof( struct x), 4, NULL);
   mulle_storage_done( NULL);
   mulle_storage_free( NULL, NULL);

   if( mulle_storage_malloc( NULL) != NULL)
      abort();
   if( mulle_storage_calloc( NULL) != NULL)
      abort();
   if( mulle_storage_copy( NULL, NULL) != NULL)
      abort();
   if( mulle_storage_get_count( NULL) != 0)
      abort();
   if( mulle_storage_get_allocator( NULL) != NULL)
      abort();

   // free with NULL pointer should not crash
   mulle_storage_init( &store, sizeof( struct x), alignof( struct x), 4, NULL);
   mulle_storage_free( &store, NULL);

   // calloc should zero-initialize
   p = _mulle_storage_calloc( &store);
   if( p->a != 0)
      abort();
   if( p->b != 0.0)
      abort();

   _mulle_storage_free( &store, p);
   mulle_storage_done( &store);
   return( 0);
}
