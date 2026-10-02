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

   for( i = 0; i < 100; i++)
   {
      index = _mulle_indexedstorage_alloc( &store);
      if( index != i)
         abort();

      p    = _mulle_indexedstorage_get( &store, index);
      p->a = i + 1848;
      p->b = (double) i * 0.5;
   }

   // verify all data is intact
   for( i = 0; i < 100; i++)
   {
      p = _mulle_indexedstorage_get( &store, i);
      if( p->a != (int) i + 1848)
         abort();
      if( p->b != (double) i * 0.5)
         abort();
   }

   if( _mulle_indexedstorage_get_count( &store) != 100)
      abort();

   _mulle_indexedstorage_done( &store);
   return( 0);
}
