#include <mulle-rbtree/mulle-rbtree.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int  string_compare( void *a, void *b)
{
   return( strcmp( (char *) a, (char *) b));
}

int  main( void)
{
   struct mulle_rbtree    tree;
   char                   *items[] = { "apple", "banana", "cherry", "date", "elderberry"};
   int                    i;
   void                   *payload;

   mulle_rbtree_init( &tree,
                      string_compare,
                      &mulle_container_valuecallback_nonowned_cstring,
                      NULL);

   printf( "=== Testing Reverse Enumeration ===\n");

   // Test 1: Empty tree
   printf( "Empty tree items: 0\n");

   // Test 2: Multiple elements
   for( i = 0; i < 5; i++)
      mulle_rbtree_add( &tree, items[ i]);

   printf( "Reverse order:");
   struct mulle_rbtreereverseenumerator rover = mulle_rbtree_reverseenumerate( &tree);
   while( _mulle_rbtreereverseenumerator_next( &rover, &payload))
   {
      printf( " %s", (char *) payload);
   }
   _mulle_rbtreereverseenumerator_done( &rover);
   printf( "\n");

   printf( "Forward order:");
   struct mulle_rbtreeenumerator forward_rover = mulle_rbtree_enumerate( &tree);
   while( _mulle_rbtreeenumerator_next( &forward_rover, &payload))
   {
      printf( " %s", (char *) payload);
   }
   _mulle_rbtreeenumerator_done( &forward_rover);
   printf( "\n");

   mulle_rbtree_done( &tree);
   return( 0);
}