#include "include.h"


static void    print_name(FILE *fp, void *value)
{
    fprintf(fp, "%s", (char *) value);
}


static void   print_tree_state(struct mulle__rbtree *tree)
{
    mulle__rbtree_node_dot_fprintf(stdout, tree, print_name);
    printf("\n");
}

///
static struct mulle_rbnode *
find_node_by_index_rec(struct mulle__rbtree *tree,
                               struct mulle_rbnode *node,
                               unsigned int target_index,
                               unsigned int current_index)
{
    struct mulle_rbnode *nil = _mulle__rbtree_get_nil_node(tree);

    if (node == nil)
        return nil;

    if (target_index == current_index)
        return node;

    struct mulle_rbnode *found;

    found = find_node_by_index_rec(tree, node->_left, target_index, current_index << 1);
    if (found != nil)
        return found;

    return find_node_by_index_rec(tree, node->_right, target_index, (current_index << 1) | 1);
}


static struct mulle_rbnode *
find_node_by_index(struct mulle__rbtree *tree, unsigned int target_index)
{
    if (target_index < 1)
        return _mulle__rbtree_get_nil_node(tree);

    return find_node_by_index_rec(tree, tree->_root, target_index, 1);
}



enum node_state
{
    NODE_NONE      = 0x0,
    NODE_BLACK     = 0x1,
    NODE_RED       = 0x2,
    NODE_ALSO_NONE = 0x3
};

static char *names[] = { "CORRUPT", "A",
                                    "B", "C",
                                    "D", "E", "F", "G",
                                    "H", "I", "J", "K",  "L", "M", "N", "O"  };  // A=1, B=2, C=3, D=4, E=5, G=7


static void   add_node( struct mulle__rbtree *tree,
                        struct mulle_rbnode *parent,
                        enum node_state state,
                        int index,
                        void *payload)
{
    struct mulle_rbnode   *node;

    node = _mulle_storage_malloc( &tree->_nodes);
    _mulle__rbtree_init_node( tree, node, payload);

    node->_color  = (state == NODE_BLACK) ? mulle__rbtree_black : mulle__rbtree_red;

    if( index == 1)
      tree->_root = node;
    else
    {
       node->_parent = parent;


       if( index & 0x1)
          parent->_right = node;
       else
          parent->_left = node;
    }
}


static int   validate_mask( uint64_t mask)
{
   unsigned int      index;
   enum node_state   state;

   index = 0;
   while( mask)
   {
      state  = mask & 0x3;
      mask >>= 2;
      if( state == NODE_ALSO_NONE)
         return( 0);
      if( state == NODE_NONE)
         continue;
      index++;
   }
   return( index > 0);
}


static int   build_tree_from_mask( struct mulle__rbtree *tree, uint64_t mask)
{
   struct mulle_rbnode  *nil;
   struct mulle_rbnode  *parent;
   unsigned int         index;
   enum node_state      state;

   nil    = _mulle__rbtree_get_nil_node( tree);
   index  = 0;
   parent = nil;

   while( mask)
   {
      index++;
      state  = mask & 0x3;
      mask >>= 2;
      if( state == NODE_ALSO_NONE)
         return( -1);
      if( state == NODE_NONE)
         continue;

      if( index != 1)
      {
         parent = find_node_by_index( tree, index >> 1);
         if( parent == nil)
           return( -1);
      }
      add_node( tree, parent, state, index, names[ index]);
   }
   return( 0);
}





int main(void)
{
   uint64_t               min_mask = 1ULL << ((1 + 2 + 4) * 2);   // 14 bits total for 7 nodes
   uint64_t               max_mask = 1ULL << ((7 + 8) * 2);       // 30 bits total for 15 nodes
#ifdef MULLE_TEST_VALGRIND
   max_mask = min_mask + 100000;  // reduced range under valgrind
#endif
   size_t                 max;
   struct mulle__rbtree   proto;
   struct mulle__rbtree   test;
   struct mulle_rbnode    *node;
   struct mulle_rbnode    *proto_node;
   size_t                 i;
   struct mulle_rbnode    *nil;
   char                   *name;
   char                   *err;

   for( uint64_t mask = min_mask; mask <= max_mask; mask++)
   {
      //// need a level 3 node
      //if( mask <= min_mask)
      //   continue;
      if( ! validate_mask( mask))
         continue;

      _mulle__rbtree_init( &proto, NULL);
      if( ! build_tree_from_mask( &proto, mask))
      {
         if( ! mulle__rbtree_validate( &proto))
         {
            max = _mulle__rbtree_get_count( &proto);
            if( max)
            {
               printf("=== Original: 0x%llx\n", (unsigned long long) mask);
               print_tree_state( &proto);
               //for( i = 1; i <= max; i++)
               i = 1;
               {
                  nil = _mulle__rbtree_get_nil_node( &proto);
                  proto_node = find_node_by_index( &proto, i);
                  assert( proto_node != NULL);
                  if( proto_node != nil)
                  {
                     _mulle__rbtree_init( &test, NULL);
                     build_tree_from_mask(&test, mask);
                     node = find_node_by_index( &test, i);
                     name = (char *) _mulle_rbnode_get_payload( node);

                     printf( "---Deleting \"%s\"\n", name);

                     _mulle__rbtree_remove_node( &test, node);

                     print_tree_state( &test);

                     err = mulle__rbtree_validate( &test);
                     if( err)
                     {
                        printf( "***INVALID***: %s\n", err);
                        return( 1);
                     }
                     _mulle__rbtree_done(&test);
                  }
                 printf( "===\n\n");
               }
            }
         }
      }
      _mulle__rbtree_done( &proto);
   }
   return( 0);
}

