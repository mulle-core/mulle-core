#include "include.h"

#include <stdio.h>
#include <string.h>


int main(void)
{
   struct mulle__rbtree  tree;
   struct mulle_rbnode   *nil;
   
   _mulle__rbtree_init(&tree, NULL);
   
   nil = _mulle__rbtree_get_nil_node(&tree);
   
   printf("NIL node is black: %s\n", _mulle_rbnode_is_black(nil) ? "YES" : "NO");
   printf("NIL node color value: %d\n", _mulle_rbnode_get_color(nil));
   printf("Empty tree validation: %s\n", mulle__rbtree_validate(&tree) == 0 ? "VALID" : "INVALID");
   
   _mulle__rbtree_done(&tree);
   
   return 0;
}
