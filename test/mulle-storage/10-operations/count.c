#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int     a;
   double  b;
};


//
// Test that count accounting is correct through mixed alloc/free cycles
//
int  main( int argc, char *argv[])
{
   struct mulle_storage   store;
   struct x               *ptrs[10];
   unsigned int           i;

   _mulle_storage_init( &store,
                        sizeof( struct x),
                        alignof( struct x),
                        4,
                        NULL);

   // initially empty
   if( _mulle_storage_get_count( &store) != 0)
      abort();

   // allocate 10
   for( i = 0; i < 10; i++)
   {
      ptrs[ i] = _mulle_storage_malloc( &store);
      ptrs[ i]->a = i;
   }
   if( _mulle_storage_get_count( &store) != 10)
      abort();

   // free all 10
   for( i = 0; i < 10; i++)
      _mulle_storage_free( &store, ptrs[ i]);

   if( _mulle_storage_get_count( &store) != 0)
      abort();

   // reallocate 5, count should be 5
   for( i = 0; i < 5; i++)
      ptrs[ i] = _mulle_storage_malloc( &store);

   if( _mulle_storage_get_count( &store) != 5)
      abort();

   // free 3, count should be 2
   for( i = 0; i < 3; i++)
      _mulle_storage_free( &store, ptrs[ i]);

   if( _mulle_storage_get_count( &store) != 2)
      abort();

   // free remaining 2
   _mulle_storage_free( &store, ptrs[ 3]);
   _mulle_storage_free( &store, ptrs[ 4]);

   if( _mulle_storage_get_count( &store) != 0)
      abort();

   _mulle_storage_done( &store);
   return( 0);
}
