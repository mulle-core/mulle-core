#include <mulle-storage/mulle-storage.h>

#include <stdalign.h>


struct x
{
   int     a;
   double  b;
};

int  main( int argc, char *argv[])
{
   struct mulle_indexedstorage   store;
   struct x                      *p;
   unsigned int                  i;
   unsigned int                  index;

   _mulle_indexedstorage_init( &store,
                               sizeof( struct x),
                               alignof( struct x),
                               4,
                               NULL);

   // allocate 10 elements
   for( i = 0; i < 10; i++)
   {
      index = _mulle_indexedstorage_alloc( &store);
      p     = _mulle_indexedstorage_get( &store, index);
      p->a  = i + 1848;
      p->b  = (double) i * 0.5;
   }

   if( _mulle_indexedstorage_get_count( &store) != 10)
      abort();

   // free every second one (indices 0, 2, 4, 6, 8)
   for( i = 0; i < 5; i++)
      _mulle_indexedstorage_free( &store, i * 2);

   if( _mulle_indexedstorage_get_count( &store) != 5)
      abort();

   // reallocate, should reuse freed indices
   for( i = 0; i < 5; i++)
   {
      index = _mulle_indexedstorage_alloc( &store);
      // reused index must be one of the freed ones (0, 2, 4, 6, 8)
      if( index != 0 && index != 2 && index != 4 && index != 6 && index != 8)
         abort();
   }

   if( _mulle_indexedstorage_get_count( &store) != 10)
      abort();

   // verify odd indices still have original data
   for( i = 0; i < 5; i++)
   {
      index = i * 2 + 1;
      p     = _mulle_indexedstorage_get( &store, index);
      if( p->a != (int) index + 1848)
         abort();
      if( p->b != (double) index * 0.5)
         abort();
   }

   _mulle_indexedstorage_done( &store);
   return( 0);
}
