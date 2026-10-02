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
   struct mulle_pointerarray     check;
   struct x                      *saved;
   unsigned int                  i;
   unsigned int                  j;
   unsigned int                  n;
   unsigned int                  index;

   mulle_pointerarray_init( &check, 0, NULL);
   _mulle_indexedstorage_init( &store,
                               sizeof( struct x),
                               alignof( struct x),
                               4,
                               NULL);

   for( i = 0; i < 100000; i++)
   {
      n = mulle_pointerarray_get_count( &check);
      if( n && (i % 3 == 0))
      {
         j     = rand() % n;
         index = (unsigned int) (uintptr_t) mulle_pointerarray_get( &check, j);
         mulle_pointerarray_remove_in_range( &check, mulle_range_make( j, 1));
         _mulle_indexedstorage_free( &store, index);
         continue;
      }

      index    = _mulle_indexedstorage_alloc( &store);
      saved    = _mulle_indexedstorage_get( &store, index);
      saved->a = i + 1848;
      saved->b = (double) i * 0.5;
      mulle_pointerarray_add( &check, (void *) (uintptr_t) index);
   }

   _mulle_indexedstorage_done( &store);

   mulle_pointerarray_done( &check);

   return( 0);
}