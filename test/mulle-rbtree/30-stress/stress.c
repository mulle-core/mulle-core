#include <mulle-rbtree/mulle-rbtree.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


//
// https://stackoverflow.com/questions/12252103/read-line-from-stdin-blocking
//
int main( int argc, const char * argv[])
{
   struct mulle_rbtree    tree;
   unsigned int           i;
   unsigned int           count;
   char                   buf[ 64];
   void                   *item;

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_copied_cstring,
                      NULL);

   for( i = 0; i < 100000; i++)
   {
      sprintf( buf, "node-%u", rand() % 100);
      if( (i % 3) == 0)
         if( mulle_rbtree_find( &tree, buf))
         {
            if( ! mulle_rbtree_remove( &tree, buf))
               continue;
         }
      mulle_rbtree_add( &tree, buf);

      // duplicates should not "add up"
      count = 0;
      mulle_rbtree_for( &tree, item)
      {
         ++count;
      }
      if( count > 100)
         abort();
   }

   mulle_rbtree_done( &tree);

   return( 0);
}
